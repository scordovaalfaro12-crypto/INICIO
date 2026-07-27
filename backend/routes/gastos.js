// ==========================================================
//  routes/gastos.js — Egresos del negocio
//
//  El sistema solo llevaba la cuenta del dinero que ENTRA. Con
//  eso es imposible saber si el gimnasio gana o pierde: se veía
//  "S/ 8.000 este mes" sin descontar alquiler, luz ni sueldos.
//  Aquí se registran los gastos y Finanzas calcula la utilidad
//  real (ingresos − gastos).
//
//  Igual que las ventas, un gasto no se borra: se ANULA. Así el
//  historial del negocio nunca queda con huecos inexplicables.
// ==========================================================

const express = require('express');
const router = express.Router();
const { query, queryOne, queryAll } = require('../db');
const { authenticateToken, requireAdmin } = require('../middleware/auth');
const { parseMonto, parseId, requireText, cleanText, parseFecha, parseMetodo } = require('../lib/validate');
const { responderError } = require('../lib/errores');

router.use(authenticateToken, requireAdmin);

router.get('/', async (req, res) => {
  try {
    let limite = parseInt(req.query.limit, 10);
    if (!Number.isInteger(limite) || limite < 1) limite = 300;
    limite = Math.min(limite, 5000);
    const desde = parseFecha(req.query.desde) || '2000-01-01';
    const hasta = parseFecha(req.query.hasta) || '2200-12-31';
    const rows = await queryAll(
      `SELECT * FROM expenses
       WHERE anulado = FALSE AND fecha >= $1 AND fecha <= $2
       ORDER BY fecha DESC, id DESC LIMIT ${limite}`,
      [desde, hasta]
    );
    res.json(rows);
  } catch (err) {
    responderError(res, err, 'GASTOS', 'Error al obtener los gastos');
  }
});

// Totales del periodo y reparto por rubro, calculados en la base de datos.
router.get('/resumen', async (req, res) => {
  try {
    const desde = parseFecha(req.query.desde) || '2000-01-01';
    const hasta = parseFecha(req.query.hasta) || '2200-12-31';
    const [totales, porCategoria] = await Promise.all([
      queryOne(
        `SELECT COALESCE(SUM(monto),0)::numeric(14,2) AS total, COUNT(*)::int AS cantidad
         FROM expenses WHERE fecha >= $1 AND fecha <= $2 AND anulado = FALSE`,
        [desde, hasta]
      ),
      queryAll(
        `SELECT categoria, COALESCE(SUM(monto),0)::numeric(14,2) AS total, COUNT(*)::int AS cantidad
         FROM expenses WHERE fecha >= $1 AND fecha <= $2 AND anulado = FALSE
         GROUP BY categoria ORDER BY total DESC LIMIT 15`,
        [desde, hasta]
      ),
    ]);
    res.json({
      total: Number(totales.total),
      cantidad: totales.cantidad,
      categorias: porCategoria.map((c) => ({
        categoria: c.categoria, total: Number(c.total), cantidad: c.cantidad,
      })),
    });
  } catch (err) {
    responderError(res, err, 'GASTOS', 'Error al obtener el resumen de gastos');
  }
});

router.post('/', async (req, res) => {
  const b = req.body || {};
  const categoria = requireText(b.categoria, 60);
  const monto = parseMonto(b.monto);
  const fecha = parseFecha(b.fecha);

  if (!categoria) return res.status(400).json({ error: 'La categoría del gasto es obligatoria' });
  if (monto === null) return res.status(400).json({ error: 'Monto inválido' });
  if (!fecha) return res.status(400).json({ error: 'Fecha inválida (usa el selector de fecha)' });

  try {
    const r = await query(
      `INSERT INTO expenses (categoria, descripcion, monto, metodo, fecha, notas, usuario_id)
       VALUES ($1, $2, $3, $4, $5, $6, $7) RETURNING id`,
      [categoria, cleanText(b.descripcion, 200), monto, parseMetodo(b.metodo), fecha,
       cleanText(b.notas, 500), req.user.id]
    );
    res.status(201).json({ id: r.rows[0].id });
  } catch (err) {
    responderError(res, err, 'GASTOS', 'Error al guardar el gasto');
  }
});

router.delete('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const r = await queryOne(
      `UPDATE expenses SET anulado = TRUE, motivo_anulacion = 'Anulado por el administrador'
       WHERE id = $1 AND anulado = FALSE RETURNING id`,
      [id]
    );
    if (!r) return res.status(404).json({ error: 'Gasto no encontrado o ya anulado' });
    res.json({ message: 'Gasto anulado: ya no se descuenta de las finanzas' });
  } catch (err) {
    responderError(res, err, 'GASTOS', 'Error al anular el gasto');
  }
});

module.exports = router;
