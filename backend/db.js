// ==========================================================
//  db.js — Conexión PostgreSQL (Supabase, Railway o similar) + esquema + migraciones
//
//  Principios de este módulo:
//  1. El historial de pagos NUNCA se borra solo. (La versión
//     anterior eliminaba membresías 15 días después de vencer,
//     destruyendo los reportes de ingresos del mes/año.)
//  2. Cada pago (matrícula nueva o renovación) se registra en
//     la tabla `payments` con su fecha real: las finanzas se
//     calculan de ahí y nunca se desinflan.
//  3. Las migraciones son idempotentes y quedan registradas en
//     `schema_migrations`: arrancar el server 1.000 veces deja
//     la base igual que arrancarlo 1 vez.
// ==========================================================

const { Pool, types } = require('pg');
const bcrypt = require('bcryptjs');
const config = require('./config');
const { todayISO } = require('./lib/dates');

// pg devuelve NUMERIC como string para no perder precisión.
// Los montos usan 2 decimales, así que convertir a número aquí
// (en un solo lugar) es seguro y el frontend recibe números.
types.setTypeParser(1700, (v) => (v === null ? null : parseFloat(v)));

// SSL según el proveedor: Supabase lo exige, el Postgres interno de Railway
// puede no soportarlo. Se respeta sslmode=disable si viene en la URL; en los
// demás casos se intenta con SSL y, si el saludo falla, initDBConReintentos
// cambia de modo y reconecta solo (sin intervención de nadie).
const esLocal = /localhost|127\.0\.0\.1/.test(config.DATABASE_URL);
let usarSSL = !esLocal && !/sslmode=disable|ssl=false/i.test(config.DATABASE_URL);

let pool = crearPool();

function crearPool() {
  const p = new Pool({
    connectionString: config.DATABASE_URL,
    ssl: usarSSL ? { rejectUnauthorized: false } : false,
    max: 8,                       // Supabase free limita conexiones: pocas y estables
    idleTimeoutMillis: 30000,
    connectionTimeoutMillis: 10000,
    keepAlive: true,
    keepAliveInitialDelayMillis: 10000, // el default del sistema son 2 HORAS:
                                        // demasiado tarde para detectar que un
                                        // intermediario cortó la conexión.
    // Una consulta jamás debe quedarse colgada para siempre: si lo hiciera,
    // retendría una conexión del pool y, tras unas pocas, el sistema entero
    // dejaría de responder aunque la base estuviera sana.
    statement_timeout: 20000,     // lo corta el servidor de base de datos
    query_timeout: 25000,         // lo corta el cliente, por si el otro falla
  });
  // CRÍTICO: sin este handler, un corte de red en una conexión inactiva
  // emite 'error' sin listener y TUMBA el proceso completo. Es la causa
  // número 1 de caídas de Node + Postgres en la nube.
  p.on('error', (err) => {
    console.error('[DB] Error en conexión inactiva (recuperado):', err.message);
  });
  return p;
}

async function query(text, params) {
  return pool.query(text, params);
}

async function queryOne(text, params) {
  const res = await pool.query(text, params);
  return res.rows[0] || null;
}

async function queryAll(text, params) {
  const res = await pool.query(text, params);
  return res.rows;
}

// Transacción: todo el bloque se aplica completo o no se aplica nada.
// Se usa para que "registrar pago" + "actualizar membresía" jamás queden a medias.
async function withTransaction(fn) {
  const client = await pool.connect();
  try {
    await client.query('BEGIN');
    const result = await fn(client);
    await client.query('COMMIT');
    return result;
  } catch (err) {
    try { await client.query('ROLLBACK'); } catch (e) { /* conexión ya caída */ }
    throw err;
  } finally {
    client.release();
  }
}

let dbReady = false;
function isDBReady() { return dbReady; }

// Durante el apagado ordenado se detienen los reintentos de conexión
// (si no, el bucle seguiría intentando sobre un pool ya cerrado).
let detenido = false;
function detenerReintentos() { detenido = true; }

// `ejecutar` es SIEMPRE la conexión que tiene tomado el candado de migraciones:
// así todo el esquema se aplica en una sola sesión, con su tope de tiempo
// ampliado y sin que otra instancia pueda entrar a la mitad.
async function runMigration(ejecutar, id, fn) {
  const r = await ejecutar('SELECT id FROM schema_migrations WHERE id = $1', [id]);
  if (r.rows.length > 0) return;
  await fn();
  await ejecutar('INSERT INTO schema_migrations (id) VALUES ($1) ON CONFLICT (id) DO NOTHING', [id]);
  console.log(`[DB] Migración aplicada: ${id}`);
}

