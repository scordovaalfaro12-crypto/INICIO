// ==========================================================
//  routes/memberships.js — Matrículas, renovaciones y finanzas
//
//  Cambio de fondo respecto a la versión anterior: cada cobro
//  se inserta en la tabla `payments` con su fecha real. Antes,
//  una renovación SUMABA el monto sobre la matrícula original,
//  así que el dinero quedaba contado en el mes equivocado y,
//  peor, el auto-borrado de vencidas lo desaparecía del todo.
//
//  Segundo cambio de fondo: el listado se filtra y se cuenta EN
//  LA BASE DE DATOS. Antes el navegador se descargaba las fichas
//  de TODOS los socios en cada búsqueda (medido: 379 KB y 13
//  segundos con 1.500 socios, una descarga por cada tecla).
// ==========================================================

const express = require('express');
const router = express.Router();
const { query, queryOne, queryAll, withTransaction, actualizarEstadoMemberships } = require('../db');
const { authenticateToken, requireAdmin } = require('../middleware/auth');
const config = require('../config');
const { todayISO, addDays, weekStart, monthStart, yearStart, quincenaStart } = require('../lib/dates');
const { parseMonto, parseId, parseDias, requireText, cleanText, parseFecha, parseMetodo } = require('../lib/validate');
const { responderError } = require('../lib/errores');

// Todas las rutas de matrículas son solo para administradores.
router.use(authenticateToken, requireAdmin);

const LIMITE_POR_DEFECTO = 300;
const LIMITE_MAXIMO = 5000;

// Quita tildes y espacios del texto buscado para que coincida con el mismo
// tratamiento que se aplica al nombre guardado.
function sinTildes(s) {
  return s.normalize('NFD').replace(/[\u0300-\u036f]/g, '').replace(/\s+/g, ' ').trim();
}

async function refreshStates() {
  try {
    await actualizarEstadoMemberships();
  } catch (e) {
    // No bloquea la lectura: el estado se recalcula en el próximo intento.
    console.error('[MEMBERSHIPS] No se pudo refrescar estados:', e.message);
  }
}

// Contadores para las tarjetas de arriba. Se calculan con COUNT en la base:
// el navegador ya no necesita la lista completa para saber cuántos hay.
router.get('/resumen', async (req, res) => {
  try {
    await refreshStates();
    const hoy = todayISO(config.TZ);
    const en7 = addDays(hoy, 7);
    const r = await queryOne(
      `SELECT
         COUNT(*) FILTER (WHERE estado = 'activa')::int AS activos,
         COUNT(*) FILTER (WHERE estado = 'vencida')::int AS vencidos,
         COUNT(*) FILTER (WHERE estado = 'activa' AND fecha_vence <= $1)::int AS pronto,
         COUNT(*)::int AS total
       FROM memberships`,
      [en7]
    );
    res.json(r || { activos: 0, vencidos: 0, pronto: 0, total: 0 });
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al obtener el resumen de socios');
  }
});

// Listado con búsqueda, filtro y tope, todo resuelto en la base de datos.
// Devuelve { items, total, limite } — `total` es cuántos coinciden en total,
// para poder avisar "mostrando 300 de 1.500".
router.get('/', async (req, res) => {
  try {
    await refreshStates();
    const hoy = todayISO(config.TZ);
    const en7 = addDays(hoy, 7);

    const q = cleanText(req.query.q, 80).toLowerCase();
    const estado = ['activa', 'vencida', 'pronto'].includes(req.query.estado) ? req.query.estado : null;
    let limite = parseInt(req.query.limit, 10);
    if (!Number.isInteger(limite) || limite < 1) limite = LIMITE_POR_DEFECTO;
    limite = Math.min(limite, LIMITE_MAXIMO);

    const condiciones = [];
    const params = [];
    if (q) {
      // Buscar "jose" tiene que encontrar a "José": se comparan ambos lados
      // sin tildes. TRANSLATE no necesita extensiones de Postgres, así que
      // funciona igual en Railway, en Supabase o en cualquier servidor.
      params.push(`%${sinTildes(q)}%`);
      const p = `$${params.length}`;
      condiciones.push(
        `(TRANSLATE(LOWER(nombre), 'áàäâãéèëêíìïîóòöôõúùüûñç', 'aaaaaeeeeiiiiooooouuuunc') LIKE ${p}` +
        ` OR COALESCE(dni,'') LIKE ${p} OR COALESCE(REPLACE(telefono,' ',''),'') LIKE ${p})`
      );
    }
    if (estado === 'activa') condiciones.push(`estado = 'activa'`);
    else if (estado === 'vencida') condiciones.push(`estado = 'vencida'`);
    else if (estado === 'pronto') {
      params.push(en7);
      condiciones.push(`estado = 'activa' AND fecha_vence <= $${params.length}`);
    }
    const where = condiciones.length ? `WHERE ${condiciones.join(' AND ')}` : '';

    const totalRow = await queryOne(`SELECT COUNT(*)::int AS total FROM memberships ${where}`, params);
    const items = await queryAll(
      `SELECT * FROM memberships ${where} ORDER BY fecha_vence DESC, id DESC LIMIT ${limite}`,
      params
    );

    res.json({ items, total: (totalRow && totalRow.total) || 0, limite });
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al obtener matrículas');
  }
});

