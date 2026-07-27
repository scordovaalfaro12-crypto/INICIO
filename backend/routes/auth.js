const express = require('express');
const router = express.Router();
const bcrypt = require('bcryptjs');
const jwt = require('jsonwebtoken');
const config = require('../config');
const { queryOne, query } = require('../db');
const { authenticateToken } = require('../middleware/auth');
const { loginRateLimit } = require('../middleware/security');
const { cleanText } = require('../lib/validate');
const { responderError } = require('../lib/errores');

// Hash de descarte con el que se compara cuando el email NO existe. Sin esto,
// un email inexistente respondía al instante y uno real tardaba lo que tarda
// bcrypt: el tiempo de respuesta delataba qué usuarios existen.
const HASH_SENUELO = bcrypt.hashSync('contraseña_que_nadie_usa_jamas', 10);

router.post('/login', loginRateLimit, async (req, res) => {
  const email = cleanText(req.body && req.body.email, 200).toLowerCase();
  const password = typeof (req.body && req.body.password) === 'string' ? req.body.password : '';
  if (!email || !password) return res.status(400).json({ error: 'Email y contraseña requeridos' });

  try {
    const user = await queryOne('SELECT * FROM users WHERE email = $1', [email]);
    // Mismo mensaje y mismo tiempo exista o no el usuario: no se regala
    // información a un atacante.
    const passwordOk = await bcrypt.compare(password, user ? user.password : HASH_SENUELO);
    if (!user || !passwordOk) {
      req.loginFallido();
      return res.status(401).json({ error: 'Usuario o contraseña incorrectos' });
    }
    req.loginExitoso();
    const token = jwt.sign(
      { id: user.id, email: user.email, role: user.role },
      config.JWT_SECRET,
      { expiresIn: config.TOKEN_TTL }
    );
    res.json({
      token,
      user: { id: user.id, firstname: user.firstname, lastname: user.lastname, email: user.email, role: user.role },
    });
  } catch (err) {
    responderError(res, err, 'AUTH', 'Error del servidor');
  }
});

router.put('/change-password', authenticateToken, async (req, res) => {
  const actual = typeof (req.body && req.body.actual) === 'string' ? req.body.actual : '';
  const nueva = typeof (req.body && req.body.nueva) === 'string' ? req.body.nueva : '';
  if (!actual || !nueva) return res.status(400).json({ error: 'Datos incompletos' });
  if (nueva.length < 6) return res.status(400).json({ error: 'La nueva contraseña debe tener mínimo 6 caracteres' });
  if (nueva.length > 200) return res.status(400).json({ error: 'Contraseña demasiado larga' });

  try {
    const user = await queryOne('SELECT * FROM users WHERE id = $1', [req.user.id]);
    if (!user || !(await bcrypt.compare(actual, user.password))) {
      return res.status(401).json({ error: 'Contraseña actual incorrecta' });
    }
    const hash = await bcrypt.hash(nueva, 10);
    await query('UPDATE users SET password = $1 WHERE id = $2', [hash, req.user.id]);
    res.json({ message: 'Contraseña actualizada' });
  } catch (err) {
    responderError(res, err, 'AUTH', 'Error del servidor');
  }
});

router.get('/me', authenticateToken, async (req, res) => {
  try {
    const user = await queryOne('SELECT id, firstname, lastname, email, role FROM users WHERE id = $1', [req.user.id]);
    if (!user) return res.status(404).json({ error: 'Usuario no encontrado' });
    res.json(user);
  } catch (err) {
    responderError(res, err, 'AUTH', 'Error del servidor');
  }
});

module.exports = router;
