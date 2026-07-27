// ==========================================================
//  middleware/security.js — Cabeceras de seguridad y freno
//  de fuerza bruta en el login. Sin dependencias nuevas.
// ==========================================================

// Política de contenido: el navegador solo ejecuta y carga lo que viene
// de este mismo sitio. Si algún día se colara texto malicioso en un nombre
// de socio, el navegador se niega a ejecutarlo aunque el escape fallara.
// 'unsafe-inline' se permite SOLO para estilos (el HTML usa style="...");
// para scripts NO se permite: por eso no queda ni un onclick en el HTML.
// No hay ni un solo dominio externo: todo (incluidas las fuentes) se sirve
// desde este mismo servidor, así que nada de fuera puede frenar ni espiar.
const CSP = [
  "default-src 'self'",
  "script-src 'self'",
  "style-src 'self' 'unsafe-inline'",
  "font-src 'self'",
  "img-src 'self' data: blob:",
  "connect-src 'self'",
  "form-action 'self'",
  "frame-ancestors 'none'",
  "base-uri 'self'",
  "object-src 'none'",
].join('; ');

function securityHeaders(req, res, next) {
  res.setHeader('X-Content-Type-Options', 'nosniff');
  res.setHeader('X-Frame-Options', 'DENY');
  res.setHeader('Referrer-Policy', 'no-referrer');
  res.setHeader('Content-Security-Policy', CSP);
  res.setHeader('Permissions-Policy', 'camera=(), microphone=(), geolocation=(), payment=()');
  if (req.secure || req.headers['x-forwarded-proto'] === 'https') {
    res.setHeader('Strict-Transport-Security', 'max-age=15552000; includeSubDomains');
  }
  if (req.path.startsWith('/api/')) {
    // Las respuestas de la API nunca deben quedar cacheadas por navegador/proxy.
    res.setHeader('Cache-Control', 'no-store');
  }
  next();
}

// ==========================================================
//  Freno de fuerza bruta — ESCALONADO
//
//  La versión anterior bloqueaba 15 minutos completos tras 10 fallos.
//  Problema real: hay UNA sola usuaria. Diez errores de tipeo (o el
//  teclado del celular en mayúsculas) la dejaban fuera de SU PROPIO
//  negocio 15 minutos, sin forma de entrar. Comprobado en pruebas.
//
//  Ahora la espera crece de a poco: los primeros errores casi no
//  estorban a una persona que se equivoca, pero un atacante que
//  prueba miles de contraseñas se topa con esperas cada vez mayores.
//  Un acierto borra el contador al instante.
// ==========================================================
const intentos = new Map();
const VENTANA_MS = 15 * 60 * 1000;   // los fallos se olvidan tras 15 min sin actividad
const MAX_IPS = 5000;                // techo de memoria: nunca crece sin límite

// fallos acumulados -> segundos de espera antes del siguiente intento
function esperaSegundos(fallos) {
  if (fallos < 5) return 0;      // 4 tropiezos seguidos no molestan a nadie
  if (fallos < 8) return 5;
  if (fallos < 12) return 15;
  if (fallos < 20) return 60;
  if (fallos < 40) return 300;   // 5 min
  return 900;                    // 15 min: ya no es alguien tecleando mal
}

const limpieza = setInterval(() => {
  const ahora = Date.now();
  for (const [ip, reg] of intentos) {
    if (ahora - reg.ultimo > VENTANA_MS) intentos.delete(ip);
  }
}, 5 * 60 * 1000);
limpieza.unref();

function loginRateLimit(req, res, next) {
  const ip = req.ip || 'desconocida';
  const ahora = Date.now();
  let reg = intentos.get(ip);

  if (reg && ahora - reg.ultimo > VENTANA_MS) reg = null; // expiró: borrón y cuenta nueva

  if (reg) {
    const esperaMs = esperaSegundos(reg.fallos) * 1000;
    const faltan = reg.ultimo + esperaMs - ahora;
    if (faltan > 0) {
      const seg = Math.ceil(faltan / 1000);
      const texto = seg >= 60 ? `${Math.ceil(seg / 60)} minuto(s)` : `${seg} segundo(s)`;
      res.setHeader('Retry-After', String(seg));
      return res.status(429).json({
        error: `Demasiados intentos seguidos. Espera ${texto} y vuelve a intentar.`,
        esperaSegundos: seg,
      });
    }
  }

  req.loginFallido = () => {
    const actual = intentos.get(ip) || { fallos: 0, ultimo: 0 };
    actual.fallos += 1;
    actual.ultimo = Date.now();
    // Si el mapa crece demasiado (ataque distribuido), se descarta la entrada
    // más vieja: el proceso nunca se queda sin memoria por esto.
    if (!intentos.has(ip) && intentos.size >= MAX_IPS) {
      const primera = intentos.keys().next().value;
      if (primera !== undefined) intentos.delete(primera);
    }
    intentos.set(ip, actual);
  };
  req.loginExitoso = () => intentos.delete(ip);
  next();
}

// Solo para pruebas: reinicia el estado en memoria.
function _reiniciarIntentos() { intentos.clear(); }

module.exports = { securityHeaders, loginRateLimit, esperaSegundos, _reiniciarIntentos, CSP };
