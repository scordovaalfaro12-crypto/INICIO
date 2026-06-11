// ==========================================================
//  routes/memberships.js — Matrículas, renovaciones y finanzas
//
//  Cambio de fondo respecto a la versión anterior: cada cobro
//  se inserta en la tabla `payments` con su fecha real. Antes,
//  una renovación SUMABA el monto sobre la matrícula original,
//  así que el dinero quedaba contado en el mes equivocado y,
//  peor, el auto-borrado de vencidas lo desaparecía del todo.
// ==========================================================

const express = require('express');
const router = express.Router();
const { queryOne, queryAll, withTransaction, actualizarEstadoMemberships } = require('../db');
const { authenticateToken, requireAdmin } = require('../middleware/auth');
const config = require('../config');
const { todayISO, addDays, weekStart, monthStart, yearStart, quincenaStart } = require('../lib/dates');
const { parseMonto, parseId, parseDias, requireText, cleanText, parseFecha, parseMetodo } = require('../lib/validate');

// Todas las rutas de matrículas son solo para administradores.
router.use(authenticateToken, requireAdmin);

async function refreshStates() {
  try {
    await actualizarEstadoMemberships();
  } catch (e) {
    // No bloquea la lectura: el estado se recalcula en el próximo intento.
    console.error('[MEMBERSHIPS] No se pudo refrescar estados:', e.message);
  }
}

router.get('/', async (req, res) => {
  try {
    await refreshStates();
    const rows = await queryAll('SELECT * FROM memberships ORDER BY fecha_vence DESC, id DESC');
    res.json(rows);
  } catch (err) {
    console.error('[MEMBERSHIPS] Error al listar:', err);
    res.status(500).json({ error: 'Error al obtener matrículas' });
  }
});

// Libro de pagos individual (para el CSV y auditoría).
router.get('/pagos', async (req, res) => {
  try {
    const desde = parseFecha(req.query.desde) || '2000-01-01';
    const hasta = parseFecha(req.query.hasta) || '2200-12-31';
    const rows = await queryAll(
      `SELECT id, tipo, membership_id, nombre, concepto, monto, metodo, fecha_pago, creado
       FROM payments
       WHERE fecha_pago >= $1 AND fecha_pago <= $2
       ORDER BY fecha_pago DESC, id DESC
       LIMIT 50000`,
      [desde, hasta]
    );
    res.json(rows);
  } catch (err) {
    console.error('[MEMBERSHIPS] Error al listar pagos:', err);
    res.status(500).json({ error: 'Error al obtener pagos' });
  }
});

// Reporte financiero: lee del libro de pagos (payments) + ventas extras.
router.get('/finanzas', async (req, res) => {
  try {
    await refreshStates();
    const hoy = todayISO(config.TZ);
    const rangos = {
      hoy,
      semana: weekStart(hoy),
      quincena: quincenaStart(hoy),
      mes: monthStart(hoy),
      anio: yearStart(hoy),
    };

    const sumPagos = (desde) => queryOne(
      `SELECT COALESCE(SUM(monto), 0)::numeric(14,2) AS total, COUNT(*)::int AS cantidad
       FROM payments WHERE fecha_pago >= $1`,
      [desde]
    );
    const sumExtras = (desde) => queryOne(
      `SELECT COALESCE(SUM(monto), 0)::numeric(14,2) AS total, COUNT(*)::int AS cantidad
       FROM extra_sales WHERE fecha >= $1`,
      [desde]
    );

    const periodos = Object.keys(rangos);
    const resultados = await Promise.all(
      periodos.map((p) => Promise.all([sumPagos(rangos[p]), sumExtras(rangos[p])]))
    );

    const finanzas = {};
    periodos.forEach((p, i) => {
      const [mat, ext] = resultados[i];
      finanzas[p] = {
        matriculas: { total: Number(mat.total), cantidad: mat.cantidad },
        extras: { total: Number(ext.total), cantidad: ext.cantidad },
        online: { total: 0, cantidad: 0 }, // compatibilidad con el frontend
        total: Math.round((Number(mat.total) + Number(ext.total)) * 100) / 100,
        cantidad: mat.cantidad + ext.cantidad,
      };
    });

    // Métodos de pago del mes (pagos de matrícula + ventas extras).
    const metodos = await queryAll(
      `SELECT metodo, SUM(total)::numeric(14,2) AS total, SUM(cantidad)::int AS cantidad FROM (
         SELECT COALESCE(metodo, 'Efectivo') AS metodo, COALESCE(SUM(monto),0) AS total, COUNT(*) AS cantidad
         FROM payments WHERE fecha_pago >= $1 GROUP BY 1
         UNION ALL
         SELECT COALESCE(metodo, 'Efectivo') AS metodo, COALESCE(SUM(monto),0) AS total, COUNT(*) AS cantidad
         FROM extra_sales WHERE fecha >= $1 GROUP BY 1
       ) t GROUP BY metodo ORDER BY total DESC`,
      [rangos.mes]
    );
    finanzas.metodos = metodos.map((m) => ({ metodo: m.metodo, total: Number(m.total), cantidad: m.cantidad }));

    const socios = await queryAll(`SELECT estado, COUNT(*)::int AS cantidad FROM memberships GROUP BY estado`);
    const sociosAgg = { activos: 0, vencidos: 0 };
    socios.forEach((s) => {
      if (s.estado === 'activa') sociosAgg.activos = s.cantidad;
      if (s.estado === 'vencida') sociosAgg.vencidos = s.cantidad;
    });
    finanzas.socios = sociosAgg;

    res.json(finanzas);
  } catch (err) {
    console.error('[MEMBERSHIPS] Error en finanzas:', err);
    res.status(500).json({ error: 'Error en finanzas' });
  }
});