// Número arbitrario pero fijo que identifica "el candado de migraciones de
// ZONA VIP GYM" dentro de Postgres.
const LOCK_MIGRACIONES = 728411;

async function initDB() {
  console.log('[DB] Verificando esquema...');

  // Candado a nivel de base de datos mientras se crea/migra el esquema.
  // Durante un redeploy, Railway puede tener la instancia vieja y la nueva
  // arrancando a la vez: sin este candado, las dos podrían ejecutar la misma
  // migración simultáneamente y duplicar TODO el historial de pagos.
  // El candado se toma con una conexión propia y se suelta pase lo que pase.
  const cliente = await pool.connect();
  try {
    // Las migraciones pueden tardar más que una consulta normal (y hay que
    // poder esperar a que la otra instancia suelte el candado), así que en
    // ESTA conexión el tope sube a 2 minutos en vez de los 20 segundos.
    await cliente.query('SET statement_timeout = 120000');
    await cliente.query('SELECT pg_advisory_lock($1)', [LOCK_MIGRACIONES]);
    await aplicarEsquema((texto, params) => cliente.query(texto, params));
  } finally {
    try { await cliente.query('SELECT pg_advisory_unlock($1)', [LOCK_MIGRACIONES]); } catch (e) { /* conexión ya caída */ }
    cliente.release();
  }
}

