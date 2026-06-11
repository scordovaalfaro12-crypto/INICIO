// ==========================================================
//  middleware/security.js — Cabeceras de seguridad y freno
//  de fuerza bruta en el login. Sin dependencias nuevas.
// ==========================================================

function securityHeaders(req, res, next) {
  res.setHeader('X-Content-Type-Options', 'nosniff');
  res.setHeader('X-Frame-Options', 'DENY');
  res.setHeader('Referrer-Policy', 'no-referrer');
  if (req.secure || req.headers['x-forwarded-proto'] === 'https') {
    res.setHeader('Strict-Transport-Security', 'max-age=15552000; includeSubDomains');
  }
  if (req.path.startsWith('/api/')) {
    // Las respuestas de la API nunca deben quedar cacheadas por navegador/proxy.
    res.setHeader('Cache-Control', 'no-store');
  }
  next();
}

// Límite de intentos de login por IP, en memoria (el sistema corre en una
// sola instancia). 10 fallos en 10 minutos → bloqueo de 15 minutos.
const intentos = new Map();
const VENTANA_MS = 10 * 60 * 1000;
const MAX_FALLOS = 10;
const BLOQUEO_MS = 15 * 60 * 1000;

setInterval(() => {
  const ahora = Date.now();
  for (const [ip, reg] of intentos) {
    if (ahora - reg.inicio > VENTANA_MS && ahora > (reg.bloqueadoHasta || 0)) intentos.delete(ip);
  }
}, 5 * 60 * 1000).unref();

function loginRateLimit(req, res, next) {
  const ip = req.ip || 'desconocida';
  const ahora = Date.now();
  let reg = intentos.get(ip);

  if (reg && reg.bloqueadoHasta && ahora < reg.bloqueadoHasta) {
    const min = Math.ceil((reg.bloqueadoHasta - ahora) / 60000);
    return res.status(429).json({ error: `Demasiados intentos fallidos. Espera ${min} minuto(s).` });
  }
  if (!reg || ahora - reg.inicio > VENTANA_MS) {
    reg = { inicio: ahora, fallos: 0, bloqueadoHasta: 0 };
    intentos.set(ip, reg);
  }

  req.loginFallido = () => {
    reg.fallos++;
    if (reg.fallos >= MAX_FALLOS) reg.bloqueadoHasta = Date.now() + BLOQUEO_MS;
  };
  req.loginExitoso = () => intentos.delete(ip);
  next();
}

module.exports = { securityHeaders, loginRateLimit };
