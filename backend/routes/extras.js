const express = require('express');
const router = express.Router();
const { query, queryAll } = require('../db');
const { authenticateToken, requireAdmin } = require('../middleware/auth');
const { parseMonto, parseId, requireText, cleanText, parseFecha, parseMetodo } = require('../lib/validate');

router.use(authenticateToken, requireAdmin);

router.get('/', async (req, res) => {
  try {
    const rows = await queryAll('SELECT * FROM extra_sales ORDER BY fecha DESC, id DESC LIMIT 5000');
    res.json(rows);
  } catch (err) {
    console.error('[EXTRAS] Error al listar:', err);
    res.status(500).json({ error: 'Error al obtener ventas' });
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
    console.error('[EXTRAS] Error al guardar:', err);
    res.status(500).json({ error: 'Error al guardar' });
  }
});

router.delete('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    await query('DELETE FROM extra_sales WHERE id = $1', [id]);
    res.json({ message: 'Eliminado' });
  } catch (err) {
    console.error('[EXTRAS] Error al eliminar:', err);
    res.status(500).json({ error: 'Error al eliminar' });
  }
});

module.exports = router;