// Socios que vencen en los próximos N días: la lista para recordarles por
// WhatsApp antes de que se vayan. Sin esto había que revisar a ojo.
router.get('/por-vencer', async (req, res) => {
  try {
    await refreshStates();
    const hoy = todayISO(config.TZ);
    let dias = parseInt(req.query.dias, 10);
    if (!Number.isInteger(dias) || dias < 0 || dias > 90) dias = 7;
    const hasta = addDays(hoy, dias);
    const rows = await queryAll(
      `SELECT id, nombre, telefono, concepto, fecha_vence, estado
       FROM memberships
       WHERE estado = 'activa' AND fecha_vence >= $1 AND fecha_vence <= $2
       ORDER BY fecha_vence ASC, nombre ASC
       LIMIT 500`,
      [hoy, hasta]
    );
    res.json(rows);
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al obtener socios por vencer');
  }
});

// Ficha completa de un socio: todo lo que pagó y todas las veces que vino.
// Antes, para saber si alguien estaba al día había que buscarlo a ojo en la
// lista general y no existía forma de ver su historial.
router.get('/:id/historial', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const socio = await queryOne('SELECT * FROM memberships WHERE id = $1', [id]);
    if (!socio) return res.status(404).json({ error: 'Socio no encontrado' });

    const [pagos, visitas, totales] = await Promise.all([
      queryAll(
        `SELECT id, tipo, concepto, monto, metodo, fecha_pago, anulado, motivo_anulacion
         FROM payments WHERE membership_id = $1
         ORDER BY fecha_pago DESC, id DESC LIMIT 200`,
        [id]
      ),
      queryAll(
        `SELECT fecha, hora FROM attendance WHERE membership_id = $1
         ORDER BY fecha DESC LIMIT 60`,
        [id]
      ),
      queryOne(
        `SELECT COALESCE(SUM(monto),0)::numeric(14,2) AS total_pagado,
                COUNT(*)::int AS veces_pago,
                (SELECT COUNT(*)::int FROM attendance WHERE membership_id = $1) AS total_visitas,
                (SELECT MAX(fecha) FROM attendance WHERE membership_id = $1) AS ultima_visita
         FROM payments WHERE membership_id = $1 AND anulado = FALSE`,
        [id]
      ),
    ]);

    res.json({
      socio,
      pagos,
      visitas,
      resumen: {
        total_pagado: Number(totales.total_pagado),
        veces_pago: totales.veces_pago,
        total_visitas: totales.total_visitas,
        ultima_visita: totales.ultima_visita,
      },
    });
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al obtener el historial del socio');
  }
});

// Libro de pagos individual (para el CSV y auditoría).
router.get('/pagos', async (req, res) => {
  try {
    const desde = parseFecha(req.query.desde) || '2000-01-01';
    const hasta = parseFecha(req.query.hasta) || '2200-12-31';
    // Por defecto no se listan los pagos anulados; con ?incluirAnulados=1 sí,
    // por si hace falta revisar qué se corrigió.
    const incluirAnulados = req.query.incluirAnulados === '1';
    // El panel solo muestra los últimos pagos; traerse el historial entero
    // (15.000 filas, 3 MB) para pintar 25 hacía lentísima la pantalla.
    let limite = parseInt(req.query.limit, 10);
    if (!Number.isInteger(limite) || limite < 1) limite = 50000;
    limite = Math.min(limite, 50000);
    const rows = await queryAll(
      `SELECT id, tipo, membership_id, nombre, concepto, monto, metodo, fecha_pago, anulado, motivo_anulacion, creado
       FROM payments
       WHERE fecha_pago >= $1 AND fecha_pago <= $2 ${incluirAnulados ? '' : 'AND anulado = FALSE'}
       ORDER BY fecha_pago DESC, id DESC
       LIMIT ${limite}`,
      [desde, hasta]
    );
    res.json(rows);
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al obtener pagos');
  }
});