async function aplicarEsquema(ejecutar) {
  await ejecutar(`
    CREATE TABLE IF NOT EXISTS schema_migrations (
      id TEXT PRIMARY KEY,
      aplicada TIMESTAMPTZ DEFAULT NOW()
    );
  `);

  await ejecutar(`
    CREATE TABLE IF NOT EXISTS users (
      id SERIAL PRIMARY KEY,
      firstname TEXT NOT NULL,
      lastname TEXT NOT NULL,
      email TEXT UNIQUE NOT NULL,
      password TEXT NOT NULL,
      role TEXT DEFAULT 'admin'
    );
  `);

  await ejecutar(`
    CREATE TABLE IF NOT EXISTS classes (
      id SERIAL PRIMARY KEY,
      titulo TEXT NOT NULL,
      instructor TEXT NOT NULL,
      fecha TEXT NOT NULL,
      hora TEXT NOT NULL,
      descripcion TEXT,
      imagen TEXT,
      capacidad INTEGER DEFAULT 20,
      precio REAL DEFAULT 15,
      estado TEXT DEFAULT 'activa'
    );
  `);

  await ejecutar(`
    CREATE TABLE IF NOT EXISTS memberships (
      id SERIAL PRIMARY KEY,
      nombre TEXT NOT NULL,
      telefono TEXT,
      dni TEXT,
      concepto TEXT NOT NULL,
      monto REAL NOT NULL,
      metodo TEXT DEFAULT 'Efectivo',
      fecha_pago TEXT NOT NULL,
      fecha_vence TEXT NOT NULL,
      notas TEXT,
      estado TEXT DEFAULT 'activa',
      creado TIMESTAMP DEFAULT NOW()
    );
  `);

  await ejecutar(`
    CREATE TABLE IF NOT EXISTS extra_sales (
      id SERIAL PRIMARY KEY,
      categoria TEXT NOT NULL,
      descripcion TEXT,
      monto REAL NOT NULL,
      metodo TEXT DEFAULT 'Efectivo',
      fecha TEXT NOT NULL,
      notas TEXT,
      creado TIMESTAMP DEFAULT NOW()
    );
  `);

  // Tabla heredada de la versión anterior: se conserva por compatibilidad.
  await ejecutar(`
    CREATE TABLE IF NOT EXISTS reservations (
      id SERIAL PRIMARY KEY,
      "userId" INTEGER,
      "userName" TEXT,
      "classId" INTEGER,
      "claseTitulo" TEXT,
      instructor TEXT,
      lugares TEXT,
      fecha TEXT,
      hora TEXT,
      monto REAL DEFAULT 0,
      estado TEXT DEFAULT 'confirmada',
      creado TIMESTAMP DEFAULT NOW()
    );
  `);

  // Libro de ingresos: una fila por cada pago real (matrícula, renovación).
  // Sin clave foránea a memberships a propósito: si el admin borra la ficha
  // de un socio, su dinero ya cobrado sigue contando en los reportes.
  await ejecutar(`
    CREATE TABLE IF NOT EXISTS payments (
      id SERIAL PRIMARY KEY,
      tipo TEXT NOT NULL DEFAULT 'matricula',
      membership_id INTEGER,
      nombre TEXT NOT NULL DEFAULT '',
      concepto TEXT DEFAULT '',
      monto NUMERIC(12,2) NOT NULL CHECK (monto >= 0),
      metodo TEXT DEFAULT 'Efectivo',
      fecha_pago TEXT NOT NULL,
      notas TEXT DEFAULT '',
      creado TIMESTAMPTZ DEFAULT NOW()
    );
  `);

  // Catálogo editable por el admin: opciones de matrícula/promos (con precio
  // y duración) y categorías de ventas extras. Quitar una opción NO toca el
  // historial: memberships y payments guardan el concepto como texto.
  await ejecutar(`
    CREATE TABLE IF NOT EXISTS catalog_options (
      id SERIAL PRIMARY KEY,
      tipo TEXT NOT NULL CHECK (tipo IN ('matricula', 'extra')),
      nombre TEXT NOT NULL,
      precio NUMERIC(12,2) CHECK (precio IS NULL OR precio >= 0),
      dias INTEGER CHECK (dias IS NULL OR (dias >= 1 AND dias <= 366)),
      creado TIMESTAMPTZ DEFAULT NOW(),
      UNIQUE (tipo, nombre)
    );
  `);

  // REAL (float binario) acumula errores de centavos con los años.
  // NUMERIC(12,2) es exacto. ROUND limpia cualquier artefacto previo.
  await runMigration(ejecutar, '001_montos_a_numeric', async () => {
    await ejecutar(`ALTER TABLE memberships ALTER COLUMN monto TYPE NUMERIC(12,2) USING ROUND(monto::numeric, 2)`);
    await ejecutar(`ALTER TABLE extra_sales ALTER COLUMN monto TYPE NUMERIC(12,2) USING ROUND(monto::numeric, 2)`);
    await ejecutar(`ALTER TABLE classes ALTER COLUMN precio TYPE NUMERIC(12,2) USING ROUND(precio::numeric, 2)`);
  });

  // Las membresías que ya existían pasan al libro de ingresos como un pago
  // en su fecha de pago original (igual que las contaban los reportes viejos).
  await runMigration(ejecutar, '002_backfill_payments', async () => {
    await ejecutar(`
      INSERT INTO payments (tipo, membership_id, nombre, concepto, monto, metodo, fecha_pago, creado)
      SELECT 'matricula', m.id, m.nombre, m.concepto, ROUND(m.monto::numeric, 2), m.metodo, m.fecha_pago,
             COALESCE(m.creado, NOW())
      FROM memberships m
      WHERE NOT EXISTS (SELECT 1 FROM payments p WHERE p.membership_id = m.id)
    `);
  });

  // Anular un pago mal registrado sin borrar el historial: queda la fila con
  // su motivo y deja de sumar en los reportes. Antes, un pago tecleado con el
  // monto equivocado quedaba en las finanzas para siempre.
  await runMigration(ejecutar, '004_pagos_anulables', async () => {
    await ejecutar(`ALTER TABLE payments ADD COLUMN IF NOT EXISTS anulado BOOLEAN NOT NULL DEFAULT FALSE`);
    await ejecutar(`ALTER TABLE payments ADD COLUMN IF NOT EXISTS motivo_anulacion TEXT DEFAULT ''`);
    await ejecutar(`ALTER TABLE extra_sales ADD COLUMN IF NOT EXISTS anulado BOOLEAN NOT NULL DEFAULT FALSE`);
    await ejecutar(`ALTER TABLE extra_sales ADD COLUMN IF NOT EXISTS motivo_anulacion TEXT DEFAULT ''`);
  });

  // Precios iniciales del catálogo: los 4 conceptos históricos del sistema
  // más la lista real del flyer del gym. El admin puede quitar o agregar
  // los que quiera desde la pestaña "Precios y promos".
  await runMigration(ejecutar, '003_seed_catalogo', async () => {
    const matriculas = [
      ['Matrícula mensual', 80, 30],
      ['Matrícula quincenal', 50, 15],
      ['Matrícula semanal', 25, 7],
      ['Pase EX LOCAL', 15, 1],
      ['Máquinas 2 meses (promo)', 130, 60],
      ['Máquinas 3 meses (promo)', 210, 90],
      ['Máquinas + Aeróbicos 1 mes', 100, 30],
      ['Aeróbicos 1 mes', 70, 30],
      ['Aeróbicos 2 personas (promo)', 120, 30],
    ];
    for (const [nombre, precio, dias] of matriculas) {
      await ejecutar(
        `INSERT INTO catalog_options (tipo, nombre, precio, dias) VALUES ('matricula', $1, $2, $3)
         ON CONFLICT (tipo, nombre) DO NOTHING`,
        [nombre, precio, dias]
      );
    }
    const extras = ['Aguas/Bebidas', 'Energizantes', 'Suplementos', 'Proteína', 'Creatina', 'Ropa', 'Toallas', 'Otros'];
    for (const nombre of extras) {
      await ejecutar(
        `INSERT INTO catalog_options (tipo, nombre) VALUES ('extra', $1)
         ON CONFLICT (tipo, nombre) DO NOTHING`,
        [nombre]
      );
    }
  });

  // Índices: con años de datos las consultas de reportes siguen siendo instantáneas.
  await ejecutar(`CREATE INDEX IF NOT EXISTS idx_memberships_fecha_vence ON memberships (fecha_vence)`);
  await ejecutar(`CREATE INDEX IF NOT EXISTS idx_memberships_estado ON memberships (estado)`);
  await ejecutar(`CREATE INDEX IF NOT EXISTS idx_payments_fecha_pago ON payments (fecha_pago)`);
  await ejecutar(`CREATE INDEX IF NOT EXISTS idx_payments_membership ON payments (membership_id)`);
  await ejecutar(`CREATE INDEX IF NOT EXISTS idx_extra_sales_fecha ON extra_sales (fecha)`);
  // Búsqueda de socios por nombre sin distinguir mayúsculas: con miles de
  // fichas el buscador sigue respondiendo al instante.
  await ejecutar(`CREATE INDEX IF NOT EXISTS idx_memberships_nombre_lower ON memberships (LOWER(nombre))`);
  await ejecutar(`CREATE INDEX IF NOT EXISTS idx_memberships_dni ON memberships (dni)`);

  await insertarAdminSiNoExiste(ejecutar);
  await restablecerAdminSiSePidio(ejecutar);
  console.log('[DB] Esquema listo ✓');
}

