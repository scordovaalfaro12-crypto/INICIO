// ==========================================================
//  tests/api.test.js — Pruebas de la API contra una base real
//
//  Las pruebas de lib/ comprueban funciones sueltas. Estas prueban
//  el sistema COMO SE USA: iniciar sesión, cobrar, renovar, marcar
//  entrada, cerrar el mes, respaldar y restaurar.
//
//  Cada fallo que se encontró probando a mano tiene aquí su prueba,
//  para que no pueda volver sin que alguien se entere:
//   - un socio vencido anunciado como "al día" en el mostrador
//   - un pago con fecha futura sumando en todos los periodos
//   - una persona que vuelve y se duplica en vez de renovar
//   - un respaldo que no se podía volver a cargar
//
//  Necesitan PostgreSQL. Si no hay TEST_DATABASE_URL, se saltan
//  solas (así `npm test` sigue funcionando en cualquier máquina).
// ==========================================================

const { test, before, after, describe } = require('node:test');
const assert = require('node:assert');

const URL_PRUEBAS = process.env.TEST_DATABASE_URL;
const SALTAR = !URL_PRUEBAS;

// La configuración debe quedar puesta ANTES de cargar el servidor.
if (!SALTAR) {
  process.env.DATABASE_URL = URL_PRUEBAS;
  process.env.JWT_SECRET = 'clave_solo_para_pruebas_0123456789abcdef';
  process.env.ADMIN_PASSWORD = 'ClaveDePrueba123';
  process.env.PORT = process.env.TEST_PORT || '4977';
  process.env.NODE_ENV = 'test';
}

let db, servidor, BASE, token;

// Petición a la API con la sesión ya iniciada.
async function pide(metodo, ruta, cuerpo) {
  const opciones = { method: metodo, headers: {} };
  if (token) opciones.headers.Authorization = 'Bearer ' + token;
  if (cuerpo !== undefined) {
    opciones.headers['Content-Type'] = 'application/json';
    opciones.body = typeof cuerpo === 'string' ? cuerpo : JSON.stringify(cuerpo);
  }
  const res = await fetch(BASE + ruta, opciones);
  let datos = null;
  try { datos = await res.json(); } catch (e) { /* respuesta sin cuerpo */ }
  return { estado: res.status, datos };
}

// Deja la base como recién instalada, sin tocar al usuario administrador.
async function limpiarDatos() {
  await db.query('TRUNCATE memberships, payments, extra_sales, expenses, attendance, classes RESTART IDENTITY CASCADE');
}

// Fecha del negocio según el servidor (no la del reloj de esta máquina).
async function hoyNegocio() {
  const { datos } = await pide('GET', '/api/asistencia');
  return datos.fecha;
}

function sumaDias(iso, n) {
  const [a, m, d] = iso.split('-').map(Number);
  const dt = new Date(Date.UTC(a, m - 1, d));
  dt.setUTCDate(dt.getUTCDate() + n);
  return dt.toISOString().slice(0, 10);
}

before(async () => {
  if (SALTAR) return;
  db = require('../db');
  servidor = require('../server'); // arranca el servidor y conecta la base
  BASE = 'http://127.0.0.1:' + process.env.PORT;

  // Espera a que el sistema esté REALMENTE listo: no basta con que el puerto
  // acepte conexiones, la base tiene que haber aplicado el esquema. Mientras
  // tanto la API responde 503 a propósito.
  let listo = false;
  for (let i = 0; i < 150 && !listo; i++) {
    try {
      const res = await fetch(BASE + '/api/health');
      const estado = await res.json();
      listo = estado.db === 'ok';
    } catch (e) { /* todavía levantando */ }
    if (!listo) await new Promise((r) => setTimeout(r, 200));
  }
  assert.ok(listo, 'la base de datos de pruebas debe estar disponible');
  const r = await pide('POST', '/api/auth/login', { email: 'admin@zonavip.com', password: 'ClaveDePrueba123' });
  assert.equal(r.estado, 200, 'el administrador debe poder entrar');
  token = r.datos.token;
});

after(async () => {
  if (SALTAR) return;
  await limpiarDatos();
  if (servidor && servidor.cerrar) await servidor.cerrar();
  await db.cerrarPool().catch(() => {});
});

