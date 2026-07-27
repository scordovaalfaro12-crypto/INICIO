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
const { query, queryOne, queryAll, withTransaction } = require('../db');
const { authenticateToken, requireAdmin, requireStaff } = require('../middleware/auth');
const { parseMonto, parseId, parseDias, requireText, cleanText, parseFecha, parseMetodo } = require('../lib/validate');
const { responderError } = require('../lib/errores');

router.use(authenticateToken, requireStaff);

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
router.get('/resumen', requireAdmin, async (req, res) => {
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
  // Venta de un producto del inventario (opcional): si viene, se descuenta
  // del stock en la MISMA transacción que registra el dinero, así nunca queda
  // una venta cobrada sin descontar ni un descuento sin venta.
  const productoId = b.producto_id ? parseId(b.producto_id) : null;
  const cantidad = productoId ? (parseDias(b.cantidad !== undefined ? b.cantidad : 1, 1, 10000) || 1) : 1;

  if (!categoria) return res.status(400).json({ error: 'La categoría es obligatoria' });
  if (monto === null) return res.status(400).json({ error: 'Monto inválido' });
  if (!fecha) return res.status(400).json({ error: 'Fecha inválida (usa el selector de fecha)' });

  try {
    const resultado = await withTransaction(async (tx) => {
      let stockDespues = null;
      let nombreProducto = null;

      if (productoId) {
        // FOR UPDATE: dos ventas simultáneas del último producto no pueden
        // dejar el stock en negativo.
        const r = await tx.query('SELECT * FROM products WHERE id = $1 FOR UPDATE', [productoId]);
        const p = r.rows[0];
        if (!p) return { error: 'El producto no existe', estado: 404 };
        if (p.stock < cantidad && !b.confirmar_sin_stock) {
          return {
            error: `De "${p.nombre}" solo quedan ${p.stock} en el sistema y estás vendiendo ${cantidad}. ` +
                   'Si el conteo está mal, confirma la venta y luego ajusta el inventario.',
            sin_stock: true, disponible: p.stock, estado: 409,
          };
        }
        stockDespues = p.stock - cantidad;
        nombreProducto = p.nombre;
        await tx.query('UPDATE products SET stock = $1 WHERE id = $2', [stockDespues, productoId]);
      }

      const venta = await tx.query(
        `INSERT INTO extra_sales (categoria, descripcion, monto, metodo, fecha, notas, producto_id, cantidad, usuario_id)
         VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9) RETURNING id`,
        [categoria, cleanText(b.descripcion, 200), monto, parseMetodo(b.metodo), fecha,
         cleanText(b.notas, 500), productoId, cantidad, req.user.id]
      );
      const ventaId = venta.rows[0].id;

      if (productoId) {
        await tx.query(
          `INSERT INTO stock_movements (producto_id, tipo, cantidad, stock_resultante, motivo, venta_id, fecha, usuario_id)
           VALUES ($1, 'salida', $2, $3, $4, $5, $6, $7)`,
          [productoId, -cantidad, stockDespues, `Venta de ${cantidad} unidad(es)`, ventaId, fecha, req.user.id]
        );
      }
      return { id: ventaId, stock: stockDespues, producto: nombreProducto };
    });

    if (resultado.error) {
      return res.status(resultado.estado).json({
        error: resultado.error, sin_stock: resultado.sin_stock, disponible: resultado.disponible,
      });
    }
    res.status(201).json({
      id: resultado.id,
      stock_restante: resultado.stock,
      aviso: (resultado.stock !== null && resultado.stock <= 0)
        ? `Se acabó el stock de ${resultado.producto}. Repón antes de seguir vendiéndolo.`
        : null,
    });
  } catch (err) {
    responderError(res, err, 'EXTRAS', 'Error al guardar');
  }
});

router.delete('/:id', requireAdmin, async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const resultado = await withTransaction(async (tx) => {
      const r = await tx.query(
        `UPDATE extra_sales SET anulado = TRUE, motivo_anulacion = 'Anulada por el administrador'
         WHERE id = $1 AND anulado = FALSE RETURNING id, producto_id, cantidad, fecha`,
        [id]
      );
      const venta = r.rows[0];
      if (!venta) return { error: 'Venta no encontrada o ya anulada' };

      // Si la venta descontó stock, al anularla el producto VUELVE al almacén:
      // si no, cada corrección iría dejando unidades fantasma perdidas.
      if (venta.producto_id) {
        const p = await tx.query('SELECT * FROM products WHERE id = $1 FOR UPDATE', [venta.producto_id]);
        if (p.rows[0]) {
          const nuevo = p.rows[0].stock + venta.cantidad;
          await tx.query('UPDATE products SET stock = $1 WHERE id = $2', [nuevo, venta.producto_id]);
          await tx.query(
            `INSERT INTO stock_movements (producto_id, tipo, cantidad, stock_resultante, motivo, venta_id, fecha)
             VALUES ($1, 'devolucion', $2, $3, 'Venta anulada: el producto vuelve al stock', $4, $5)`,
            [venta.producto_id, venta.cantidad, nuevo, id, venta.fecha]
          );
          return { devuelto: venta.cantidad, nombre: p.rows[0].nombre, stock: nuevo };
        }
      }
      return { devuelto: 0 };
    });

    if (resultado.error) return res.status(404).json({ error: resultado.error });
    res.json({
      message: resultado.devuelto
        ? `Venta anulada. ${resultado.devuelto} unidad(es) de ${resultado.nombre} vuelven al stock (ahora hay ${resultado.stock}).`
        : 'Venta anulada: ya no cuenta en las finanzas',
    });
  } catch (err) {
    responderError(res, err, 'EXTRAS', 'Error al anular la venta');
  }
});

module.exports = router;
