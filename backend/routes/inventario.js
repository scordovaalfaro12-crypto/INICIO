// ==========================================================
//  routes/inventario.js — Productos y control de stock
//
//  El gimnasio vende aguas, proteínas, ropa. Antes esas ventas se
//  anotaban solo como dinero: nadie sabía cuántas quedaban hasta
//  abrir la nevera y ver el hueco.
//
//  Dos reglas de fondo:
//   1. El stock NUNCA se toca a mano sin dejar rastro. Cada cambio
//      —venta, reposición, ajuste de conteo— queda anotado con su
//      motivo, así que un número raro siempre se puede explicar.
//   2. Las cuentas de stock se hacen dentro de una transacción con
//      la fila bloqueada: dos ventas simultáneas del último producto
//      no pueden dejar el stock en -1.
// ==========================================================

const express = require('express');
const router = express.Router();
const { query, queryOne, queryAll, withTransaction } = require('../db');
const { authenticateToken, requireAdmin, requireStaff } = require('../middleware/auth');
const config = require('../config');
const { todayISO } = require('../lib/dates');
const { parseMonto, parseId, parseDias, requireText, cleanText, parseFecha } = require('../lib/validate');
const { responderError } = require('../lib/errores');

// Quien atiende necesita VER qué productos hay y cuánto cuestan para poder
// vender. Reponer, ajustar el conteo, cambiar precios y ver cuánto vale el
// inventario es cosa de la dueña.
router.use(authenticateToken, requireStaff);

// Cantidad de unidades: entero entre 1 y 10.000.
function parseCantidad(v) {
  return parseDias(v, 1, 10000);
}

// Listado de productos con su situación de stock.
router.get('/', async (req, res) => {
  try {
    const soloBajos = req.query.bajo_stock === '1';
    const incluirInactivos = req.query.incluir_inactivos === '1';
    const condiciones = [];
    if (!incluirInactivos) condiciones.push('activo = TRUE');
    if (soloBajos) condiciones.push('stock <= stock_minimo');
    const where = condiciones.length ? `WHERE ${condiciones.join(' AND ')}` : '';

    const rows = await queryAll(
      `SELECT id, nombre, categoria, precio, stock, stock_minimo, activo,
              (stock <= stock_minimo) AS falta_reponer,
              (stock <= 0) AS agotado
       FROM products ${where}
       ORDER BY (stock <= stock_minimo) DESC, categoria ASC, nombre ASC
       LIMIT 500`
    );
    res.json(rows.map((p) => ({ ...p, precio: Number(p.precio) })));
  } catch (err) {
    responderError(res, err, 'INVENTARIO', 'Error al obtener los productos');
  }
});

// Resumen para las tarjetas: cuánto vale el inventario y qué hay que reponer.
router.get('/resumen', requireAdmin, async (req, res) => {
  try {
    const [totales, faltantes] = await Promise.all([
      queryOne(
        `SELECT COUNT(*)::int AS productos,
                COALESCE(SUM(stock), 0)::int AS unidades,
                COALESCE(SUM(stock * precio), 0)::numeric(14,2) AS valor,
                COUNT(*) FILTER (WHERE stock <= stock_minimo)::int AS por_reponer,
                COUNT(*) FILTER (WHERE stock <= 0)::int AS agotados
         FROM products WHERE activo = TRUE`
      ),
      queryAll(
        `SELECT id, nombre, stock, stock_minimo FROM products
         WHERE activo = TRUE AND stock <= stock_minimo
         ORDER BY stock ASC, nombre ASC LIMIT 20`
      ),
    ]);
    res.json({
      productos: totales.productos,
      unidades: totales.unidades,
      valor: Number(totales.valor),
      por_reponer: totales.por_reponer,
      agotados: totales.agotados,
      faltantes,
    });
  } catch (err) {
    responderError(res, err, 'INVENTARIO', 'Error al obtener el resumen de inventario');
  }
});

