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

describe('Pausar y reactivar membresías', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('los días pausados se devuelven exactos al reactivar', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const nuevo = await pide('POST', '/api/memberships', {
      nombre: 'Viajero', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 25),
    });
    const id = nuevo.datos.id;

    const congelar = await pide('PUT', `/api/memberships/${id}/congelar`, { motivo: 'Viaje' });
    assert.equal(congelar.estado, 200);
    assert.equal(congelar.datos.dias_guardados, 25);

    // Se simula que estuvo 14 días parado.
    await db.query(`UPDATE memberships SET congelada_desde = $1 WHERE id = $2`, [sumaDias(hoy, -14), id]);
    const reactivar = await pide('PUT', `/api/memberships/${id}/reactivar`);
    assert.equal(reactivar.datos.dias_devueltos, 14);
    assert.equal(reactivar.datos.nuevoVence, sumaDias(hoy, 39), '25 que le quedaban + 14 parado');
  });

  test('una membresía pausada no cuenta como activa ni vence sola', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    const nuevo = await pide('POST', '/api/memberships', {
      nombre: 'Pausado', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: hoy, fecha_vence: sumaDias(hoy, 3),
    });
    await pide('PUT', `/api/memberships/${nuevo.datos.id}/congelar`, {});

    const resumen = (await pide('GET', '/api/memberships/resumen')).datos;
    assert.equal(resumen.congelados, 1);
    assert.equal(resumen.activos, 0, 'un socio en pausa no se cuenta como activo');
    assert.equal(resumen.pronto, 0, 'ni aparece entre los que vencen pronto');

    const porVencer = (await pide('GET', '/api/memberships/por-vencer?dias=30')).datos;
    assert.equal(porVencer.length, 0, 'no se le avisa: su tiempo está parado');

    // Aunque pase la fecha, el refresco automático no debe marcarla vencida.
    await db.query(`UPDATE memberships SET fecha_vence = $1 WHERE id = $2`, [sumaDias(hoy, -5), nuevo.datos.id]);
    await db.actualizarEstadoMemberships(true);
    const fila = await db.queryOne('SELECT estado FROM memberships WHERE id = $1', [nuevo.datos.id]);
    assert.equal(fila.estado, 'congelada');
  });

  test('en el mostrador se avisa de que está pausada', async () => {
    const socios = (await pide('GET', '/api/memberships')).datos.items;
    const r = await pide('POST', '/api/asistencia', { membership_id: socios[0].id });
    assert.match(r.datos.mensaje, /CONGELADA/);
  });

  test('no se puede pausar dos veces ni una ya vencida', async () => {
    const socios = (await pide('GET', '/api/memberships')).datos.items;
    const dos = await pide('PUT', `/api/memberships/${socios[0].id}/congelar`, {});
    assert.equal(dos.estado, 400);

    const hoy = await hoyNegocio();
    const vencido = await pide('POST', '/api/memberships', {
      nombre: 'Ya Vencido', concepto: 'Matrícula mensual', monto: 80,
      fecha_pago: '2026-01-01', fecha_vence: sumaDias(hoy, -10),
    });
    const r = await pide('PUT', `/api/memberships/${vencido.datos.id}/congelar`, {});
    assert.equal(r.estado, 400);
  });
});

