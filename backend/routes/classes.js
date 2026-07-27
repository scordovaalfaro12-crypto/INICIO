const express = require('express');
const router = express.Router();
const { query, queryAll, queryOne } = require('../db');
const { authenticateToken, requireAdmin } = require('../middleware/auth');
const config = require('../config');
const { todayISO } = require('../lib/dates');
const { parseMonto, parseId, parseDias, requireText, cleanText, parseFecha } = require('../lib/validate');
const { responderError } = require('../lib/errores');

router.get('/', authenticateToken, async (req, res) => {
  try {
    const classes = await queryAll("SELECT * FROM classes WHERE estado = 'activa' ORDER BY fecha ASC, hora ASC");
    res.json(classes);
  } catch (err) {
    responderError(res, err, 'CLASSES', 'Error al obtener clases');
  }
});

// (Aquí había un endpoint /next abierto sin contraseña para una cartelera
//  pública que nunca se llegó a usar: ninguna pantalla lo consultaba. Era el
//  único punto de la API accesible sin iniciar sesión, así que se retiró.)

router.get('/:id', authenticateToken, async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const cls = await queryOne('SELECT * FROM classes WHERE id = $1', [id]);
    if (!cls) return res.status(404).json({ error: 'No encontrada' });
    res.json(cls);
  } catch (err) {
    responderError(res, err, 'CLASSES', 'Error al obtener la clase');
  }
});

router.post('/', authenticateToken, requireAdmin, async (req, res) => {
  const b = req.body || {};
  const titulo = requireText(b.titulo, 120);
  const instructor = requireText(b.instructor, 120);
  const fecha = parseFecha(b.fecha);
  const hora = cleanText(b.hora, 10);
  const capacidad = parseDias(b.capacidad !== undefined ? b.capacidad : 20, 1, 10000);
  const precio = parseMonto(b.precio !== undefined && b.precio !== '' ? b.precio : 15);

  if (!titulo || !instructor) return res.status(400).json({ error: 'Título e instructor son obligatorios' });
  if (!fecha) return res.status(400).json({ error: 'Fecha inválida' });
  if (!/^\d{2}:\d{2}$/.test(hora)) return res.status(400).json({ error: 'Hora inválida (HH:MM)' });
  if (capacidad === null) return res.status(400).json({ error: 'Capacidad inválida' });
  if (precio === null) return res.status(400).json({ error: 'Precio inválido' });

  try {
    const r = await query(
      `INSERT INTO classes (titulo, instructor, fecha, hora, descripcion, imagen, capacidad, precio, estado)
       VALUES ($1, $2, $3, $4, $5, $6, $7, $8, 'activa') RETURNING id`,
      [titulo, instructor, fecha, hora, cleanText(b.descripcion, 500), cleanText(b.imagen, 500), capacidad, precio]
    );
    res.status(201).json({ id: r.rows[0].id, message: 'Clase creada' });
  } catch (err) {
    responderError(res, err, 'CLASSES', 'Error al crear clase');
  }
});

router.delete('/:id', authenticateToken, requireAdmin, async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    await query('DELETE FROM classes WHERE id = $1', [id]);
    res.json({ message: 'Eliminada' });
  } catch (err) {
    responderError(res, err, 'CLASSES', 'Error al eliminar la clase');
  }
});

module.exports = router;