// Anula un pago mal registrado. NO se borra: queda con su motivo y deja de
// sumar en los reportes, así el historial sigue siendo auditable.
router.put('/pagos/:id/anular', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  const motivo = cleanText((req.body || {}).motivo, 200) || 'Corrección del administrador';
  try {
    const r = await queryOne(
      `UPDATE payments SET anulado = TRUE, motivo_anulacion = $1
       WHERE id = $2 AND anulado = FALSE RETURNING id, monto`,
      [motivo, id]
    );
    if (!r) return res.status(404).json({ error: 'Pago no encontrado o ya anulado' });
    res.json({ message: 'Pago anulado. Ya no cuenta en las finanzas.', id: r.id });
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al anular el pago');
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

    // TODOS los periodos llevan tope superior en HOY. Sin él, un pago tecleado
    // con año equivocado (2027 en vez de 2026) se sumaba a la vez en "hoy",
    // "esta semana", "este mes" y "este año", y seguía sumando para siempre.
    //
    // Además los cinco periodos se resuelven en UNA sola consulta por tabla.
    // Antes eran 10 consultas simultáneas contra un pool de 5 conexiones: el
    // reporte se peleaba consigo mismo y podía hacer fallar el healthcheck.
    const periodos = Object.keys(rangos);
    const sumas = (tabla, col) => queryOne(
      `SELECT
         ${periodos.map((p, i) => `
           COALESCE(SUM(monto) FILTER (WHERE ${col} >= $${i + 1} AND ${col} <= $6), 0)::numeric(14,2) AS total_${p},
           COUNT(*) FILTER (WHERE ${col} >= $${i + 1} AND ${col} <= $6)::int AS cant_${p}`).join(',')}
       FROM ${tabla} WHERE anulado = FALSE`,
      [...periodos.map((p) => rangos[p]), hoy]
    );

    const [pagos, extras, gastos] = await Promise.all([
      sumas('payments', 'fecha_pago'),
      sumas('extra_sales', 'fecha'),
      sumas('expenses', 'fecha'),
    ]);

    const finanzas = {};
    periodos.forEach((p) => {
      const mat = { total: Number(pagos[`total_${p}`]), cantidad: pagos[`cant_${p}`] };
      const ext = { total: Number(extras[`total_${p}`]), cantidad: extras[`cant_${p}`] };
      const gas = { total: Number(gastos[`total_${p}`]), cantidad: gastos[`cant_${p}`] };
      const ingresos = Math.round((mat.total + ext.total) * 100) / 100;
      finanzas[p] = {
        matriculas: mat,
        extras: ext,
        gastos: gas,
        online: { total: 0, cantidad: 0 }, // compatibilidad con el frontend
        total: ingresos,
        // Lo que de verdad queda después de pagar alquiler, luz, sueldos...
        // Antes el sistema solo mostraba el dinero que entra, así que el
        // "S/ 8.000 este mes" no decía nada sobre si el gimnasio gana o pierde.
        utilidad: Math.round((ingresos - gas.total) * 100) / 100,
        cantidad: mat.cantidad + ext.cantidad,
      };
    });

    // Dinero registrado con fecha futura: no se pierde, pero se avisa aparte
    // para que la dueña pueda corregir el error de tecleo.
    const futuros = await queryOne(
      `SELECT COUNT(*)::int AS cantidad, COALESCE(SUM(monto),0)::numeric(14,2) AS total
       FROM payments WHERE fecha_pago > $1 AND anulado = FALSE`,
      [hoy]
    );
    finanzas.futuros = { cantidad: futuros.cantidad, total: Number(futuros.total) };

    // Métodos de pago del mes (pagos de matrícula + ventas extras).
    const metodos = await queryAll(
      `SELECT metodo, SUM(total)::numeric(14,2) AS total, SUM(cantidad)::int AS cantidad FROM (
         SELECT COALESCE(metodo, 'Efectivo') AS metodo, COALESCE(SUM(monto),0) AS total, COUNT(*) AS cantidad
         FROM payments WHERE fecha_pago >= $1 AND fecha_pago <= $2 AND anulado = FALSE GROUP BY 1
         UNION ALL
         SELECT COALESCE(metodo, 'Efectivo') AS metodo, COALESCE(SUM(monto),0) AS total, COUNT(*) AS cantidad
         FROM extra_sales WHERE fecha >= $1 AND fecha <= $2 AND anulado = FALSE GROUP BY 1
       ) t GROUP BY metodo ORDER BY total DESC`,
      [rangos.mes, hoy]
    );
    finanzas.metodos = metodos.map((m) => ({ metodo: m.metodo, total: Number(m.total), cantidad: m.cantidad }));

    const socios = await queryAll(`SELECT estado, COUNT(*)::int AS cantidad FROM memberships GROUP BY estado`);
    const sociosAgg = { activos: 0, vencidos: 0 };
    socios.forEach((s) => {
      if (s.estado === 'activa') sociosAgg.activos = s.cantidad;
      if (s.estado === 'vencida') sociosAgg.vencidos = s.cantidad;
    });
    finanzas.socios = sociosAgg;

    // Precio de referencia real (el plan más usado del catálogo) para las
    // proyecciones: antes el frontend multiplicaba por 80 a fuego, así que
    // cambiar los precios del gym no cambiaba las proyecciones.
    const ref = await queryOne(
      `SELECT precio FROM catalog_options
       WHERE tipo = 'matricula' AND precio IS NOT NULL AND dias BETWEEN 28 AND 31
       ORDER BY precio DESC LIMIT 1`
    );
    finanzas.precio_referencia = ref && ref.precio !== null ? Number(ref.precio) : 0;

    res.json(finanzas);
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error en finanzas');
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
    // Freno a los cobros duplicados por mala señal: si el celular pierde la
    // conexión justo después de guardar, la dueña no ve la confirmación y
    // vuelve a pulsar "Registrar pago". Sin esto quedaban dos fichas y dos
    // cobros del mismo socio, inflando los ingresos del mes.
    const yaExiste = await queryOne(
      `SELECT id FROM memberships
       WHERE nombre = $1 AND monto = $2 AND fecha_pago = $3 AND creado > NOW() - INTERVAL '3 minutes'
       LIMIT 1`,
      [nombre, monto, fechaPago]
    );
    if (yaExiste && !b.confirmar_duplicado) {
      return res.status(409).json({
        error: `Hace un momento ya se registró un pago igual de ${nombre} por S/ ${monto}. ` +
               'Si de verdad son dos cobros distintos, vuelve a intentarlo para confirmarlo.',
        duplicado: true,
        id: yaExiste.id,
      });
    }

    // La persona YA es socia y está volviendo: lo correcto es RENOVARLE, no
    // abrirle una segunda ficha. Sin esta comprobación, cada vez que alguien
    // regresaba quedaba duplicado: el conteo de socios activos se inflaba, su
    // historial se partía en dos y aparecía repetido en la lista.
    if (!b.confirmar_duplicado) {
      const mismaPersona = await queryOne(
        `SELECT id, nombre, concepto, fecha_vence, estado FROM memberships
         WHERE (COALESCE($1,'') <> '' AND dni = $1)
            OR TRANSLATE(LOWER(nombre), 'áàäâãéèëêíìïîóòöôõúùüûñç', 'aaaaaeeeeiiiiooooouuuunc')
               = TRANSLATE(LOWER($2), 'áàäâãéèëêíìïîóòöôõúùüûñç', 'aaaaaeeeeiiiiooooouuuunc')
         ORDER BY id DESC LIMIT 1`,
        [dni || null, nombre]
      );
      if (mismaPersona) {
        return res.status(409).json({
          error: `${mismaPersona.nombre} ya tiene una ficha (vence el ${mismaPersona.fecha_vence}). ` +
                 'Lo normal es RENOVARLE, así su historial de pagos queda junto.',
          socio_existente: mismaPersona,
        });
      }
    }

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
    responderError(res, err, 'MEMBERSHIPS', 'Error al crear matrícula');
  }
});