describe('Acceso al sistema', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('sin token no se puede ver nada', async () => {
    const guardado = token; token = null;
    const r = await pide('GET', '/api/memberships');
    token = guardado;
    assert.equal(r.estado, 401);
  });

  test('un token inventado no sirve', async () => {
    const guardado = token;
    token = 'eyJhbGciOiJIUzI1NiJ9.eyJpZCI6MSwicm9sZSI6ImFkbWluIn0.falso';
    const r = await pide('GET', '/api/memberships');
    token = guardado;
    assert.equal(r.estado, 401);
  });

  test('la contraseña equivocada no entra y no dice por qué', async () => {
    const r = await pide('POST', '/api/auth/login', { email: 'admin@zonavip.com', password: 'incorrecta' });
    assert.ok(r.estado === 401 || r.estado === 429);
    if (r.estado === 401) assert.match(r.datos.error, /Usuario o contraseña/);
  });
});

describe('Cobrar y renovar', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('registrar un pago crea la ficha Y su asiento en el libro', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const r = await pide('POST', '/api/memberships', {
      nombre: 'Ana Torres', telefono: '987654321', dni: '12345678',
      concepto: 'Matrícula mensual', monto: 80, metodo: 'Yape',
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    assert.equal(r.estado, 201);
    const pagos = await pide('GET', '/api/memberships/pagos');
    assert.equal(pagos.datos.length, 1, 'cada matrícula deja exactamente un pago');
    assert.equal(Number(pagos.datos[0].monto), 80);
  });

  test('una persona que ya existe NO se duplica: se ofrece renovarle', async () => {
    const hoy = await hoyNegocio();
    const r = await pide('POST', '/api/memberships', {
      nombre: 'ana torres', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    assert.equal(r.estado, 409);
    assert.ok(r.datos.socio_existente, 'debe señalar la ficha que ya existe');
  });

  test('el nombre se compara sin tildes ("Jose" encuentra a "José")', async () => {
    const hoy = await hoyNegocio();
    await pide('POST', '/api/memberships', {
      nombre: 'José Ramírez', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    const r = await pide('POST', '/api/memberships', {
      nombre: 'Jose Ramirez', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    assert.equal(r.estado, 409, 'debe reconocerlo aunque se escriba sin tildes');
    const busca = await pide('GET', '/api/memberships?q=jose');
    assert.equal(busca.datos.total, 1);
  });

  test('renovar suma días y deja el cobro en su fecha real', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const nuevo = await pide('POST', '/api/memberships', {
      nombre: 'Luis Quispe', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 10),
    });
    const id = nuevo.datos.id;
    const ayer = sumaDias(hoy, -1);
    const r = await pide('PUT', `/api/memberships/${id}/renew`, {
      dias: 30, monto: 210, metodo: 'Efectivo',
      concepto: 'Máquinas 3 meses (promo)', fecha_pago: ayer,
    });
    assert.equal(r.estado, 200);
    assert.equal(r.datos.nuevoVence, sumaDias(hoy, 40), 'los días se suman al vencimiento vigente');

    const pagos = await pide('GET', '/api/memberships/pagos');
    const renovacion = pagos.datos.find((p) => p.tipo === 'renovacion');
    assert.equal(renovacion.fecha_pago, ayer, 'el cobro cuenta en el día en que se recibió');
    assert.equal(renovacion.concepto, 'Máquinas 3 meses (promo)', 'guarda el plan realmente cobrado');
  });

  test('no se acepta un cobro con fecha futura', async () => {
    const hoy = await hoyNegocio();
    const socios = await pide('GET', '/api/memberships');
    const id = socios.datos.items[0].id;
    const r = await pide('PUT', `/api/memberships/${id}/renew`, {
      dias: 30, monto: 80, fecha_pago: sumaDias(hoy, 5),
    });
    assert.equal(r.estado, 400);
  });

  test('dos renovaciones a la vez no se pisan (cada una suma sus días)', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const nuevo = await pide('POST', '/api/memberships', {
      nombre: 'Rosa Vega', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: hoy,
    });
    const id = nuevo.datos.id;
    await Promise.all(Array.from({ length: 5 }, () =>
      pide('PUT', `/api/memberships/${id}/renew`, { dias: 10, monto: 50 })));
    const socios = await pide('GET', '/api/memberships');
    assert.equal(socios.datos.items[0].fecha_vence, sumaDias(hoy, 50), '5 renovaciones de 10 días = 50 días');
    const pagos = await pide('GET', '/api/memberships/pagos');
    assert.equal(pagos.datos.filter((p) => p.tipo === 'renovacion').length, 5);
  });
});

describe('El dinero cuadra', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('un pago con fecha futura NO infla los periodos', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    await pide('POST', '/api/memberships', {
      nombre: 'Socio Normal', concepto: 'Matrícula mensual', monto: 100,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    // Dedazo típico: el año siguiente en vez del actual.
    await db.query(
      `INSERT INTO payments (tipo, nombre, concepto, monto, metodo, fecha_pago)
       VALUES ('matricula', 'Dedazo', 'Matrícula mensual', 5000, 'Efectivo', $1)`,
      [sumaDias(hoy, 300)]
    );
    const f = (await pide('GET', '/api/memberships/finanzas')).datos;
    assert.equal(f.hoy.total, 100, 'hoy solo cuenta lo cobrado hoy');
    assert.equal(f.anio.total, 100, 'el año tampoco incluye dinero del futuro');
    assert.equal(f.futuros.cantidad, 1, 'pero avisa de que existe');
    assert.equal(f.futuros.total, 5000);
  });

  test('anular un pago lo saca de las cuentas sin borrarlo', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    await pide('POST', '/api/memberships', {
      nombre: 'Socio Error', concepto: 'Matrícula mensual', monto: 800,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    const pagos = await pide('GET', '/api/memberships/pagos');
    const idPago = pagos.datos[0].id;
    assert.equal((await pide('GET', '/api/memberships/finanzas')).datos.hoy.total, 800);

    const r = await pide('PUT', `/api/memberships/pagos/${idPago}/anular`, { motivo: 'Tecleé 800 en vez de 80' });
    assert.equal(r.estado, 200);
    assert.equal((await pide('GET', '/api/memberships/finanzas')).datos.hoy.total, 0);

    const fila = await db.queryOne('SELECT anulado, motivo_anulacion FROM payments WHERE id = $1', [idPago]);
    assert.equal(fila.anulado, true, 'el registro sobrevive');
    assert.match(fila.motivo_anulacion, /800/);
  });

  test('la utilidad descuenta los gastos de los ingresos', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    await pide('POST', '/api/memberships', {
      nombre: 'Socia Paga', concepto: 'Matrícula mensual', monto: 300,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    await pide('POST', '/api/extras', { categoria: 'Proteína', monto: 90, fecha: hoy });
    await pide('POST', '/api/gastos', { categoria: 'Alquiler', monto: 1200, fecha: hoy });

    const f = (await pide('GET', '/api/memberships/finanzas')).datos;
    assert.equal(f.mes.total, 390, 'ingresos = matrículas + ventas');
    assert.equal(f.mes.gastos.total, 1200);
    assert.equal(f.mes.utilidad, -810, 'utilidad = ingresos - gastos (aquí, pérdida)');
  });

  test('los céntimos no se pierden al sumar', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    for (const monto of [10.10, 20.25, 0.05, 33.33]) {
      await pide('POST', '/api/extras', { categoria: 'Aguas/Bebidas', monto, fecha: hoy });
    }
    const f = (await pide('GET', '/api/memberships/finanzas')).datos;
    assert.equal(f.hoy.total, 63.73, '10.10 + 20.25 + 0.05 + 33.33');
  });
});

describe('Control de entrada', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('un socio vencido se anuncia como VENCIDO aunque la columna diga otra cosa', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const nuevo = await pide('POST', '/api/memberships', {
      nombre: 'Vencido Hace Meses', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 5),
    });
    // Se simula lo que pasa de verdad: venció, pero el refresco horario aún
    // no corrió y la columna sigue diciendo 'activa'.
    await db.query(`UPDATE memberships SET fecha_vence = $1, estado = 'activa' WHERE id = $2`,
      [sumaDias(hoy, -87), nuevo.datos.id]);

    const r = await pide('POST', '/api/asistencia', { membership_id: nuevo.datos.id });
    assert.match(r.datos.mensaje, /VENCIDA hace 87 días/);
    assert.equal(r.datos.socio.estado, 'vencida');
    assert.equal(r.datos.dias_restantes, -87);
  });

  test('marcar dos veces el mismo día cuenta una sola visita', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const nuevo = await pide('POST', '/api/memberships', {
      nombre: 'Doble Marca', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    const primera = await pide('POST', '/api/asistencia', { membership_id: nuevo.datos.id });
    const segunda = await pide('POST', '/api/asistencia', { membership_id: nuevo.datos.id });
    assert.equal(primera.datos.ya_marcado, false);
    assert.equal(segunda.datos.ya_marcado, true);
    const visitas = await db.queryOne('SELECT COUNT(*)::int AS n FROM attendance');
    assert.equal(visitas.n, 1);
  });

  test('avisa cuando faltan pocos días (momento de cobrar la renovación)', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const nuevo = await pide('POST', '/api/memberships', {
      nombre: 'Casi Vence', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 2),
    });
    const r = await pide('POST', '/api/asistencia', { membership_id: nuevo.datos.id });
    assert.match(r.datos.mensaje, /vence en 2 días/);
  });
});

