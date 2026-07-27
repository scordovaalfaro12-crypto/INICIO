const { test } = require('node:test');
const assert = require('node:assert');
const {
  parseMonto, parseId, parseDias, cleanText, requireText,
  parseFecha, parseMetodo, parseTelefonoWhatsApp,
} = require('../lib/validate');

test('parseMonto: válidos e inválidos', () => {
  assert.equal(parseMonto('80'), 80);
  assert.equal(parseMonto(80), 80);
  assert.equal(parseMonto('3.50'), 3.5);
  assert.equal(parseMonto('79.999'), 80); // redondeo a 2 decimales
  assert.equal(parseMonto(0), 0);
  assert.equal(parseMonto('abc'), null);
  assert.equal(parseMonto(''), null);
  assert.equal(parseMonto(null), null);
  assert.equal(parseMonto(undefined), null);
  assert.equal(parseMonto(-5), null);
  assert.equal(parseMonto(NaN), null);
  assert.equal(parseMonto(Infinity), null);
  assert.equal(parseMonto(2000000), null); // sobre el tope
  // La notación científica se RECHAZA a propósito: en la caja de un gimnasio
  // nadie escribe "1e3" para mil soles, pero un dedazo así se guardaría como
  // un cobro enorme sin que nadie lo note.
  assert.equal(parseMonto('1e3'), null);
});

// Valores que Number() convertía en montos válidos sin que nadie lo pidiera:
// un cobro de S/ 1 salido de un `true`, o de S/ 0 salido de un campo con
// espacios, entraban a las finanzas en silencio.
test('parseMonto: rechaza valores que no son números de verdad', () => {
  assert.equal(parseMonto(true), null);
  assert.equal(parseMonto(false), null);
  assert.equal(parseMonto(' '), null);
  assert.equal(parseMonto([]), null);
  assert.equal(parseMonto([5]), null);
  assert.equal(parseMonto({}), null);
  assert.equal(parseMonto('80 soles'), null);
  assert.equal(parseMonto('8,50'), null);   // coma decimal: se rechaza, no se adivina
  assert.equal(parseMonto('0x10'), null);
  assert.equal(parseMonto(' 80 '), 80);     // espacios alrededor sí se toleran
});

test('parseId y parseDias: tampoco aceptan booleanos ni listas', () => {
  assert.equal(parseId(true), null);
  assert.equal(parseId([3]), null);
  assert.equal(parseDias(true), null);
  assert.equal(parseDias([30]), null);
  assert.equal(parseDias(' 30 '), 30);
});

test('parseId: solo enteros positivos', () => {
  assert.equal(parseId('5'), 5);
  assert.equal(parseId(5), 5);
  assert.equal(parseId('0'), null);
  assert.equal(parseId('-3'), null);
  assert.equal(parseId('5.5'), null);
  assert.equal(parseId('abc'), null);
  assert.equal(parseId('99999999999999'), null); // fuera de rango int4
});

test('parseDias: rango razonable', () => {
  assert.equal(parseDias(30), 30);
  assert.equal(parseDias('7'), 7);
  assert.equal(parseDias(0), null);
  assert.equal(parseDias(400), null);
  assert.equal(parseDias('treinta'), null);
});

test('cleanText: recorta, limita y elimina caracteres de control', () => {
  assert.equal(cleanText('  hola  '), 'hola');
  assert.equal(cleanText('a'.repeat(500), 10), 'aaaaaaaaaa');
  assert.equal(cleanText(null), '');
  assert.equal(cleanText(undefined), '');
  assert.equal(cleanText('hola\u0007mundo'), 'holamundo');
  assert.equal(cleanText(12345), '12345');
});

test('requireText: null si queda vacío', () => {
  assert.equal(requireText('  Juan  '), 'Juan');
  assert.equal(requireText('   '), null);
  assert.equal(requireText(''), null);
  assert.equal(requireText(null), null);
});

test('parseFecha: solo YYYY-MM-DD reales', () => {
  assert.equal(parseFecha('2026-06-11'), '2026-06-11');
  assert.equal(parseFecha(' 2026-06-11 '), '2026-06-11');
  assert.equal(parseFecha('11/06/2026'), null);
  assert.equal(parseFecha('2026-06-32'), null);
  assert.equal(parseFecha(''), null);
});

test('parseMetodo: lista blanca con respaldo seguro', () => {
  assert.equal(parseMetodo('Yape'), 'Yape');
  assert.equal(parseMetodo('Efectivo'), 'Efectivo');
  assert.equal(parseMetodo(''), 'Efectivo');
  assert.equal(parseMetodo(null), 'Efectivo');
  assert.equal(parseMetodo('<script>alert(1)</script>'), 'Otro');
  assert.equal(parseMetodo('Bitcoin'), 'Otro');
});

test('parseTelefonoWhatsApp: celulares peruanos listos para el enlace', () => {
  assert.equal(parseTelefonoWhatsApp('987654321'), '51987654321');
  assert.equal(parseTelefonoWhatsApp('987 654 321'), '51987654321');
  assert.equal(parseTelefonoWhatsApp('+51 987 654 321'), '51987654321');
  assert.equal(parseTelefonoWhatsApp('51987654321'), '51987654321');
  assert.equal(parseTelefonoWhatsApp(''), null);
  assert.equal(parseTelefonoWhatsApp('123'), null);
  assert.equal(parseTelefonoWhatsApp(null), null);
});
