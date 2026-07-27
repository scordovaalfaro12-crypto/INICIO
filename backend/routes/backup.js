// ==========================================================
//  routes/backup.js — Respaldo completo en un clic + restauración
//
//  Descarga un JSON con todas las tablas del negocio. Guardar ese
//  archivo (una vez al mes, por ejemplo) garantiza que aunque pase
//  lo peor con la nube, los datos del gym existen en local.
//
//  Y ahora se puede VOLVER A CARGAR. Un respaldo que no se puede
//  restaurar solo da una sensación falsa de seguridad: el día que
//  hiciera falta, el archivo estaría ahí sin forma de usarlo.
// ==========================================================

const express = require('express');
const router = express.Router();
const { queryAll, queryOne, withTransaction } = require('../db');
const { authenticateToken, requireAdmin } = require('../middleware/auth');
const config = require('../config');
const { todayISO } = require('../lib/dates');
const { responderError } = require('../lib/errores');

router.use(authenticateToken, requireAdmin);

router.get('/', async (req, res) => {
  try {
    const [memberships, payments, extraSales, classes, users, catalogo, gastos, asistencia] = await Promise.all([
      queryAll('SELECT * FROM memberships ORDER BY id'),
      queryAll('SELECT * FROM payments ORDER BY id'),
      queryAll('SELECT * FROM extra_sales ORDER BY id'),
      queryAll('SELECT * FROM classes ORDER BY id'),
      // Nunca exportar contraseñas, ni siquiera hasheadas.
      queryAll('SELECT id, firstname, lastname, email, role FROM users ORDER BY id'),
      queryAll('SELECT * FROM catalog_options ORDER BY id'),
      queryAll('SELECT * FROM expenses ORDER BY id'),
      queryAll('SELECT * FROM attendance ORDER BY id'),
    ]);

    const respaldo = {
      sistema: 'ZONA VIP GYM',
      version: 4,
      generado: new Date().toISOString(),
      fecha_negocio: todayISO(config.TZ),
      tablas: {
        memberships,
        payments,
        extra_sales: extraSales,
        classes,
        users,
        catalog_options: catalogo,
        expenses: gastos,
        attendance: asistencia,
      },
    };

    res.setHeader('Content-Disposition', `attachment; filename="respaldo-zonavip-${todayISO(config.TZ)}.json"`);
    res.json(respaldo);
  } catch (err) {
    responderError(res, err, 'BACKUP', 'Error al generar respaldo');
  }
});

// Cuánto hay ahora mismo: se muestra antes de restaurar para que quede claro
// qué se va a reemplazar.
router.get('/estado', async (req, res) => {
  try {
    const r = await queryOne(
      `SELECT (SELECT COUNT(*) FROM memberships)::int AS socios,
              (SELECT COUNT(*) FROM payments)::int AS pagos,
              (SELECT COUNT(*) FROM extra_sales)::int AS ventas,
              (SELECT COUNT(*) FROM catalog_options)::int AS opciones,
              (SELECT COUNT(*) FROM expenses)::int AS gastos,
              (SELECT COUNT(*) FROM attendance)::int AS visitas`
    );
    res.json(r);
  } catch (err) {
    responderError(res, err, 'BACKUP', 'Error al consultar el estado');
  }
});

