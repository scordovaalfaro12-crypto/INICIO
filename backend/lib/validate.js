// ==========================================================
//  lib/validate.js — Validación de entradas
//  Nada llega a la base de datos sin pasar por aquí. Un dato
//  malformado (fecha en otro formato, monto "abc", NaN, texto
//  kilométrico) se rechaza con error claro en vez de guardarse
//  y corromper los reportes a futuro.
// ==========================================================

const { isISODate } = require('./dates');

const MAX_MONTO = 1000000; // tope de S/ 1,000,000 por operación

// Convierte a número SOLO lo que de verdad es un número o un texto numérico.
// Sin esto, Number() aceptaba cosas absurdas: Number(true) = 1 guardaba un
// cobro de S/ 1, y Number(' ') = 0 y Number([]) = 0 guardaban cobros de S/ 0
// en vez de rechazar la petición y avisar del error.
function aNumero(v) {
  if (typeof v === 'number') return Number.isFinite(v) ? v : null;
  if (typeof v !== 'string') return null;
  const s = v.trim();
  if (s === '' || !/^[+-]?(\d+(\.\d+)?|\.\d+)$/.test(s)) return null;
  const n = Number(s);
  return Number.isFinite(n) ? n : null;
}

// Monto como número con 2 decimales, o null si es inválido.
function parseMonto(v) {
  if (v === null || v === undefined || v === '') return null;
  const n = aNumero(v);
  if (n === null || n < 0 || n > MAX_MONTO) return null;
  return Math.round(n * 100) / 100;
}

function parseId(v) {
  const n = aNumero(v);
  return (n !== null && Number.isInteger(n) && n > 0 && n <= 2147483647) ? n : null;
}

function parseDias(v, min = 1, max = 366) {
  const n = aNumero(v);
  return (n !== null && Number.isInteger(n) && n >= min && n <= max) ? n : null;
}

// Texto saneado: sin caracteres de control, recortado y con tope de longitud.
function cleanText(v, max = 200) {
  if (v === null || v === undefined) return '';
  return String(v)
    .replace(/[\u0000-\u0008\u000B\u000C\u000E-\u001F\u007F]/g, '')
    .trim()
    .slice(0, max);
}

// Igual que cleanText pero el campo es obligatorio: null si queda vacío.
function requireText(v, max = 200) {
  const s = cleanText(v, max);
  return s.length > 0 ? s : null;
}

function parseFecha(v) {
  const s = cleanText(v, 10);
  return isISODate(s) ? s : null;
}

const METODOS = ['Efectivo', 'Yape', 'Plin', 'Tarjeta', 'Transferencia', 'Otro'];

function parseMetodo(v) {
  const s = cleanText(v, 30);
  if (!s) return 'Efectivo';
  return METODOS.includes(s) ? s : 'Otro';
}

// Teléfono peruano listo para WhatsApp: solo dígitos, con prefijo 51 si es
// un celular local de 9 cifras. Devuelve null si no parece un número usable.
function parseTelefonoWhatsApp(v) {
  const solo = cleanText(v, 30).replace(/\D/g, '');
  if (!solo) return null;
  if (solo.length === 9 && solo.startsWith('9')) return '51' + solo;
  if (solo.length === 11 && solo.startsWith('51')) return solo;
  if (solo.length >= 8 && solo.length <= 15) return solo;
  return null;
}

module.exports = {
  parseMonto, parseId, parseDias, cleanText, requireText,
  parseFecha, parseMetodo, parseTelefonoWhatsApp, aNumero, MAX_MONTO, METODOS,
};
