// ==========================================================
//  routes/usuarios.js — Quién puede entrar al sistema
//
//  Solo la administradora gestiona esto. Sirve para dar de alta a
//  quien atiende el mostrador con un usuario propio: así cada cobro
//  queda con su nombre y nadie tiene que compartir la contraseña de
//  la dueña (que es la que da acceso a las finanzas).
//
//  Un usuario NUNCA se borra: se desactiva. Si se borrara, todos los
//  pagos y entradas que registró se quedarían sin autor.
// ==========================================================

const express = require('express');
const router = express.Router();
const bcrypt = require('bcryptjs');
const { query, queryOne, queryAll } = require('../db');
const { authenticateToken, requireAdmin, ROLES } = require('../middleware/auth');
const { parseId, requireText, cleanText } = require('../lib/validate');
const { responderError } = require('../lib/errores');

router.use(authenticateToken, requireAdmin);

const MIN_PASSWORD = 6;

function parseRol(v) {
  return ROLES.includes(v) ? v : null;
}

// Correo con forma razonable. No se valida contra el mundo real: es la
// etiqueta con la que esa persona inicia sesión.
function parseEmail(v) {
  const s = cleanText(v, 200).toLowerCase();
  return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(s) ? s : null;
}

router.get('/', async (req, res) => {
  try {
    const rows = await queryAll(
      `SELECT id, firstname, lastname, email, role, activo, creado,
              (SELECT COUNT(*)::int FROM payments p WHERE p.usuario_id = u.id) AS pagos_registrados
       FROM users u ORDER BY activo DESC, id ASC`
    );
    res.json(rows);
  } catch (err) {
    responderError(res, err, 'USUARIOS', 'Error al obtener los usuarios');
  }
});

router.post('/', async (req, res) => {
  const b = req.body || {};
  const nombre = requireText(b.firstname, 60);
  const apellido = cleanText(b.lastname, 60);
  const email = parseEmail(b.email);
  const rol = parseRol(b.role);
  const password = typeof b.password === 'string' ? b.password : '';

  if (!nombre) return res.status(400).json({ error: 'El nombre es obligatorio' });
  if (!email) return res.status(400).json({ error: 'El correo no tiene un formato válido' });
  if (!rol) return res.status(400).json({ error: 'Elige si es administradora o recepción' });
  if (password.length < MIN_PASSWORD) {
    return res.status(400).json({ error: `La contraseña debe tener mínimo ${MIN_PASSWORD} caracteres` });
  }
  if (password.length > 200) return res.status(400).json({ error: 'Contraseña demasiado larga' });

  try {
    const hash = await bcrypt.hash(password, 10);
    const r = await query(
      `INSERT INTO users (firstname, lastname, email, password, role) VALUES ($1, $2, $3, $4, $5) RETURNING id`,
      [nombre, apellido, email, hash, rol]
    );
    res.status(201).json({ id: r.rows[0].id, message: `${nombre} ya puede entrar al sistema` });
  } catch (err) {
    if (err.code === '23505') return res.status(409).json({ error: 'Ya existe un usuario con ese correo' });
    responderError(res, err, 'USUARIOS', 'Error al crear el usuario');
  }
});

// Cambiar nombre o rol. La contraseña se cambia aparte, a propósito.
router.put('/:id', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  const b = req.body || {};
  const nombre = requireText(b.firstname, 60);
  const rol = parseRol(b.role);
  if (!nombre) return res.status(400).json({ error: 'El nombre es obligatorio' });
  if (!rol) return res.status(400).json({ error: 'Rol inválido' });

  try {
    // No se puede dejar el sistema sin ninguna administradora: si esta fuera
    // la última y se le bajara el rol, nadie podría volver a entrar a las
    // finanzas ni a los usuarios.
    if (rol !== 'admin') {
      const admins = await queryOne(
        `SELECT COUNT(*)::int AS n FROM users WHERE role = 'admin' AND activo = TRUE AND id <> $1`, [id]
      );
      if (admins.n === 0) {
        return res.status(400).json({ error: 'Debe quedar al menos una administradora activa en el sistema.' });
      }
    }
    const r = await queryOne(
      `UPDATE users SET firstname = $1, lastname = $2, role = $3 WHERE id = $4 RETURNING id`,
      [nombre, cleanText(b.lastname, 60), rol, id]
    );
    if (!r) return res.status(404).json({ error: 'Usuario no encontrado' });
    res.json({ message: 'Usuario actualizado' });
  } catch (err) {
    responderError(res, err, 'USUARIOS', 'Error al actualizar el usuario');
  }
});

// Restablecer la contraseña de otra persona (la olvidó, o se va del puesto).
router.put('/:id/password', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  const nueva = typeof (req.body || {}).password === 'string' ? req.body.password : '';
  if (nueva.length < MIN_PASSWORD) {
    return res.status(400).json({ error: `La contraseña debe tener mínimo ${MIN_PASSWORD} caracteres` });
  }
  if (nueva.length > 200) return res.status(400).json({ error: 'Contraseña demasiado larga' });

  try {
    const hash = await bcrypt.hash(nueva, 10);
    const r = await queryOne('UPDATE users SET password = $1 WHERE id = $2 RETURNING firstname', [hash, id]);
    if (!r) return res.status(404).json({ error: 'Usuario no encontrado' });
    res.json({ message: `Contraseña de ${r.firstname} cambiada. Dísela en persona, no por escrito.` });
  } catch (err) {
    responderError(res, err, 'USUARIOS', 'Error al cambiar la contraseña');
  }
});

// Dar de baja o volver a dar de alta.
router.put('/:id/activo', async (req, res) => {
  const id = parseId(req.params.id);
  if (!id) return res.status(400).json({ error: 'ID inválido' });
  const activo = (req.body || {}).activo === true;

  try {
    if (id === req.user.id && !activo) {
      return res.status(400).json({ error: 'No puedes desactivarte a ti misma.' });
    }
    if (!activo) {
      const admins = await queryOne(
        `SELECT COUNT(*)::int AS n FROM users WHERE role = 'admin' AND activo = TRUE AND id <> $1`, [id]
      );
      if (admins.n === 0) {
        return res.status(400).json({ error: 'Debe quedar al menos una administradora activa en el sistema.' });
      }
    }
    const r = await queryOne('UPDATE users SET activo = $1 WHERE id = $2 RETURNING firstname', [activo, id]);
    if (!r) return res.status(404).json({ error: 'Usuario no encontrado' });
    res.json({
      message: activo
        ? `${r.firstname} vuelve a tener acceso`
        : `${r.firstname} ya no puede entrar. Todo lo que registró sigue en el historial.`,
    });
  } catch (err) {
    responderError(res, err, 'USUARIOS', 'Error al cambiar el acceso');
  }
});

module.exports = router;
