// ==========================================================
//  lib/validate.js — Validación de entradas
//  Nada llega a la base de datos sin pasar por aquí. Un dato
//  malformado (fecha en otro formato, monto "abc", NaN, texto
//  kilométrico) se rechaza con error claro en vez de guardarse
//  y corromper los reportes a futuro.
// ==========================================================

const { isISODate } = require('./dates');

const MAX_MONTO = 1000000; // tope de S/ 1,000,000 por operación

// Monto como número con 2 decimales, o null si es inválido.
function parseMonto(v) {
  if (v === null || v === undefined || v === '') return null;
  const n = Number(v);
  if (!Number.isFinite(n) || n < 0 || n > MAX_MONTO) return null;
  return Math.round(n * 100) / 100;
}

function parseId(v) {
  const n = Number(v);
  return (Number.isInteger(n) && n > 0 && n <= 2147483647) ? n : null;
}

function parseDias(v, min = 1, max = 366) {
  const n = Number(v);
  return (Number.isInteger(n) && n >= min && n <= max) ? n : null;
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

module.exports = {
  parseMonto, parseId, parseDias, cleanText, requireText,
  parseFecha, parseMetodo, MAX_MONTO, METODOS,
};