// Reintenta para siempre con espera creciente: si Supabase tarda en responder
// al arrancar (o está caído un rato), el server espera y se conecta solo,
// en vez de morir y depender de reinicios externos.
async function initDBConReintentos() {
  let intento = 0;
  let ajustesSSL = 0; // máximo 2 cambios de modo; después aplica la espera normal
  for (;;) {
    if (detenido) return;
    try {
      await initDB();
      dbReady = true;
      return;
    } catch (err) {
      if (detenido) return;
      const msg = err.message || '';
      // Desajuste de SSL con el servidor: se invierte el modo UNA vez y se
      // reintenta al instante con un pool nuevo. Cubre ambos sentidos:
      // "The server does not support SSL connections" (Railway interno) y
      // "SSL connection is required" / "no encryption" (Supabase y similares).
      const quitarSSL = usarSSL && /does not support SSL/i.test(msg);
      const ponerSSL = !usarSSL && /(SSL[\s\S]*required|required[\s\S]*SSL|no encryption)/i.test(msg);
      if ((quitarSSL || ponerSSL) && ajustesSSL < 2) {
        ajustesSSL++;
        usarSSL = ponerSSL;
        console.warn(`[DB] El servidor ${usarSSL ? 'exige' : 'no soporta'} SSL: reconectando ${usarSSL ? 'con' : 'sin'} SSL...`);
        const anterior = pool;
        pool = crearPool();
        anterior.end().catch(() => {});
        continue;
      }
      intento++;
      const esperaMs = Math.min(30000, 2000 * 2 ** Math.min(intento - 1, 4));
      console.error(`[DB] Sin conexión (intento ${intento}): ${msg} — reintento en ${esperaMs / 1000}s`);
      await new Promise((r) => setTimeout(r, esperaMs));
    }
  }
}