describe('Inventario', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('vender descuenta el stock y anular lo devuelve', async () => {
    await limpiarDatos();
    await db.query('TRUNCATE products, stock_movements RESTART IDENTITY CASCADE');
    const hoy = await hoyNegocio();
    const p = await pide('POST', '/api/inventario', {
      nombre: 'Proteína', categoria: 'Suplementos', precio: 90, stock: 10, stock_minimo: 3,
    });
    const id = p.datos.id;

    const venta = await pide('POST', '/api/extras', {
      categoria: 'Suplementos', monto: 180, fecha: hoy, producto_id: id, cantidad: 2,
    });
    assert.equal(venta.estado, 201);
    assert.equal(venta.datos.stock_restante, 8);

    const anular = await pide('DELETE', `/api/extras/${venta.datos.id}`);
    assert.equal(anular.estado, 200);
    const prod = await db.queryOne('SELECT stock FROM products WHERE id = $1', [id]);
    assert.equal(prod.stock, 10, 'el producto vuelve al almacén');
  });

  test('el stock siempre cuadra con su historial de movimientos', async () => {
    const productos = (await pide('GET', '/api/inventario')).datos;
    const id = productos[0].id;
    await pide('POST', `/api/inventario/${id}/entrada`, { cantidad: 5, motivo: 'Pedido' });
    await pide('POST', '/api/extras', {
      categoria: 'Suplementos', monto: 90, fecha: await hoyNegocio(), producto_id: id, cantidad: 1,
    });
    await pide('POST', `/api/inventario/${id}/ajuste`, { stock_real: 12, motivo: 'Conteo' });

    const cuadre = await db.queryOne(
      `SELECT p.stock, COALESCE(SUM(m.cantidad), 0)::int AS suma
       FROM products p LEFT JOIN stock_movements m ON m.producto_id = p.id
       WHERE p.id = $1 GROUP BY p.stock`,
      [id]
    );
    assert.equal(cuadre.stock, cuadre.suma, 'stock y suma de movimientos coinciden');
  });

  test('vender más de lo que hay avisa antes de dejar el stock en negativo', async () => {
    const productos = (await pide('GET', '/api/inventario')).datos;
    const id = productos[0].id;
    const stockActual = productos[0].stock;
    const r = await pide('POST', '/api/extras', {
      categoria: 'Suplementos', monto: 900, fecha: await hoyNegocio(),
      producto_id: id, cantidad: stockActual + 5,
    });
    assert.equal(r.estado, 409);
    assert.equal(r.datos.sin_stock, true);
  });

  test('avisa de los productos por debajo del mínimo', async () => {
    await db.query('TRUNCATE products, stock_movements RESTART IDENTITY CASCADE');
    await pide('POST', '/api/inventario', { nombre: 'Agua', precio: 2.5, stock: 2, stock_minimo: 6 });
    const r = (await pide('GET', '/api/inventario/resumen')).datos;
    assert.equal(r.por_reponer, 1);
    assert.equal(r.faltantes[0].nombre, 'Agua');
  });
});

describe('Recepción no puede tocar el dinero', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  let tokenRecepcion;

  before(async () => {
    if (SALTAR) return;
    await pide('POST', '/api/usuarios', {
      firstname: 'Mostrador', lastname: 'Prueba', email: 'recepcion.test@zonavip.com',
      role: 'recepcion', password: 'Mostrador2026',
    });
    const r = await fetch(BASE + '/api/auth/login', {
      method: 'POST', headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ email: 'recepcion.test@zonavip.com', password: 'Mostrador2026' }),
    }).then((x) => x.json());
    tokenRecepcion = r.token;
    assert.ok(tokenRecepcion, 'recepción debe poder iniciar sesión');
  });

  async function comoRecepcion(metodo, ruta, cuerpo) {
    const guardado = token;
    token = tokenRecepcion;
    const r = await pide(metodo, ruta, cuerpo);
    token = guardado;
    return r;
  }

  test('SÍ puede lo operativo del mostrador', async () => {
    const hoy = await hoyNegocio();
    for (const [metodo, ruta, cuerpo] of [
      ['GET', '/api/memberships'],
      ['GET', '/api/memberships/resumen'],
      ['GET', '/api/asistencia'],
      ['GET', '/api/inventario'],
      ['GET', '/api/catalogo'],
      ['POST', '/api/memberships', { nombre: 'Socio de Recepción', concepto: 'Matrícula mensual', monto: 80, fecha_pago: hoy, fecha_vence: sumaDias(hoy, 30) }],
      ['POST', '/api/extras', { categoria: 'Bebidas', monto: 2.5, fecha: hoy }],
    ]) {
      const r = await comoRecepcion(metodo, ruta, cuerpo);
      assert.notEqual(r.estado, 403, `recepción debería poder: ${metodo} ${ruta}`);
    }
  });

  test('NO puede ver ni tocar el dinero', async () => {
    for (const [metodo, ruta, cuerpo] of [
      ['GET', '/api/memberships/finanzas'],
      ['GET', '/api/memberships/pagos'],
      ['PUT', '/api/memberships/pagos/1/anular', { motivo: 'x' }],
      ['GET', '/api/gastos'],
      ['POST', '/api/gastos', { categoria: 'Luz', monto: 100, fecha: '2026-07-01' }],
      ['GET', '/api/extras/resumen?desde=2026-01-01'],
      ['DELETE', '/api/extras/1'],
      ['GET', '/api/inventario/resumen'],
      ['POST', '/api/inventario/1/entrada', { cantidad: 5 }],
      ['GET', '/api/backup'],
      ['GET', '/api/usuarios'],
      ['POST', '/api/catalogo', { tipo: 'matricula', nombre: 'X', precio: 1, dias: 30 }],
    ]) {
      const r = await comoRecepcion(metodo, ruta, cuerpo);
      assert.equal(r.estado, 403, `recepción NO debería poder: ${metodo} ${ruta}`);
    }
  });

  test('quitarle el acceso surte efecto AL INSTANTE, sin esperar a que caduque su sesión', async () => {
    const usuarios = (await pide('GET', '/api/usuarios')).datos;
    const recep = usuarios.find((u) => u.email === 'recepcion.test@zonavip.com');

    assert.notEqual((await comoRecepcion('GET', '/api/memberships')).estado, 401);
    await pide('PUT', `/api/usuarios/${recep.id}/activo`, { activo: false });
    // Su token sigue siendo válido criptográficamente, pero ya no debe servir.
    assert.equal((await comoRecepcion('GET', '/api/memberships')).estado, 401);

    await pide('PUT', `/api/usuarios/${recep.id}/activo`, { activo: true });
    assert.notEqual((await comoRecepcion('GET', '/api/memberships')).estado, 401);
  });

  test('subirle el rol también surte efecto al instante', async () => {
    const usuarios = (await pide('GET', '/api/usuarios')).datos;
    const recep = usuarios.find((u) => u.email === 'recepcion.test@zonavip.com');

    assert.equal((await comoRecepcion('GET', '/api/memberships/finanzas')).estado, 403);
    await pide('PUT', `/api/usuarios/${recep.id}`, { firstname: 'Mostrador', role: 'admin' });
    assert.equal((await comoRecepcion('GET', '/api/memberships/finanzas')).estado, 200);
    await pide('PUT', `/api/usuarios/${recep.id}`, { firstname: 'Mostrador', role: 'recepcion' });
    assert.equal((await comoRecepcion('GET', '/api/memberships/finanzas')).estado, 403);
  });

  test('siempre debe quedar una administradora', async () => {
    const usuarios = (await pide('GET', '/api/usuarios')).datos;
    const admins = usuarios.filter((u) => u.role === 'admin' && u.activo);
    if (admins.length === 1) {
      const r = await pide('PUT', `/api/usuarios/${admins[0].id}`, { firstname: 'X', role: 'recepcion' });
      assert.equal(r.estado, 400);
    }
  });
});

