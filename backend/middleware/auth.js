const jwt = require('jsonwebtoken');
const config = require('../config');

function authenticateToken(req, res, next) {
  const authHeader = req.headers['authorization'] || '';
  const token = authHeader.startsWith('Bearer ') ? authHeader.slice(7) : null;
  if (!token) return res.status(401).json({ error: 'Acceso no autorizado' });

  jwt.verify(token, config.JWT_SECRET, (err, user) => {
    if (err) return res.status(401).json({ error: 'Sesión expirada. Vuelve a iniciar sesión.' });
    req.user = user;
    next();
  });
}

function requireAdmin(req, res, next) {
  if (!req.user || req.user.role !== 'admin') {
    return res.status(403).json({ error: 'Requiere permisos de administrador' });
  }
  next();
}

module.exports = { authenticateToken, requireAdmin };