async function insertarAdminSiNoExiste(ejecutar) {
  const r = await ejecutar('SELECT COUNT(*)::int AS count FROM users');
  const res = r.rows[0];
  if (res && res.count === 0) {
    // Sin ADMIN_PASSWORD se genera una contraseña ALEATORIA y se muestra en
    // los logs del arranque. Antes se usaba "admin123", que está escrita en
    // el código publicado: cualquiera que encontrara la web podía entrar.
    const generada = !config.ADMIN_PASSWORD;
    const passwordInicial = config.ADMIN_PASSWORD ||
      ('ZonaVip-' + require('crypto').randomBytes(6).toString('base64url'));
    const hash = await bcrypt.hash(passwordInicial, 10);
    await ejecutar(
      `INSERT INTO users (firstname, lastname, email, password, role) VALUES ($1, $2, $3, $4, $5)`,
      ['Administradora', 'ZONA VIP GYM', config.ADMIN_EMAIL, hash, 'admin']
    );
    console.log(`[DB] Admin creado: ${config.ADMIN_EMAIL}`);
    if (generada) {
      console.warn('╔══════════════════════════════════════════════════════════╗');
      console.warn('║ No definiste ADMIN_PASSWORD, así que se generó una clave  ');
      console.warn('║ aleatoria para el primer acceso. ANÓTALA AHORA:           ');
      console.warn(`║     Usuario:    ${config.ADMIN_EMAIL}`);
      console.warn(`║     Contraseña: ${passwordInicial}`);
      console.warn('║ Entra al panel y cámbiala desde "Mi cuenta".              ');
      console.warn('╚══════════════════════════════════════════════════════════╝');
    }
  }
}

// Salida de emergencia: si la dueña olvida su contraseña, define
// ADMIN_RESET_PASSWORD en Railway → Variables y reinicia. Al arrancar, la
// contraseña queda restablecida y el log le recuerda borrar la variable.
// Sin esto, olvidar la contraseña dejaba el sistema cerrado para siempre.
async function restablecerAdminSiSePidio(ejecutar) {
  if (!config.ADMIN_RESET_PASSWORD) return;
  const hash = await bcrypt.hash(config.ADMIN_RESET_PASSWORD, 10);
  const r = await ejecutar(
    `UPDATE users SET password = $1 WHERE email = $2 RETURNING id`,
    [hash, config.ADMIN_EMAIL]
  );
  if (r.rowCount > 0) {
    console.warn('╔══════════════════════════════════════════════════════════╗');
    console.warn(`║ CONTRASEÑA RESTABLECIDA para ${config.ADMIN_EMAIL}`);
    console.warn('║ Entra al sistema y luego BORRA la variable                ');
    console.warn('║ ADMIN_RESET_PASSWORD en Railway → Variables.              ');
    console.warn('╚══════════════════════════════════════════════════════════╝');
  } else {
    console.warn(`[DB] ADMIN_RESET_PASSWORD definida pero no existe el usuario ${config.ADMIN_EMAIL}.`);
  }
}

// Mantiene el campo `estado` coherente con la fecha de vencimiento, en ambos
// sentidos (también re-activa si quedó mal marcada). YA NO BORRA NADA:
// el borrado automático de la versión anterior destruía el historial.
//
// Se ejecuta como mucho una vez por minuto: antes se lanzaban dos UPDATE en
// CADA lectura de la lista de socios, y el buscador dispara una lectura por
// cada tecla. Con miles de fichas eso era escribir en la base decenas de
// veces por segundo sin ninguna necesidad (el estado solo cambia de día a día).
let ultimoRefresco = 0;
const REFRESCO_MS = 60 * 1000;

async function actualizarEstadoMemberships(forzar = false) {
  const ahora = Date.now();
  if (!forzar && ahora - ultimoRefresco < REFRESCO_MS) return;
  ultimoRefresco = ahora;
  const hoy = todayISO(config.TZ);
  try {
    await query(`UPDATE memberships SET estado = 'vencida' WHERE fecha_vence < $1 AND estado = 'activa'`, [hoy]);
    await query(`UPDATE memberships SET estado = 'activa' WHERE fecha_vence >= $1 AND estado = 'vencida'`, [hoy]);
  } catch (err) {
    // Si falla (base caída), se reintenta en la próxima llamada: no se
    // "quema" la ventana de un minuto con un intento fallido.
    ultimoRefresco = 0;
    throw err;
  }
}

// El pool puede recrearse (ajuste de SSL), así que el apagado pasa por aquí
// y no por una referencia directa que podría quedar obsoleta.
function cerrarPool() {
  return pool.end();
}

module.exports = {
  query, queryOne, queryAll, withTransaction,
  initDB, initDBConReintentos, isDBReady, detenerReintentos,
  actualizarEstadoMemberships, cerrarPool,
};
