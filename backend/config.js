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
let JWT_SECRET = process.env.JWT_SECRET;
if (!JWT_SECRET || JWT_SECRET.length < 16) {
  JWT_SECRET = 'zonavip_clave_por_defecto_INSEGURA_definir_JWT_SECRET';
  warnings.push(
    'JWT_SECRET no está definida (o es muy corta). El sistema funciona igual, ' +
    'pero define JWT_SECRET en Railway → Variables (64 caracteres aleatorios) para mayor seguridad.'
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

const config = {
  PORT: parseInt(process.env.PORT, 10) || 3000,
  DATABASE_URL: process.env.DATABASE_URL || '',
  JWT_SECRET,
  TOKEN_TTL: process.env.TOKEN_TTL || '24h',
  TZ,
  ADMIN_EMAIL: (process.env.ADMIN_EMAIL || 'admin@zonavip.com').toLowerCase().trim(),
  ADMIN_PASSWORD: process.env.ADMIN_PASSWORD || '',
  NODE_ENV: process.env.NODE_ENV || 'development',
  warnings,
};

if (!config.DATABASE_URL) {
  warnings.push('DATABASE_URL no está definida: no habrá conexión a la base de datos hasta configurarla.');
}

module.exports = config;