describe('Entradas inválidas', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  const malos = [
    ['monto negativo', { monto: -50 }],
    ['monto que no es número', { monto: true }],
    ['monto en blanco', { monto: '   ' }],
    ['fecha inexistente', { fecha_pago: '2026-02-30' }],
    ['fecha en otro formato', { fecha_pago: '27/07/2026' }],
    ['sin nombre', { nombre: '   ' }],
  ];
  for (const [caso, cambio] of malos) {
    test(`se rechaza: ${caso}`, async () => {
      const hoy = await hoyNegocio();
      const base = {
        nombre: 'Prueba', concepto: 'Matrícula mensual', monto: 80,
        fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
      };
      const r = await pide('POST', '/api/memberships', { ...base, ...cambio });
      assert.equal(r.estado, 400, 'debe rechazarse con un mensaje claro');
    });
  }

  test('el texto malicioso se guarda como texto, no se ejecuta', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const veneno = "Robert'); DROP TABLE memberships;--";
    const r = await pide('POST', '/api/memberships', {
      nombre: veneno, concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    assert.equal(r.estado, 201);
    const fila = await db.queryOne('SELECT nombre FROM memberships WHERE id = $1', [r.datos.id]);
    assert.equal(fila.nombre, veneno, 'se guarda tal cual, la tabla sigue en pie');
  });

  test('un JSON roto no tumba nada', async () => {
    const r = await pide('POST', '/api/memberships', '{roto');
    assert.equal(r.estado, 400);
  });
});

