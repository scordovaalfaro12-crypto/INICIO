// ==========================================================
//  routes/catalog.js — Precios, promociones y categorías
//  El administrador crea sus propias opciones (nombre + precio
//  + duración para matrículas; nombre + precio sugerido para
//  productos extras) y las quita cuando termina la promo.
//  Quitar una opción NUNCA toca el historial: las matrículas y
//  pagos ya registrados guardan el concepto como texto propio.
// ==========================================================

const express = require('express');
const router = express.Router();
const { query, queryAll } = require('../db');
const { authenticateToken, requireAdmin } = require('../middleware/auth');
const { parseMonto, parseId, parseDias, requireText } = require('../lib/validate');
const { responderError } = require('../lib/errores');

router.use(authenticateToken, requireAdmin);

router.get('/', async (req, res) => {
  try {
    const rows = await queryAll(
      `SELECT id, tipo, nombre, precio, dias FROM catalog_options
       ORDER BY tipo, dias ASC NULLS LAST, precio ASC NULLS LAST, id ASC`
    );
    res.json(rows);
  } catch (err) {
    responderError(res, err, 'CATALOGO', 'Error al obtener el catálogo');
  }
});

router.post('/', async (req, res) => {
  const b = req.body || {};
  // 'gasto' son los rubros de egreso (alquiler, luz, sueldos…). Sin él, la
  // pantalla de precios prometía poder crearlos pero el servidor los rechazaba.
  const TIPOS = ['matricula', 'extra', 'gasto'];
  const tipo = TIPOS.includes(b.tipo) ? b.tipo : null;
  const nombre = requireText(b.nombre, 60);

  if (!tipo) return res.status(400).json({ error: 'Tipo inválido' });
  if (!nombre) return res.status(400).json({ error: 'El nombre es obligatorio' });

  let precio = null;
  let dias = null;

  if (tipo === 'matricula') {
    precio = parseMonto(b.precio);
    dias = parseDias(b.dias);
    if (precio === null) return res.status(400).json({ error: 'Precio inválido' });
    if (dias === null) return res.status(400).json({ error: 'Duración inválida (1 a 366 días)' });
  } else {
    // Para categorías de extras el precio sugerido es opcional.
    if (b.precio !== undefined && b.precio !== null && b.precio !== '') {
      precio = parseMonto(b.precio);
      if (precio === null) return res.status(400).json({ error: 'Precio sugerido inválido' });
    }
  }

  try {
    const r = await query(
      `INSERT INTO catalog_options (tipo, nombre, precio, dias) VALUES ($1, $2, $3, $4) RETURNING id`,
      [tipo, nombre, precio, dias]
    );
    res.status(201).json({ id: r.rows[0].id });
  } catch (err) {
    if (err.code === '23505') {
      return res.status(400).json({ error: 'Ya existe una opción con ese nombre' });
    }
    return responderError(res, err, 'CATALOGO', 'Error al guardar la opción');
  }
});

router.delete('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    await query('DELETE FROM catalog_options WHERE id = $1', [id]);
    res.json({ message: 'Opción eliminada (el historial de pagos no cambia)' });
  } catch (err) {
    responderError(res, err, 'CATALOGO', 'Error al eliminar');
  }
});

module.exports = router;
