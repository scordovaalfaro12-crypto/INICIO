// ==========================================================
//  db.js — Conexión PostgreSQL (Supabase) + esquema + migraciones
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

const esLocal = /localhost|127\.0\.0\.1/.test(config.DATABASE_URL);

const pool = new Pool({
  connectionString: config.DATABASE_URL,
  ssl: esLocal ? false : { rejectUnauthorized: false },
  max: 5,                       // Supabase free limita conexiones: pocas y estables
  idleTimeoutMillis: 30000,
  connectionTimeoutMillis: 10000,
  keepAlive: true,
});

// CRÍTICO: sin este handler, un corte de red en una conexión inactiva
// emite 'error' sin listener y TUMBA el proceso completo. Es la causa
// número 1 de caídas de Node + Postgres en la nube.
pool.on('error', (err) => {
  console.error('[DB] Error en conexión inactiva (recuperado):', err.message);
});

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

async function runMigration(id, fn) {
  const aplicada = await queryOne('SELECT id FROM schema_migrations WHERE id = $1', [id]);
  if (aplicada) return;
  await fn();
  await query('INSERT INTO schema_migrations (id) VALUES ($1) ON CONFLICT (id) DO NOTHING', [id]);
  console.log(`[DB] Migración aplicada: ${id}`);
}

async function initDB() {
  console.log('[DB] Verificando esquema...');

  await query(`
    CREATE TABLE IF NOT EXISTS schema_migrations (
      id TEXT PRIMARY KEY,
      aplicada TIMESTAMPTZ DEFAULT NOW()
    );
  `);

  await query(`
    CREATE TABLE IF NOT EXISTS users (
      id SERIAL PRIMARY KEY,
      firstname TEXT NOT NULL,
      lastname TEXT NOT NULL,
      email TEXT UNIQUE NOT NULL,
      password TEXT NOT NULL,
      role TEXT DEFAULT 'admin'
    );
  `);

  await query(`
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

  await query(`
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

  await query(`
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
  await query(`
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
  await query(`
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

  // REAL (float binario) acumula errores de centavos con los años.
  // NUMERIC(12,2) es exacto. ROUND limpia cualquier artefacto previo.
  await runMigration('001_montos_a_numeric', async () => {
    await query(`ALTER TABLE memberships ALTER COLUMN monto TYPE NUMERIC(12,2) USING ROUND(monto::numeric, 2)`);
    await query(`ALTER TABLE extra_sales ALTER COLUMN monto TYPE NUMERIC(12,2) USING ROUND(monto::numeric, 2)`);
    await query(`ALTER TABLE classes ALTER COLUMN precio TYPE NUMERIC(12,2) USING ROUND(precio::numeric, 2)`);
  });

  // Las membresías que ya existían pasan al libro de ingresos como un pago
  // en su fecha de pago original (igual que las contaban los reportes viejos).
  await runMigration('002_backfill_payments', async () => {
    await query(`
      INSERT INTO payments (tipo, membership_id, nombre, concepto, monto, metodo, fecha_pago, creado)
      SELECT 'matricula', m.id, m.nombre, m.concepto, ROUND(m.monto::numeric, 2), m.metodo, m.fecha_pago,
             COALESCE(m.creado, NOW())
      FROM memberships m
      WHERE NOT EXISTS (SELECT 1 FROM payments p WHERE p.membership_id = m.id)
    `);
  });

  // Índices: con años de datos las consultas de reportes siguen siendo instantáneas.
  await query(`CREATE INDEX IF NOT EXISTS idx_memberships_fecha_vence ON memberships (fecha_vence)`);
  await query(`CREATE INDEX IF NOT EXISTS idx_memberships_estado ON memberships (estado)`);
  await query(`CREATE INDEX IF NOT EXISTS idx_payments_fecha_pago ON payments (fecha_pago)`);
  await query(`CREATE INDEX IF NOT EXISTS idx_payments_membership ON payments (membership_id)`);
  await query(`CREATE INDEX IF NOT EXISTS idx_extra_sales_fecha ON extra_sales (fecha)`);

  await insertarAdminSiNoExiste();
  console.log('[DB] Esquema listo ✓');
}

// Reintenta para siempre con espera creciente: si Supabase tarda en responder
// al arrancar (o está caído un rato), el server espera y se conecta solo,
// en vez de morir y depender de reinicios externos.
async function initDBConReintentos() {
  let intento = 0;
  for (;;) {
    try {
      await initDB();
      dbReady = true;
      return;
    } catch (err) {
      intento++;
      const esperaMs = Math.min(30000, 2000 * 2 ** Math.min(intento - 1, 4));
      console.error(`[DB] Sin conexión (intento ${intento}): ${err.message} — reintento en ${esperaMs / 1000}s`);
      await new Promise((r) => setTimeout(r, esperaMs));
    }
  }
}

async function insertarAdminSiNoExiste() {
  const res = await queryOne('SELECT COUNT(*)::int AS count FROM users');
  if (res && res.count === 0) {
    const passwordInicial = config.ADMIN_PASSWORD || 'admin123';
    const hash = await bcrypt.hash(passwordInicial, 10);
    await query(
      `INSERT INTO users (firstname, lastname, email, password, role) VALUES ($1, $2, $3, $4, $5)`,
      ['Administradora', 'ZONA VIP GYM', config.ADMIN_EMAIL, hash, 'admin']
    );
    console.log(`[DB] Admin creado: ${config.ADMIN_EMAIL}`);
    if (!config.ADMIN_PASSWORD) {
      console.warn('[DB] ⚠ Se usó la contraseña por defecto "admin123". CÁMBIALA apenas entres al sistema.');
    }
  }
}

// Mantiene el campo `estado` coherente con la fecha de vencimiento, en ambos
// sentidos (también re-activa si quedó mal marcada). YA NO BORRA NADA:
// el borrado automático de la versión anterior destruía el historial.
async function actualizarEstadoMemberships() {
  const hoy = todayISO(config.TZ);
  await query(`UPDATE memberships SET estado = 'vencida' WHERE fecha_vence < $1 AND estado = 'activa'`, [hoy]);
  await query(`UPDATE memberships SET estado = 'activa' WHERE fecha_vence >= $1 AND estado = 'vencida'`, [hoy]);
}

module.exports = {
  pool, query, queryOne, queryAll, withTransaction,
  initDB, initDBConReintentos, isDBReady, actualizarEstadoMemberships,
};
