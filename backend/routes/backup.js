// ==========================================================
//  routes/backup.js — Respaldo completo en un clic
//  Descarga un JSON con todas las tablas del negocio. Guardar
//  ese archivo (mensualmente, por ejemplo) garantiza que aunque
//  pase lo peor con la nube, los datos del gym existen en local.
// ==========================================================

const express = require('express');
const router = express.Router();
const { queryAll } = require('../db');
const { authenticateToken, requireAdmin } = require('../middleware/auth');
const config = require('../config');
const { todayISO } = require('../lib/dates');

router.get('/', authenticateToken, requireAdmin, async (req, res) => {
  try {
    const [memberships, payments, extraSales, classes, users] = await Promise.all([
      queryAll('SELECT * FROM memberships ORDER BY id'),
      queryAll('SELECT * FROM payments ORDER BY id'),
      queryAll('SELECT * FROM extra_sales ORDER BY id'),
      queryAll('SELECT * FROM classes ORDER BY id'),
      // Nunca exportar contraseñas, ni siquiera hasheadas.
      queryAll('SELECT id, firstname, lastname, email, role FROM users ORDER BY id'),
    ]);

    const respaldo = {
      sistema: 'ZONA VIP GYM',
      version: 2,
      generado: new Date().toISOString(),
      fecha_negocio: todayISO(config.TZ),
      tablas: {
        memberships,
        payments,
        extra_sales: extraSales,
        classes,
        users,
      },
    };

    res.setHeader('Content-Disposition', `attachment; filename="respaldo-zonavip-${todayISO(config.TZ)}.json"`);
    res.json(respaldo);
  } catch (err) {
    console.error('[BACKUP] Error al generar respaldo:', err);
    res.status(500).json({ error: 'Error al generar respaldo' });
  }
});

module.exports = router;