// Tablas que se pueden restaurar, con sus columnas permitidas. La lista es
// fija a propósito: el archivo subido NUNCA decide qué tablas o columnas se
// tocan, así un JSON manipulado no puede escribir donde no debe.
const RESTAURABLES = [
  { tabla: 'memberships', columnas: ['id', 'nombre', 'telefono', 'dni', 'concepto', 'monto', 'metodo', 'fecha_pago', 'fecha_vence', 'notas', 'estado'] },
  { tabla: 'payments', columnas: ['id', 'tipo', 'membership_id', 'nombre', 'concepto', 'monto', 'metodo', 'fecha_pago', 'notas', 'anulado', 'motivo_anulacion'] },
  { tabla: 'extra_sales', columnas: ['id', 'categoria', 'descripcion', 'monto', 'metodo', 'fecha', 'notas', 'anulado', 'motivo_anulacion'] },
  { tabla: 'classes', columnas: ['id', 'titulo', 'instructor', 'fecha', 'hora', 'descripcion', 'imagen', 'capacidad', 'precio', 'estado'] },
  { tabla: 'catalog_options', columnas: ['id', 'tipo', 'nombre', 'precio', 'dias'] },
  { tabla: 'expenses', columnas: ['id', 'categoria', 'descripcion', 'monto', 'metodo', 'fecha', 'notas', 'anulado', 'motivo_anulacion'] },
  { tabla: 'attendance', columnas: ['id', 'membership_id', 'nombre', 'fecha', 'hora', 'estado_al_entrar'] },
];

const MAX_FILAS = 200000; // techo de seguridad para no agotar la memoria

// Restaura un respaldo. Reemplaza los datos del negocio por los del archivo.
// Los USUARIOS y contraseñas NO se tocan nunca: si el respaldo fuera de otra
// instalación, la dueña seguiría pudiendo entrar con su clave actual.
router.post('/restaurar', async (req, res) => {
  const cuerpo = req.body || {};
  const tablas = cuerpo.tablas;

  if (!tablas || typeof tablas !== 'object' || Array.isArray(tablas)) {
    return res.status(400).json({ error: 'El archivo no parece un respaldo de ZONA VIP GYM (falta "tablas").' });
  }
  if (cuerpo.sistema && cuerpo.sistema !== 'ZONA VIP GYM') {
    return res.status(400).json({ error: 'El archivo es de otro sistema, no de ZONA VIP GYM.' });
  }

  const total = RESTAURABLES.reduce((s, r) => s + (Array.isArray(tablas[r.tabla]) ? tablas[r.tabla].length : 0), 0);
  if (total === 0) {
    return res.status(400).json({ error: 'El respaldo está vacío: no hay nada que restaurar.' });
  }
  if (total > MAX_FILAS) {
    return res.status(413).json({ error: 'El respaldo es demasiado grande para restaurarlo por la web.' });
  }

  try {
    const resumen = await withTransaction(async (tx) => {
      const hecho = {};
      for (const { tabla, columnas } of RESTAURABLES) {
        const filas = Array.isArray(tablas[tabla]) ? tablas[tabla] : null;
        if (!filas) continue; // tabla ausente en el archivo: se deja como está

        // TRUNCATE + inserción dentro de UNA transacción: si algo falla a la
        // mitad, la base queda exactamente como estaba antes de empezar.
        await tx.query(`TRUNCATE TABLE ${tabla} RESTART IDENTITY CASCADE`);

        for (const fila of filas) {
          if (!fila || typeof fila !== 'object') continue;
          const cols = columnas.filter((c) => fila[c] !== undefined);
          if (cols.length === 0) continue;
          const marcadores = cols.map((_, i) => `$${i + 1}`).join(', ');
          await tx.query(
            `INSERT INTO ${tabla} (${cols.map((c) => `"${c}"`).join(', ')}) VALUES (${marcadores})
             ON CONFLICT DO NOTHING`,
            cols.map((c) => fila[c])
          );
        }

        // Deja los contadores de ID por encima del máximo restaurado, si no
        // el siguiente socio nuevo chocaría con un ID ya usado.
        await tx.query(
          `SELECT setval(pg_get_serial_sequence('${tabla}', 'id'),
                         GREATEST((SELECT COALESCE(MAX(id), 0) FROM ${tabla}), 1))`
        );
        hecho[tabla] = filas.length;
      }
      return hecho;
    });

    console.warn('[BACKUP] Restauración completada:', JSON.stringify(resumen));
    res.json({ message: 'Respaldo restaurado correctamente', restaurado: resumen });
  } catch (err) {
    responderError(res, err, 'BACKUP', 'No se pudo restaurar el respaldo. Los datos actuales NO se modificaron.');
  }
});

module.exports = router;
