const { test } = require('node:test');
const assert = require('node:assert');
const {
  parseMonto, parseId, parseDias, cleanText, requireText,
  parseFecha, parseMetodo,
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
  assert.equal(parseMonto('1e3'), 1000);   // notación válida de JS
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
