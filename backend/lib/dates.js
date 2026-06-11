// ==========================================================
//  lib/dates.js — Fechas a prueba de zona horaria
//  Todas las fechas del sistema son strings 'YYYY-MM-DD' y la
//  aritmética se hace en UTC puro: el resultado no depende del
//  reloj ni la zona del servidor. (El bug clásico era usar
//  new Date().toISOString(), que en Perú a partir de las 7pm
//  ya devuelve la fecha de MAÑANA.)
// ==========================================================

const ISO_RE = /^\d{4}-\d{2}-\d{2}$/;

function isISODate(s) {
  if (typeof s !== 'string' || !ISO_RE.test(s)) return false;
  const [y, m, d] = s.split('-').map(Number);
  if (y < 2000 || y > 2200) return false;
  const dt = new Date(Date.UTC(y, m - 1, d));
  return dt.getUTCFullYear() === y && dt.getUTCMonth() === m - 1 && dt.getUTCDate() === d;
}

// "Hoy" en la zona horaria del negocio, sin importar dónde corra el servidor.
// 'en-CA' formatea como YYYY-MM-DD.
function todayISO(tz) {
  return new Intl.DateTimeFormat('en-CA', {
    timeZone: tz, year: 'numeric', month: '2-digit', day: '2-digit',
  }).format(new Date());
}

function toUTC(iso) {
  const [y, m, d] = iso.split('-').map(Number);
  return new Date(Date.UTC(y, m - 1, d));
}

function addDays(iso, n) {
  const dt = toUTC(iso);
  dt.setUTCDate(dt.getUTCDate() + n);
  return dt.toISOString().slice(0, 10);
}

// a - b, en días enteros
function diffDays(a, b) {
  return Math.round((toUTC(a) - toUTC(b)) / 86400000);
}

// Lunes de la semana a la que pertenece `iso`
function weekStart(iso) {
  const dt = toUTC(iso);
  const day = dt.getUTCDay(); // 0 = domingo
  dt.setUTCDate(dt.getUTCDate() + (day === 0 ? -6 : 1 - day));
  return dt.toISOString().slice(0, 10);
}

function monthStart(iso) { return iso.slice(0, 8) + '01'; }

function yearStart(iso) { return iso.slice(0, 4) + '-01-01'; }

// Quincena: del 1 al 15, o del 16 a fin de mes
function quincenaStart(iso) {
  const d = Number(iso.slice(8, 10));
  return iso.slice(0, 8) + (d <= 15 ? '01' : '16');
}

module.exports = {
  isISODate, todayISO, addDays, diffDays,
  weekStart, monthStart, yearStart, quincenaStart,
};
