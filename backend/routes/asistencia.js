// ==========================================================
//  routes/asistencia.js — Control de entrada de socios
//
//  Responde dos preguntas que antes no se podían responder:
//   1. ¿Este socio está al día? (se ve al marcar la entrada, con
//      su membresía y los días que le quedan)
//   2. ¿Quién paga pero no viene? Un socio que lleva semanas sin
//      aparecer se da de baja pronto; verlo a tiempo permite
//      llamarlo antes de perderlo.
// ==========================================================

const express = require('express');
const router = express.Router();
const { query, queryOne, queryAll } = require('../db');
const { authenticateToken, requireStaff } = require('../middleware/auth');
const config = require('../config');
const { todayISO, addDays, diffDays } = require('../lib/dates');
const { parseId, parseFecha } = require('../lib/validate');
const { responderError } = require('../lib/errores');

router.use(authenticateToken, requireStaff);

// Frase que se muestra en el mostrador. Distingue los tres casos que importan:
// vencida (no debería pasar sin renovar), a punto de vencer (momento de
// cobrarle) y al día.
function situacion(socio, diasRestantes) {
  if (socio.congelada_desde) {
    return `⏸ ${socio.nombre} tiene la membresía CONGELADA desde el ${socio.congelada_desde}. Reactívala antes de que entre.`;
  }
  if (diasRestantes < 0) {
    const d = Math.abs(diasRestantes);
    return `⚠ ${socio.nombre} tiene la membresía VENCIDA hace ${d} día${d === 1 ? '' : 's'} (venció el ${socio.fecha_vence})`;
  }
  if (diasRestantes === 0) return `⚠ ${socio.nombre}: su ${socio.concepto} vence HOY`;
  if (diasRestantes <= 3) return `${socio.nombre} entra, pero su ${socio.concepto} vence en ${diasRestantes} día${diasRestantes === 1 ? '' : 's'}`;
  return `${socio.nombre} al día · le quedan ${diasRestantes} días`;
}

// Hora local del negocio en formato HH:MM (el servidor corre en UTC).
function horaAhora() {
  return new Intl.DateTimeFormat('es-PE', {
    timeZone: config.TZ, hour: '2-digit', minute: '2-digit', hour12: false,
  }).format(new Date());
}

// Quiénes entraron en una fecha (por defecto, hoy).
router.get('/', async (req, res) => {
  try {
    const fecha = parseFecha(req.query.fecha) || todayISO(config.TZ);
    const rows = await queryAll(
      `SELECT a.id, a.membership_id, a.nombre, a.fecha, a.hora, a.estado_al_entrar,
              m.concepto, m.fecha_vence
       FROM attendance a
       LEFT JOIN memberships m ON m.id = a.membership_id
       WHERE a.fecha = $1
       ORDER BY a.hora DESC, a.id DESC
       LIMIT 500`,
      [fecha]
    );
    res.json({ fecha, items: rows, total: rows.length });
  } catch (err) {
    responderError(res, err, 'ASISTENCIA', 'Error al obtener la asistencia');
  }
});

// Resumen para las tarjetas: hoy, esta semana y socios que no vienen hace mucho.
router.get('/resumen', async (req, res) => {
  try {
    const hoy = todayISO(config.TZ);
    const hace7 = addDays(hoy, -7);
    const hace30 = addDays(hoy, -30);
    const r = await queryOne(
      `SELECT
         (SELECT COUNT(*)::int FROM attendance WHERE fecha = $1) AS hoy,
         (SELECT COUNT(*)::int FROM attendance WHERE fecha >= $2 AND fecha <= $1) AS semana,
         (SELECT COUNT(DISTINCT membership_id)::int FROM attendance
            WHERE fecha >= $2 AND fecha <= $1 AND membership_id IS NOT NULL) AS socios_semana`,
      [hoy, hace7]
    );
    // Socios al día que no aparecen hace más de 30 días: los que están a punto
    // de irse sin avisar. Es la lista para llamarlos antes de perderlos.
    const ausentes = await queryAll(
      `SELECT m.id, m.nombre, m.telefono, m.concepto, m.fecha_vence,
              (SELECT MAX(a.fecha) FROM attendance a WHERE a.membership_id = m.id) AS ultima_visita
       FROM memberships m
       WHERE m.fecha_vence >= $2 AND m.congelada_desde IS NULL
         AND NOT EXISTS (
           SELECT 1 FROM attendance a
           WHERE a.membership_id = m.id AND a.fecha >= $1
         )
       ORDER BY m.nombre ASC
       LIMIT 50`,
      [hace30, hoy]
    );
    res.json({ ...r, ausentes });
  } catch (err) {
    responderError(res, err, 'ASISTENCIA', 'Error al obtener el resumen de asistencia');
  }
});

// Marcar la entrada de un socio. Devuelve su situación para que se vea en el
// mostrador si está al día, si le quedan pocos días o si ya venció.
router.post('/', async (req, res) => {
  const b = req.body || {};
  const id = parseId(b.membership_id);
  if (!id) return res.status(400).json({ error: 'Elige un socio de la lista' });

  try {
    const hoy = todayISO(config.TZ);
    const socio = await queryOne('SELECT id, nombre, concepto, fecha_vence, estado, congelada_desde FROM memberships WHERE id = $1', [id]);
    if (!socio) return res.status(404).json({ error: 'Socio no encontrado' });

    // La situación se calcula COMPARANDO LA FECHA, no leyendo la columna
    // `estado`. Esa columna se refresca cada hora o al abrir el panel, así que
    // podía estar desactualizada: en pruebas, una membresía vencida hacía 87
    // días se anunciaba como "vence HOY" y en el mostrador la dejaban pasar.
    const diasRestantes = diffDays(socio.fecha_vence, hoy);
    const estado = socio.congelada_desde ? 'congelada' : (diasRestantes < 0 ? 'vencida' : 'activa');

    // El índice único (membership_id, fecha) evita contar dos visitas el mismo
    // día; si ya marcó, se responde igual con su situación en vez de un error.
    const r = await query(
      `INSERT INTO attendance (membership_id, nombre, fecha, hora, estado_al_entrar, usuario_id)
       VALUES ($1, $2, $3, $4, $5, $6)
       ON CONFLICT (membership_id, fecha) WHERE membership_id IS NOT NULL DO NOTHING
       RETURNING id, hora`,
      [id, socio.nombre, hoy, horaAhora(), estado, req.user.id]
    );
    const yaHabiaMarcado = r.rowCount === 0;

    res.status(yaHabiaMarcado ? 200 : 201).json({
      ya_marcado: yaHabiaMarcado,
      socio: {
        id: socio.id, nombre: socio.nombre, concepto: socio.concepto,
        fecha_vence: socio.fecha_vence, estado,
      },
      dias_restantes: diasRestantes,
      mensaje: situacion(socio, diasRestantes),
    });
  } catch (err) {
    responderError(res, err, 'ASISTENCIA', 'Error al registrar la entrada');
  }
});

router.delete('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const r = await queryOne('DELETE FROM attendance WHERE id = $1 RETURNING id', [id]);
    if (!r) return res.status(404).json({ error: 'Registro no encontrado' });
    res.json({ message: 'Entrada borrada' });
  } catch (err) {
    responderError(res, err, 'ASISTENCIA', 'Error al borrar la entrada');
  }
});

module.exports = router;
