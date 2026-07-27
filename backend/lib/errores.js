// ==========================================================
//  lib/errores.js — Respuestas de error uniformes
//
//  Antes, si la base de datos estaba caída, TODAS las rutas
//  devolvían 500 "Error al obtener matrículas": un mensaje que
//  parece una falla del programa cuando en realidad es una
//  interrupción temporal de la nube. La dueña no puede saber si
//  debe esperar o llamar a alguien.
//
//  Ahora se distingue: 503 = "esperá, la base volverá sola",
//  500 = "esto sí es un error de verdad".
// ==========================================================

// Códigos y mensajes que emite `pg` cuando el problema es de RED o
// de servidor no disponible, no de la consulta en sí.
const CODIGOS_CONEXION = new Set([
  'ECONNREFUSED',   // la base no acepta conexiones (apagada o reiniciando)
  'ENOTFOUND',      // el host no resuelve (DNS aún no responde)
  'ETIMEDOUT',      // la red se quedó colgada
  'ECONNRESET',     // el servidor cortó la conexión a media consulta
  'EPIPE',
  'EHOSTUNREACH',
  'ENETUNREACH',
  'EAI_AGAIN',      // fallo temporal de DNS
  '57P01',          // admin_shutdown: el servidor se está apagando
  '57P02',          // crash_shutdown
  '57P03',          // cannot_connect_now: arrancando todavía
  '53300',          // too_many_connections
  '08000', '08003', '08006', '08001', '08004', // familia connection_exception
  'XX000',          // internal_error: Supabase lo usa al despertar del pausado
]);

const TEXTOS_CONEXION = /terminated unexpectedly|Connection terminated|timeout exceeded|server closed the connection|connection is closed|Client has encountered a connection error/i;

function esErrorDeConexion(err) {
  if (!err) return false;
  if (CODIGOS_CONEXION.has(err.code)) return true;
  return TEXTOS_CONEXION.test(err.message || '');
}

// Respuesta estándar para el catch de cualquier ruta.
// `etiqueta` sale en los logs; `mensaje` es lo que ve la usuaria.
function responderError(res, err, etiqueta, mensaje) {
  if (esErrorDeConexion(err)) {
    console.error(`[${etiqueta}] Base de datos no disponible: ${err.message}`);
    return res.status(503).json({
      error: 'Sin conexión con la base de datos. El sistema lo reintenta solo; espera unos segundos y vuelve a intentar.',
      reintentable: true,
    });
  }
  console.error(`[${etiqueta}]`, err);
  return res.status(500).json({ error: mensaje });
}

module.exports = { esErrorDeConexion, responderError };
