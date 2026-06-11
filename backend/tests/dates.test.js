const { test } = require('node:test');
const assert = require('node:assert');
const {
  isISODate, todayISO, addDays, diffDays,
  weekStart, monthStart, yearStart, quincenaStart,
} = require('../lib/dates');

test('isISODate acepta fechas válidas', () => {
  assert.equal(isISODate('2026-06-11'), true);
  assert.equal(isISODate('2026-02-29'), false); // 2026 no es bisiesto
  assert.equal(isISODate('2024-02-29'), true);  // 2024 sí
  assert.equal(isISODate('2026-13-01'), false);
  assert.equal(isISODate('2026-00-10'), false);
  assert.equal(isISODate('2026-06-31'), false);
  assert.equal(isISODate('11/06/2026'), false);
  assert.equal(isISODate('2026-6-1'), false);
  assert.equal(isISODate(''), false);
  assert.equal(isISODate(null), false);
  assert.equal(isISODate(20260611), false);
  assert.equal(isISODate('1999-01-01'), false); // fuera de rango razonable
});

test('todayISO devuelve YYYY-MM-DD en la zona pedida', () => {
  const lima = todayISO('America/Lima');
  assert.match(lima, /^\d{4}-\d{2}-\d{2}$/);
  // Lima (UTC-5) y Auckland (UTC+12/13) difieren a veces, pero ambas deben ser válidas
  assert.equal(isISODate(todayISO('Pacific/Auckland')), true);
});

test('addDays cruza meses y años correctamente', () => {
  assert.equal(addDays('2026-06-11', 30), '2026-07-11');
  assert.equal(addDays('2026-12-25', 10), '2027-01-04');
  assert.equal(addDays('2024-02-28', 1), '2024-02-29'); // bisiesto
  assert.equal(addDays('2026-02-28', 1), '2026-03-01'); // no bisiesto
  assert.equal(addDays('2026-01-31', 1), '2026-02-01');
  assert.equal(addDays('2026-06-11', 0), '2026-06-11');
  assert.equal(addDays('2026-06-11', 365), '2027-06-11');
});

test('diffDays calcula días enteros', () => {
  assert.equal(diffDays('2026-06-15', '2026-06-11'), 4);
  assert.equal(diffDays('2026-06-11', '2026-06-11'), 0);
  assert.equal(diffDays('2026-06-01', '2026-06-11'), -10);
  assert.equal(diffDays('2027-06-11', '2026-06-11'), 365);
});

test('weekStart devuelve el lunes de la semana', () => {
  assert.equal(weekStart('2026-06-11'), '2026-06-08'); // jueves → lunes
  assert.equal(weekStart('2026-06-08'), '2026-06-08'); // lunes → mismo lunes
  assert.equal(weekStart('2026-06-14'), '2026-06-08'); // domingo → lunes anterior
  assert.equal(weekStart('2026-06-01'), '2026-06-01'); // lunes 1 → él mismo
});

test('inicios de periodo', () => {
  assert.equal(monthStart('2026-06-11'), '2026-06-01');
  assert.equal(yearStart('2026-06-11'), '2026-01-01');
  assert.equal(quincenaStart('2026-06-11'), '2026-06-01');
  assert.equal(quincenaStart('2026-06-15'), '2026-06-01');
  assert.equal(quincenaStart('2026-06-16'), '2026-06-16');
  assert.equal(quincenaStart('2026-06-30'), '2026-06-16');
});