// Corregir los datos de un socio (nombre mal escrito, teléfono nuevo, cambio
// de plan, ajuste de vencimiento). Antes había que borrar la ficha y volver a
// crearla, lo que dejaba los pagos viejos huérfanos y duplicaba al socio.
router.put('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });

  const b = req.body || {};
  const nombre = requireText(b.nombre, 120);
  const concepto = requireText(b.concepto, 80);
  const fechaVence = parseFecha(b.fecha_vence);
  if (!nombre) return res.status(400).json({ error: 'El nombre es obligatorio' });
  if (!concepto) return res.status(400).json({ error: 'El concepto es obligatorio' });
  if (!fechaVence) return res.status(400).json({ error: 'Fecha de vencimiento inválida' });

  const telefono = cleanText(b.telefono, 30);
  const dni = cleanText(b.dni, 20);
  const notas = cleanText(b.notas, 500);
  const estado = fechaVence < todayISO(config.TZ) ? 'vencida' : 'activa';

  try {
    const r = await queryOne(
      `UPDATE memberships
       SET nombre = $1, telefono = $2, dni = $3, concepto = $4, notas = $5, fecha_vence = $6, estado = $7
       WHERE id = $8 RETURNING id`,
      [nombre, telefono, dni, concepto, notas, fechaVence, estado, id]
    );
    if (!r) return res.status(404).json({ error: 'Socio no encontrado' });
    // El nombre también se actualiza en los pagos futuros del libro para que
    // el reporte no muestre el nombre viejo mal escrito.
    await query(`UPDATE payments SET nombre = $1 WHERE membership_id = $2`, [nombre, id]);
    res.json({ message: 'Datos actualizados', id });
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al actualizar el socio');
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

  // Fecha real del cobro: si se registra el lunes un pago recibido el sábado,
  // el dinero debe contar en su día (y en su MES) verdadero. Antes siempre se
  // usaba la fecha de hoy, así que los cobros de fin de mes caían en el mes
  // siguiente y descuadraban el cierre.
  const fechaCobro = parseFecha(b.fecha_pago);

  try {
    const hoy = todayISO(config.TZ);
    if (fechaCobro && fechaCobro > hoy) {
      return res.status(400).json({ error: 'La fecha del pago no puede ser futura' });
    }
    const resultado = await withTransaction(async (tx) => {
      // FOR UPDATE: dos clics simultáneos en "Renovar" no se pisan entre sí.
      const r = await tx.query('SELECT * FROM memberships WHERE id = $1 FOR UPDATE', [id]);
      const m = r.rows[0];
      if (!m) return null;

      const base = (m.fecha_vence && m.fecha_vence > hoy) ? m.fecha_vence : hoy;
      const nuevoVence = addDays(base, dias);
      const metodo = b.metodo ? parseMetodo(b.metodo) : (m.metodo || 'Efectivo');
      const concepto = requireText(b.concepto, 80) || m.concepto;

      // fecha_pago también se actualiza: si no, la ficha del socio seguía
      // mostrando "Pagó: 05 ene" meses después de su última renovación.
      await tx.query(
        `UPDATE memberships SET fecha_vence = $1, estado = 'activa', monto = $2, metodo = $3, concepto = $4, fecha_pago = $5 WHERE id = $6`,
        [nuevoVence, monto, metodo, concepto, fechaCobro || hoy, id]
      );
      await tx.query(
        `INSERT INTO payments (tipo, membership_id, nombre, concepto, monto, metodo, fecha_pago)
         VALUES ('renovacion', $1, $2, $3, $4, $5, $6)`,
        [id, m.nombre, concepto, monto, metodo, fechaCobro || hoy]
      );
      return nuevoVence;
    });

    if (!resultado) return res.status(404).json({ error: 'No encontrada' });
    res.json({ message: 'Renovada', nuevoVence: resultado });
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al renovar');
  }
});

// Borra la ficha del socio. Sus pagos ya cobrados QUEDAN en el libro de
// ingresos (payments), así que los reportes históricos no cambian.
router.delete('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const r = await queryOne('DELETE FROM memberships WHERE id = $1 RETURNING id', [id]);
    if (!r) return res.status(404).json({ error: 'Socio no encontrado' });
    res.json({ message: 'Eliminada' });
  } catch (err) {
    responderError(res, err, 'MEMBERSHIPS', 'Error al eliminar');
  }
});

module.exports = router;