describe('Paginación y recordatorio de respaldo', { skip: SALTAR && 'sin TEST_DATABASE_URL' }, () => {
  test('se puede pasar de página más allá del tope', async () => {
    await limpiarDatos();
    const hoy = await hoyNegocio();
    await db.query(
      `INSERT INTO memberships (nombre, concepto, monto, metodo, fecha_pago, fecha_vence, estado)
       SELECT 'Socio '||g, 'Matrícula mensual', 80, 'Efectivo', $1, $2, 'activa'
       FROM generate_series(1, 700) g`,
      [hoy, sumaDias(hoy, 30)]
    );
    const p1 = (await pide('GET', '/api/memberships?limit=300')).datos;
    assert.equal(p1.items.length, 300);
    assert.equal(p1.total, 700);
    assert.equal(p1.hay_mas, true);

    const p3 = (await pide('GET', '/api/memberships?limit=300&offset=600')).datos;
    assert.equal(p3.items.length, 100, 'la última página trae el resto');
    assert.equal(p3.hay_mas, false);

    const ids1 = new Set(p1.items.map((m) => m.id));
    assert.ok(p3.items.every((m) => !ids1.has(m.id)), 'las páginas no repiten socios');
  });

  test('avisa cuando hace mucho que no se descarga un respaldo', async () => {
    await db.query(`DELETE FROM ajustes WHERE clave = 'ultimo_respaldo'`);
    const sinNada = (await pide('GET', '/api/backup/recordatorio')).datos;
    assert.equal(sinNada.nunca, true);
    assert.equal(sinNada.toca, true);

    await pide('GET', '/api/backup');
    const recien = (await pide('GET', '/api/backup/recordatorio')).datos;
    assert.equal(recien.toca, false, 'recién descargado no molesta');

    await db.query(`UPDATE ajustes SET valor = $1 WHERE clave = 'ultimo_respaldo'`,
      [new Date(Date.now() - 45 * 86400000).toISOString()]);
    const viejo = (await pide('GET', '/api/backup/recordatorio')).datos;
    assert.equal(viejo.toca, true);
    assert.equal(viejo.dias, 45);
  });
});
