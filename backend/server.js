// ==========================================================
//  server.js — ZONA VIP GYM
//  Diseñado para correr años sin intervención:
//  - Si la base de datos no responde al arrancar, reintenta
//    solo (no muere esperando reinicios externos).
//  - Cierre ordenado en redeploys de Railway (SIGTERM).
//  - Solo se sirve la carpeta public/: el código fuente del
//    servidor ya no queda descargable por la web.
// ==========================================================

const express = require('express');
const cors = require('cors');
const path = require('path');
const config = require('./config');
const db = require('./db');
const { securityHeaders } = require('./middleware/security');

const authRoutes = require('./routes/auth');
const classRoutes = require('./routes/classes');
const membershipRoutes = require('./routes/memberships');
const extraRoutes = require('./routes/extras');
const backupRoutes = require('./routes/backup');

const app = express();

// Railway pone un proxy delante: con esto req.ip es la IP real del visitante
// (necesario para que el límite de intentos de login funcione por persona).
app.set('trust proxy', 1);
app.disable('x-powered-by');

app.use(securityHeaders);
app.use(cors());
app.use(express.json({ limit: '200kb' }));

// Salud del sistema: Railway puede usar /api/health como healthcheck.
app.get('/api/health', async (req, res) => {
  if (!db.isDBReady()) {
    return res.status(503).json({ status: 'INICIANDO', db: 'conectando', timestamp: new Date().toISOString() });
  }
  try {
    await Promise.race([
      db.query('SELECT 1'),
      new Promise((_, rej) => setTimeout(() => rej(new Error('timeout')), 2500)),
    ]);
    res.json({ status: 'OK', db: 'ok', timestamp: new Date().toISOString() });
  } catch (e) {
    res.status(503).json({ status: 'ERROR', db: 'sin conexión', timestamp: new Date().toISOString() });
  }
});

// Mientras la base de datos inicia, la API responde 503 claro (no errores raros).
app.use('/api', (req, res, next) => {
  if (!db.isDBReady()) {
    return res.status(503).json({ error: 'El sistema está iniciando. Intenta de nuevo en unos segundos.' });
  }
  next();
});

app.use('/api/auth', authRoutes);
app.use('/api/classes', classRoutes);
app.use('/api/memberships', membershipRoutes);
app.use('/api/extras', extraRoutes);
app.use('/api/backup', backupRoutes);

// Frontend: SOLO la carpeta public (antes se servía todo el backend,
// incluido el código fuente del servidor).
const PUBLIC_PATH = path.join(__dirname, 'public');
app.use(express.static(PUBLIC_PATH, {
  setHeaders: (res, filePath) => {
    // HTML y service worker siempre frescos; el resto puede cachearse un poco.
    if (filePath.endsWith('.html') || filePath.endsWith('sw.js')) {
      res.setHeader('Cache-Control', 'no-cache');
    } else {
      res.setHeader('Cache-Control', 'public, max-age=300');
    }
  },
}));

app.get('*', (req, res) => {
  if (req.path.startsWith('/api/')) {
    return res.status(404).json({ error: 'Endpoint no encontrado' });
  }
  res.sendFile(path.join(PUBLIC_PATH, 'login.html'));
});

// Red de seguridad final: cualquier error no manejado en una ruta devuelve
// JSON limpio en vez de tumbar la petición con un stack trace.
app.use((err, req, res, next) => {
  if (err && err.type === 'entity.parse.failed') {
    return res.status(400).json({ error: 'JSON inválido en la petición' });
  }
  if (err && err.type === 'entity.too.large') {
    return res.status(413).json({ error: 'Petición demasiado grande' });
  }
  console.error('[HTTP] Error no controlado:', err);
  if (res.headersSent) return next(err);
  res.status(500).json({ error: 'Error interno del servidor' });
});

const server = app.listen(config.PORT, '0.0.0.0', () => {
  console.log('');
  console.log('╔════════════════════════════════════════════╗');
  console.log('║     ZONA VIP GYM — SISTEMA EN LA NUBE ✓    ║');
  console.log('╚════════════════════════════════════════════╝');
  console.log(`  Puerto: ${config.PORT} · Modo: ${config.NODE_ENV} · Zona horaria: ${config.TZ}`);
  config.warnings.forEach((w) => console.warn(`  ⚠ ${w}`));
  console.log('');
});

// La base de datos se conecta en paralelo y reintenta sola si hace falta.
db.initDBConReintentos().then(() => {
  console.log('[DB] Base de datos conectada y lista ✓');
  // Refresco de estados cada hora: las membresías pasan a "vencida" a tiempo
  // aunque nadie abra el panel ese día.
  setInterval(() => {
    db.actualizarEstadoMemberships().catch((e) => console.error('[CRON] Estados:', e.message));
  }, 60 * 60 * 1000).unref();
});

// ===== Blindaje del proceso =====
process.on('unhandledRejection', (err) => {
  console.error('[PROCESO] Promesa rechazada sin manejar (recuperado):', err);
});

process.on('uncaughtException', (err) => {
  console.error('[PROCESO] Excepción no capturada, cerrando para reiniciar limpio:', err);
  shutdown(1);
});

process.on('SIGTERM', () => shutdown(0)); // redeploy de Railway
process.on('SIGINT', () => shutdown(0));  // Ctrl+C local

let cerrando = false;
function shutdown(code) {
  if (cerrando) return;
  cerrando = true;
  console.log('[PROCESO] Cerrando ordenadamente...');
  server.close(() => {
    db.pool.end()
      .catch(() => {})
      .finally(() => process.exit(code));
  });
  // Si algo se queda colgado, salida forzada a los 8 segundos.
  setTimeout(() => process.exit(code), 8000).unref();
}