router.post('/', async (req, res) => {
  const b = req.body || {};
  const nombre = requireText(b.nombre, 120);
  const concepto = requireText(b.concepto, 80);
  const monto = parseMonto(b.monto);
  const fechaPago = parseFecha(b.fecha_pago);
  const fechaVence = parseFecha(b.fecha_vence);

  if (!nombre || !concepto) return res.status(400).json({ error: 'Nombre y concepto son obligatorios' });
  if (monto === null) return res.status(400).json({ error: 'Monto inválido' });
  if (!fechaPago || !fechaVence) return res.status(400).json({ error: 'Fechas inválidas (usa el selector de fecha)' });
  if (fechaVence < fechaPago) return res.status(400).json({ error: 'La fecha de vencimiento no puede ser anterior al pago' });

  const telefono = cleanText(b.telefono, 30);
  const dni = cleanText(b.dni, 20);
  const metodo = parseMetodo(b.metodo);
  const notas = cleanText(b.notas, 500);
  // El estado se deriva de la fecha: coherente incluso si registran con fecha pasada.
  const estado = fechaVence < todayISO(config.TZ) ? 'vencida' : 'activa';

  try {
    const id = await withTransaction(async (tx) => {
      const r = await tx.query(
        `INSERT INTO memberships (nombre, telefono, dni, concepto, monto, metodo, fecha_pago, fecha_vence, notas, estado)
         VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10) RETURNING id`,
        [nombre, telefono, dni, concepto, monto, metodo, fechaPago, fechaVence, notas, estado]
      );
      const newId = r.rows[0].id;
      await tx.query(
        `INSERT INTO payments (tipo, membership_id, nombre, concepto, monto, metodo, fecha_pago)
         VALUES ('matricula', $1, $2, $3, $4, $5, $6)`,
        [newId, nombre, concepto, monto, metodo, fechaPago]
      );
      return newId;
    });
    res.status(201).json({ id });
  } catch (err) {
    console.error('[MEMBERSHIPS] Error al crear:', err);
    res.status(500).json({ error: 'Error al crear matrícula' });
  }
});

router.put('/:id/renew', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });

  const b = req.body || {};
  const dias = parseDias(b.dias !== undefined ? b.dias : 30);
  const monto = parseMonto(b.monto !== undefined && b.monto !== '' ? b.monto : 0);
  if (dias === null) return res.status(400).json({ error: 'Días inválidos (1 a 366)' });
  if (monto === null) return res.status(400).json({ error: 'Monto inválido' });

  try {
    const hoy = todayISO(config.TZ);
    const resultado = await withTransaction(async (tx) => {
      // FOR UPDATE: dos clics simultáneos en "Renovar" no se pisan entre sí.
      const r = await tx.query('SELECT * FROM memberships WHERE id = $1 FOR UPDATE', [id]);
      const m = r.rows[0];
      if (!m) return null;

      const base = (m.fecha_vence && m.fecha_vence > hoy) ? m.fecha_vence : hoy;
      const nuevoVence = addDays(base, dias);
      const metodo = b.metodo ? parseMetodo(b.metodo) : (m.metodo || 'Efectivo');

      await tx.query(
        `UPDATE memberships SET fecha_vence = $1, estado = 'activa', monto = $2, metodo = $3 WHERE id = $4`,
        [nuevoVence, monto, metodo, id]
      );
      await tx.query(
        `INSERT INTO payments (tipo, membership_id, nombre, concepto, monto, metodo, fecha_pago)
         VALUES ('renovacion', $1, $2, $3, $4, $5, $6)`,
        [id, m.nombre, m.concepto, monto, metodo, hoy]
      );
      return nuevoVence;
    });

    if (!resultado) return res.status(404).json({ error: 'No encontrada' });
    res.json({ message: 'Renovada', nuevoVence: resultado });
  } catch (err) {
    console.error('[MEMBERSHIPS] Error al renovar:', err);
    res.status(500).json({ error: 'Error al renovar' });
  }
});

// Borra la ficha del socio. Sus pagos ya cobrados QUEDAN en el libro de
// ingresos (payments), así que los reportes históricos no cambian.
router.delete('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    await queryOne('DELETE FROM memberships WHERE id = $1 RETURNING id', [id]);
    res.json({ message: 'Eliminada' });
  } catch (err) {
    console.error('[MEMBERSHIPS] Error al eliminar:', err);
    res.status(500).json({ error: 'Error al eliminar' });
  }
});

module.exports = router;