describe('Respaldo y restauración', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('un respaldo se puede volver a cargar y devuelve el dinero exacto', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    await pide('POST', '/api/memberships', {
      nombre: 'Socio Respaldo', concepto: 'Matrícula mensual', monto: 250,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30),
    });
    await pide('POST', '/api/gastos', { categoria: 'Luz', monto: 180, fecha: hoy });

    const respaldo = (await pide('GET', '/api/backup')).datos;
    assert.equal(respaldo.sistema, 'ZONA VIP GYM');
    assert.ok(respaldo.tablas.payments.length > 0);

    const antes = (await pide('GET', '/api/memberships/finanzas')).datos.mes;
    await limpiarDatos();
    assert.equal((await pide('GET', '/api/backup/estado')).datos.socios, 0);

    const r = await pide('POST', '/api/backup/restaurar', respaldo);
    assert.equal(r.estado, 200);

    const despues = (await pide('GET', '/api/memberships/finanzas')).datos.mes;
    assert.equal(despues.total, antes.total, 'los ingresos vuelven exactos');
    assert.equal(despues.gastos.total, antes.gastos.total, 'los gastos también');
  });

  test('la contraseña NO se toca al restaurar', async () => {
    const r = await pide('POST', '/api/auth/login', { email: 'admin@zonavip.com', password: 'ClaveDePrueba123' });
    assert.ok(r.estado === 200 || r.estado === 429, 'se sigue pudiendo entrar tras una restauración');
  });

  test('un archivo que no es un respaldo se rechaza sin tocar los datos', async () => {
    const antes = (await pide('GET', '/api/backup/estado')).datos;
    for (const basura of [{ hola: 'mundo' }, { sistema: 'Otro', tablas: { memberships: [] } }, { tablas: {} }]) {
      const r = await pide('POST', '/api/backup/restaurar', basura);
      assert.equal(r.estado, 400);
    }
    const despues = (await pide('GET', '/api/backup/estado')).datos;
    assert.deepEqual(despues, antes, 'nada cambió');
  });
});

describe('Aguanta el volumen', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('con 1.000 socios el listado y el resumen siguen siendo rápidos', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    await db.query(
      `INSERT INTO memberships (nombre, telefono, dni, concepto, monto, metodo, fecha_pago, fecha_vence, estado)
       SELECT 'Socio '||g, '9'||lpad(g::text,8,'0'), lpad(g::text,8,'0'), 'Matrícula mensual',
              80, 'Efectivo', $1, $2, 'activa' FROM generate_series(1,1000) g`,
      [hoy, sumaDias(hoy, 30)]
    );
    const t0 = Date.now();
    const lista = await pide('GET', '/api/memberships?limit=300');
    const resumen = await pide('GET', '/api/memberships/resumen');
    const ms = Date.now() - t0;

    assert.equal(lista.datos.items.length, 300, 'la pantalla recibe como mucho el tope pedido');
    assert.equal(lista.datos.total, 1000, 'pero sabe cuántos hay en total');
    assert.equal(resumen.datos.activos, 1000);
    assert.ok(ms < 3000, `debería responder rápido y tardó ${ms} ms`);
  });
});
