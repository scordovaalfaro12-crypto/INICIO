// ==========================================================
//  middleware/auth.js — Sesión y permisos
//
//  Dos tipos de usuario:
//   · admin      — la dueña. Ve y hace todo, incluido el dinero.
//   · recepcion  — quien atiende el mostrador. Puede cobrar, renovar,
//                  marcar entradas y vender productos, pero NO ve las
//                  finanzas ni los gastos, no anula pagos ajenos, no
//                  toca los precios ni el inventario, y no puede
//                  descargar ni restaurar respaldos.
//
//  El reparto no es por desconfianza: es para que quien atiende no
//  tenga delante información que no necesita, y para que un descuido
//  suyo no pueda deshacer la contabilidad del negocio.
// ==========================================================

const jwt = require('jsonwebtoken');
const config = require('../config');

const ROLES = ['admin', 'recepcion'];

// El token dice quién eres; la BASE DE DATOS dice qué puedes hacer AHORA.
//
// Antes se confiaba en el rol grabado dentro del token. Como el token dura 24
// horas, quitarle el acceso a alguien —bajarle el rol o darle de baja— no
// surtía efecto hasta el día siguiente: comprobado, un usuario degradado
// seguía viendo las finanzas y uno dado de baja seguía dentro. Ahora cada
// petición confirma el estado real de la persona, así retirar un permiso es
// inmediato.
function authenticateToken(req, res, next) {
  const authHeader = req.headers['authorization'] || '';
  const token = authHeader.startsWith('Bearer ') ? authHeader.slice(7) : null;
  if (!token) return res.status(401).json({ error: 'Acceso no autorizado' });

  jwt.verify(token, config.JWT_SECRET, async (err, datos) => {
    if (err) return res.status(401).json({ error: 'Sesión expirada. Vuelve a iniciar sesión.' });
    try {
      // require aquí dentro para no crear una dependencia circular con db.js.
      const { queryOne } = require('../db');
      const actual = await queryOne(
        'SELECT id, email, role, activo, firstname FROM users WHERE id = $1',
        [datos.id]
      );
      if (!actual || !actual.activo) {
        return res.status(401).json({ error: 'Tu acceso al sistema fue retirado. Consulta con la administradora.' });
      }
      req.user = { id: actual.id, email: actual.email, role: actual.role, firstname: actual.firstname };
      next();
    } catch (e) {
      // Si la base no responde, la sesión no se puede confirmar: mejor pedir
      // que reintenten que dejar pasar a alguien sin comprobar quién es.
      return res.status(503).json({
        error: 'No se pudo verificar tu sesión (base de datos no disponible). Reintenta en unos segundos.',
        reintentable: true,
      });
    }
  });
}

// Solo la dueña: dinero, precios, inventario, respaldos y usuarios.
function requireAdmin(req, res, next) {
  if (!req.user || req.user.role !== 'admin') {
    return res.status(403).json({
      error: 'Esta parte del sistema es solo para la administradora.',
      requiere_admin: true,
    });
  }
  next();
}

// Cualquiera que atienda el mostrador (incluida la dueña).
function requireStaff(req, res, next) {
  if (!req.user || !ROLES.includes(req.user.role)) {
    return res.status(403).json({ error: 'Tu usuario no tiene permiso para esta acción.' });
  }
  next();
}

function esAdmin(req) {
  return !!(req.user && req.user.role === 'admin');
}

module.exports = { authenticateToken, requireAdmin, requireStaff, esAdmin, ROLES };
