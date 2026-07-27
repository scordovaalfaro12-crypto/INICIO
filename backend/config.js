// ==========================================================
//  config.js — Configuración central del sistema
//  ÚNICA fuente de verdad para variables de entorno.
//  Antes el secreto JWT estaba duplicado en dos archivos con
//  valores distintos: si faltaba la variable JWT_SECRET el
//  login emitía tokens que el middleware rechazaba.
// ==========================================================
require('dotenv').config();

const warnings = [];

// El MISMO secreto se usa para firmar y verificar tokens.
//
// Si falta la variable se genera uno ALEATORIO en cada arranque. Antes se
// usaba una clave fija escrita en el código: como el código está en GitHub,
// cualquiera podía fabricarse un token de administradora y entrar al sistema
// sin saber la contraseña. Con una clave aleatoria el peor caso es tener que
// volver a iniciar sesión tras un reinicio, que no es ningún riesgo.
let JWT_SECRET = process.env.JWT_SECRET;
if (!JWT_SECRET || JWT_SECRET.length < 16) {
  JWT_SECRET = require('crypto').randomBytes(48).toString('hex');
  warnings.push(
    'JWT_SECRET no está definida: se generó una clave temporal distinta para este arranque. ' +
    'El sistema funciona, pero habrá que iniciar sesión otra vez cada vez que se reinicie. ' +
    'Define JWT_SECRET en Railway → Variables (64 caracteres aleatorios) para evitarlo.'
  );
}

// Zona horaria del negocio. El gym está en Lima; el servidor (Railway) corre
// en UTC, así que "hoy" se calcula SIEMPRE con esta zona, no con el reloj del server.
let TZ = process.env.GYM_TZ || 'America/Lima';
try {
  new Intl.DateTimeFormat('en-CA', { timeZone: TZ });
} catch (e) {
  warnings.push(`GYM_TZ inválida ("${TZ}"). Se usará America/Lima.`);
  TZ = 'America/Lima';
}

// Salida de emergencia si se olvida la contraseña del administrador.
// ADMIN_PASSWORD solo sirve la PRIMERA vez (con la base vacía); sin esto,
// olvidar la contraseña dejaba el sistema inaccesible para siempre.
// Al definir ADMIN_RESET_PASSWORD en Railway, el siguiente arranque la
// restablece y avisa en los logs que la variable debe borrarse.
const ADMIN_RESET_PASSWORD = process.env.ADMIN_RESET_PASSWORD || '';
if (ADMIN_RESET_PASSWORD && ADMIN_RESET_PASSWORD.length < 6) {
  warnings.push('ADMIN_RESET_PASSWORD es demasiado corta (mínimo 6 caracteres): se ignorará.');
}

// Orígenes web autorizados a llamar la API desde OTRO dominio. Vacío = solo
// este mismo sitio, que es lo normal: el panel se sirve del propio servidor.
const ALLOWED_ORIGINS = (process.env.ALLOWED_ORIGINS || '')
  .split(',').map((s) => s.trim()).filter(Boolean);

const config = {
  PORT: parseInt(process.env.PORT, 10) || 3000,
  DATABASE_URL: process.env.DATABASE_URL || '',
  JWT_SECRET,
  TOKEN_TTL: process.env.TOKEN_TTL || '24h',
  TZ,
  ADMIN_EMAIL: (process.env.ADMIN_EMAIL || 'admin@zonavip.com').toLowerCase().trim(),
  ADMIN_PASSWORD: process.env.ADMIN_PASSWORD || '',
  ADMIN_RESET_PASSWORD: ADMIN_RESET_PASSWORD.length >= 6 ? ADMIN_RESET_PASSWORD : '',
  ALLOWED_ORIGINS,
  NODE_ENV: process.env.NODE_ENV || 'development',
  warnings,
};

if (!config.DATABASE_URL) {
  warnings.push('DATABASE_URL no está definida: no habrá conexión a la base de datos hasta configurarla.');
}

module.exports = config;