// Historial de movimientos de un producto: de dónde salió cada unidad.
router.get('/:id/movimientos', requireAdmin, async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const producto = await queryOne('SELECT * FROM products WHERE id = $1', [id]);
    if (!producto) return res.status(404).json({ error: 'Producto no encontrado' });
    const movimientos = await queryAll(
      `SELECT id, tipo, cantidad, stock_resultante, motivo, fecha, creado
       FROM stock_movements WHERE producto_id = $1
       ORDER BY id DESC LIMIT 200`,
      [id]
    );
    res.json({ producto: { ...producto, precio: Number(producto.precio) }, movimientos });
  } catch (err) {
    responderError(res, err, 'INVENTARIO', 'Error al obtener el historial del producto');
  }
});

router.post('/', requireAdmin, async (req, res) => {
  const b = req.body || {};
  const nombre = requireText(b.nombre, 80);
  const precio = parseMonto(b.precio);
  const categoria = cleanText(b.categoria, 60) || 'Otros';

  if (!nombre) return res.status(400).json({ error: 'El nombre del producto es obligatorio' });
  if (precio === null) return res.status(400).json({ error: 'Precio inválido' });

  let stockInicial = 0;
  if (b.stock !== undefined && b.stock !== null && b.stock !== '') {
    stockInicial = parseDias(b.stock, 0, 100000);
    if (stockInicial === null) return res.status(400).json({ error: 'Stock inicial inválido' });
  }
  let minimo = 3;
  if (b.stock_minimo !== undefined && b.stock_minimo !== null && b.stock_minimo !== '') {
    minimo = parseDias(b.stock_minimo, 0, 10000);
    if (minimo === null) return res.status(400).json({ error: 'Aviso de stock mínimo inválido' });
  }

  try {
    const hoy = todayISO(config.TZ);
    const id = await withTransaction(async (tx) => {
      const r = await tx.query(
        `INSERT INTO products (nombre, categoria, precio, stock, stock_minimo)
         VALUES ($1, $2, $3, $4, $5) RETURNING id`,
        [nombre, categoria, precio, stockInicial, minimo]
      );
      const nuevoId = r.rows[0].id;
      if (stockInicial > 0) {
        await tx.query(
          `INSERT INTO stock_movements (producto_id, tipo, cantidad, stock_resultante, motivo, fecha, usuario_id)
           VALUES ($1, 'entrada', $2, $2, 'Stock inicial al crear el producto', $3, $4)`,
          [nuevoId, stockInicial, hoy, req.user.id]
        );
      }
      return nuevoId;
    });
    res.status(201).json({ id });
  } catch (err) {
    if (err.code === '23505' || err.code === '23000') {
      return res.status(409).json({ error: 'Ya existe un producto con ese nombre' });
    }
    responderError(res, err, 'INVENTARIO', 'Error al crear el producto');
  }
});

router.put('/:id', requireAdmin, async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  const b = req.body || {};
  const nombre = requireText(b.nombre, 80);
  const precio = parseMonto(b.precio);
  if (!nombre) return res.status(400).json({ error: 'El nombre es obligatorio' });
  if (precio === null) return res.status(400).json({ error: 'Precio inválido' });
  const minimo = parseDias(b.stock_minimo !== undefined ? b.stock_minimo : 3, 0, 10000);
  if (minimo === null) return res.status(400).json({ error: 'Aviso de stock mínimo inválido' });

  try {
    // El stock NO se cambia aquí: se mueve con entrada/ajuste, que dejan rastro.
    const r = await queryOne(
      `UPDATE products SET nombre = $1, categoria = $2, precio = $3, stock_minimo = $4
       WHERE id = $5 RETURNING id`,
      [nombre, cleanText(b.categoria, 60) || 'Otros', precio, minimo, id]
    );
    if (!r) return res.status(404).json({ error: 'Producto no encontrado' });
    res.json({ message: 'Producto actualizado' });
  } catch (err) {
    if (err.code === '23505') return res.status(409).json({ error: 'Ya existe un producto con ese nombre' });
    responderError(res, err, 'INVENTARIO', 'Error al actualizar el producto');
  }
});

