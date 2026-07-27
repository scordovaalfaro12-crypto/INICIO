// ==========================================================
//  routes/extras.js — Ventas de productos (aguas, proteínas, ropa…)
//
//  "Borrar" una venta ya no la elimina de la base: la marca como
//  anulada. Deja de contar en las finanzas (que es lo que la dueña
//  quiere al corregir un error) pero el registro sobrevive, así que
//  el historial del negocio nunca queda con huecos inexplicables.
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
    const rows = await queryAll(
      `SELECT * FROM extra_sales WHERE anulado = FALSE ORDER BY fecha DESC, id DESC LIMIT ${limite}`
    );
    res.json(rows);
  } catch (err) {
    responderError(res, err, 'EXTRAS', 'Error al obtener ventas');
  }
});

// Totales del mes calculados en la base: el navegador ya no necesita
// descargarse todas las ventas para sumar tres tarjetas.
router.get('/resumen', async (req, res) => {
  try {
    const desde = parseFecha(req.query.desde) || '2000-01-01';
    const [totales, porCategoria] = await Promise.all([
      queryOne(
        `SELECT COALESCE(SUM(monto),0)::numeric(14,2) AS total, COUNT(*)::int AS cantidad
         FROM extra_sales WHERE fecha >= $1 AND anulado = FALSE`,
        [desde]
      ),
      queryAll(
        `SELECT categoria, COALESCE(SUM(monto),0)::numeric(14,2) AS total, COUNT(*)::int AS cantidad
         FROM extra_sales WHERE fecha >= $1 AND anulado = FALSE
         GROUP BY categoria ORDER BY total DESC LIMIT 10`,
        [desde]
      ),
    ]);
    res.json({
      total: Number(totales.total),
      cantidad: totales.cantidad,
      categorias: porCategoria.map((c) => ({ categoria: c.categoria, total: Number(c.total), cantidad: c.cantidad })),
    });
  } catch (err) {
    responderError(res, err, 'EXTRAS', 'Error al obtener el resumen de ventas');
  }
});

router.post('/', async (req, res) => {
  const b = req.body || {};
  const categoria = requireText(b.categoria, 60);
  const monto = parseMonto(b.monto);
  const fecha = parseFecha(b.fecha);

  if (!categoria) return res.status(400).json({ error: 'La categoría es obligatoria' });
  if (monto === null) return res.status(400).json({ error: 'Monto inválido' });
  if (!fecha) return res.status(400).json({ error: 'Fecha inválida (usa el selector de fecha)' });

  try {
    const r = await query(
      `INSERT INTO extra_sales (categoria, descripcion, monto, metodo, fecha, notas)
       VALUES ($1, $2, $3, $4, $5, $6) RETURNING id`,
      [categoria, cleanText(b.descripcion, 200), monto, parseMetodo(b.metodo), fecha, cleanText(b.notas, 500)]
    );
    res.status(201).json({ id: r.rows[0].id });
  } catch (err) {
    responderError(res, err, 'EXTRAS', 'Error al guardar');
  }
});

router.delete('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const r = await queryOne(
      `UPDATE extra_sales SET anulado = TRUE, motivo_anulacion = 'Anulada por el administrador'
       WHERE id = $1 AND anulado = FALSE RETURNING id`,
      [id]
    );
    if (!r) return res.status(404).json({ error: 'Venta no encontrada o ya anulada' });
    res.json({ message: 'Venta anulada: ya no cuenta en las finanzas' });
  } catch (err) {
    responderError(res, err, 'EXTRAS', 'Error al anular la venta');
  }
});

module.exports = router;