// Reposición: llegó mercadería. Suma stock y lo deja anotado.
router.post('/:id/entrada', requireAdmin, async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  const cantidad = parseCantidad((req.body || {}).cantidad);
  if (cantidad === null) return res.status(400).json({ error: 'Cantidad inválida (1 a 10.000)' });
  const motivo = cleanText((req.body || {}).motivo, 200) || 'Reposición de mercadería';
  const fecha = parseFecha((req.body || {}).fecha) || todayISO(config.TZ);

  try {
    const resultado = await withTransaction(async (tx) => {
      const r = await tx.query('SELECT * FROM products WHERE id = $1 FOR UPDATE', [id]);
      const p = r.rows[0];
      if (!p) return { error: 'Producto no encontrado', estado: 404 };
      const nuevo = p.stock + cantidad;
      await tx.query('UPDATE products SET stock = $1 WHERE id = $2', [nuevo, id]);
      await tx.query(
        `INSERT INTO stock_movements (producto_id, tipo, cantidad, stock_resultante, motivo, fecha, usuario_id)
         VALUES ($1, 'entrada', $2, $3, $4, $5, $6)`,
        [id, cantidad, nuevo, motivo, fecha, req.user.id]
      );
      return { nombre: p.nombre, stock: nuevo };
    });
    if (resultado.error) return res.status(resultado.estado).json({ error: resultado.error });
    res.json({ message: `${resultado.nombre}: ahora hay ${resultado.stock} unidad(es)`, stock: resultado.stock });
  } catch (err) {
    responderError(res, err, 'INVENTARIO', 'Error al registrar la entrada de stock');
  }
});

// Ajuste por conteo físico: se contó la nevera y el número real es otro.
// Guarda la diferencia y el motivo en vez de sobrescribir en silencio.
router.post('/:id/ajuste', requireAdmin, async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  const b = req.body || {};
  const contado = parseDias(b.stock_real, 0, 100000);
  if (contado === null) return res.status(400).json({ error: 'Cantidad contada inválida' });
  const motivo = cleanText(b.motivo, 200) || 'Ajuste por conteo físico';

  try {
    const hoy = todayISO(config.TZ);
    const resultado = await withTransaction(async (tx) => {
      const r = await tx.query('SELECT * FROM products WHERE id = $1 FOR UPDATE', [id]);
      const p = r.rows[0];
      if (!p) return { error: 'Producto no encontrado', estado: 404 };
      const diferencia = contado - p.stock;
      if (diferencia === 0) return { nombre: p.nombre, stock: contado, diferencia: 0 };
      await tx.query('UPDATE products SET stock = $1 WHERE id = $2', [contado, id]);
      await tx.query(
        `INSERT INTO stock_movements (producto_id, tipo, cantidad, stock_resultante, motivo, fecha, usuario_id)
         VALUES ($1, 'ajuste', $2, $3, $4, $5, $6)`,
        [id, diferencia, contado, `${motivo} (el sistema decía ${p.stock})`, hoy, req.user.id]
      );
      return { nombre: p.nombre, stock: contado, diferencia };
    });
    if (resultado.error) return res.status(resultado.estado).json({ error: resultado.error });
    res.json({
      message: resultado.diferencia === 0
        ? `${resultado.nombre}: el conteo coincide, no había que ajustar nada`
        : `${resultado.nombre} ajustado a ${resultado.stock} unidad(es) (${resultado.diferencia > 0 ? '+' : ''}${resultado.diferencia})`,
      stock: resultado.stock,
      diferencia: resultado.diferencia,
    });
  } catch (err) {
    responderError(res, err, 'INVENTARIO', 'Error al ajustar el stock');
  }
});

// Retirar un producto del mostrador sin borrar su historial de ventas.
router.delete('/:id', requireAdmin, async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  try {
    const r = await queryOne(
      `UPDATE products SET activo = FALSE WHERE id = $1 AND activo = TRUE RETURNING nombre`,
      [id]
    );
    if (!r) return res.status(404).json({ error: 'Producto no encontrado o ya retirado' });
    res.json({ message: `${r.nombre} retirado del mostrador. Sus ventas siguen en el historial.` });
  } catch (err) {
    responderError(res, err, 'INVENTARIO', 'Error al retirar el producto');
  }
});

module.exports = router;
