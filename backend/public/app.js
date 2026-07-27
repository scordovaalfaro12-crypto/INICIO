// ==========================================================
//  app.js — Cliente API para sistema interno ZONA VIP GYM
// ==========================================================

// API_BASE relativo - el backend sirve tanto el frontend como las APIs en el mismo puerto
const API_BASE = '/api';
const TOKEN_KEY = 'zvg_token';

function getToken() { return localStorage.getItem(TOKEN_KEY); }
function setToken(t) { localStorage.setItem(TOKEN_KEY, t); }
function clearToken() { localStorage.removeItem(TOKEN_KEY); }

// Todas las llamadas autenticadas pasan por aquí.
//  - 401: la sesión expiró -> se limpia el token y se vuelve al login, en vez
//    de dejar la pantalla "muerta" mostrando errores.
//  - 503: la base de datos está reconectando -> se avisa que es temporal.
//  - Sin internet: se devuelve un error claro en vez de reventar la promesa,
//    para que la pantalla nunca se quede a medias sin explicación.
async function authFetch(url, options = {}) {
  let res;
  try {
    res = await fetch(url, {
      ...options,
      headers: {
        ...(options.headers || {}),
        'Authorization': `Bearer ${getToken()}`,
      },
    });
  } catch (e) {
    return { error: 'Sin conexión a internet. Revisa la señal y vuelve a intentar.', sinRed: true };
  }

  if (res.status === 401) {
    clearToken();
    window.location.href = 'login.html';
    return { error: 'Sesión expirada' };
  }

  let datos;
  try {
    datos = await res.json();
  } catch (e) {
    return { error: 'Respuesta inválida del servidor' };
  }
  if (!res.ok && datos && !datos.error) datos.error = 'Error del servidor';
  return datos;
}

const api = {
  // === AUTH ===
  login: async (email, password) => {
    try {
      const res = await fetch(`${API_BASE}/auth/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ email, password })
      });
      return await res.json();
    } catch (e) {
      return { error: 'No se pudo conectar con el sistema. Revisa tu internet.' };
    }
  },

  me: () => authFetch(`${API_BASE}/auth/me`),

  changePassword: (actual, nueva) => authFetch(`${API_BASE}/auth/change-password`, {
    method: 'PUT',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ actual, nueva })
  }),

  // === CLASES ===
  getClasses: () => authFetch(`${API_BASE}/classes`),

  createClass: (data) => authFetch(`${API_BASE}/classes`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  }),

  deleteClass: (id) => authFetch(`${API_BASE}/classes/${id}`, { method: 'DELETE' }),

  // === MATRÍCULAS / SOCIOS ===
  // La búsqueda y el filtro los resuelve la base de datos: el navegador ya no
  // se descarga la ficha de todos los socios para buscar un nombre.
  getMemberships: ({ q = '', estado = 'todos', limit = 300 } = {}) => {
    const p = new URLSearchParams();
    if (q) p.set('q', q);
    if (estado && estado !== 'todos') p.set('estado', estado);
    p.set('limit', String(limit));
    return authFetch(`${API_BASE}/memberships?${p.toString()}`);
  },

  getResumenSocios: () => authFetch(`${API_BASE}/memberships/resumen`),

  getPorVencer: (dias = 7) => authFetch(`${API_BASE}/memberships/por-vencer?dias=${dias}`),

  getFinanzas: () => authFetch(`${API_BASE}/memberships/finanzas`),

  // Libro de pagos individual (cada matrícula y renovación con su fecha real)
  getPagos: (limit) => authFetch(`${API_BASE}/memberships/pagos${limit ? '?limit=' + limit : ''}`),

  anularPago: (id, motivo) => authFetch(`${API_BASE}/memberships/pagos/${id}/anular`, {
    method: 'PUT',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ motivo })
  }),

  createMembership: (data) => authFetch(`${API_BASE}/memberships`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  }),

  updateMembership: (id, data) => authFetch(`${API_BASE}/memberships/${id}`, {
    method: 'PUT',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  }),

  renewMembership: (id, data) => authFetch(`${API_BASE}/memberships/${id}/renew`, {
    method: 'PUT',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  }),

  deleteMembership: (id) => authFetch(`${API_BASE}/memberships/${id}`, { method: 'DELETE' }),

  // === EXTRAS ===
  getExtras: (limit = 300) => authFetch(`${API_BASE}/extras?limit=${limit}`),

  getExtrasResumen: (desde) => authFetch(`${API_BASE}/extras/resumen?desde=${desde}`),

  createExtra: (data) => authFetch(`${API_BASE}/extras`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  }),

  deleteExtra: (id) => authFetch(`${API_BASE}/extras/${id}`, { method: 'DELETE' }),

  // === GASTOS (egresos del negocio) ===
  getGastos: (desde, hasta) => authFetch(`${API_BASE}/gastos?desde=${desde}&hasta=${hasta}`),

  getGastosResumen: (desde, hasta) => authFetch(`${API_BASE}/gastos/resumen?desde=${desde}&hasta=${hasta}`),

  createGasto: (data) => authFetch(`${API_BASE}/gastos`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  }),

  deleteGasto: (id) => authFetch(`${API_BASE}/gastos/${id}`, { method: 'DELETE' }),

  // === ASISTENCIA (control de entrada) ===
  getAsistencia: (fecha) => authFetch(`${API_BASE}/asistencia${fecha ? '?fecha=' + fecha : ''}`),

  getAsistenciaResumen: () => authFetch(`${API_BASE}/asistencia/resumen`),

  marcarEntrada: (membership_id) => authFetch(`${API_BASE}/asistencia`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ membership_id })
  }),

  borrarEntrada: (id) => authFetch(`${API_BASE}/asistencia/${id}`, { method: 'DELETE' }),

  // === HISTORIAL DE UN SOCIO ===
  getHistorialSocio: (id) => authFetch(`${API_BASE}/memberships/${id}/historial`),

  // === CATÁLOGO (precios, promos y categorías) ===
  getCatalogo: () => authFetch(`${API_BASE}/catalogo`),

  createCatalogoOption: (data) => authFetch(`${API_BASE}/catalogo`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  }),

  deleteCatalogoOption: (id) => authFetch(`${API_BASE}/catalogo/${id}`, { method: 'DELETE' }),

  // === RESPALDO ===
  descargarBackup: async () => {
    const res = await fetch(`${API_BASE}/backup`, {
      headers: { 'Authorization': `Bearer ${getToken()}` }
    });
    if (res.status === 401) {
      clearToken();
      window.location.href = 'login.html';
      throw new Error('Sesión expirada');
    }
    if (!res.ok) throw new Error('No se pudo generar el respaldo');
    return res.blob();
  },

  getBackupEstado: () => authFetch(`${API_BASE}/backup/estado`),

  restaurarBackup: (contenido) => authFetch(`${API_BASE}/backup/restaurar`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: contenido // ya es el texto JSON del archivo
  }),
};

// ===== Helpers compartidos =====
async function requireSession() {
  const token = getToken();
  if (!token) { window.location.href = 'login.html'; return null; }
  try {
    const user = await api.me();
    // Un fallo de red NO debe cerrar la sesión: si se borrara el token cada
    // vez que el wifi parpadea, habría que volver a escribir la contraseña.
    if (user && user.sinRed) {
      mostrarAvisoConexion('Sin conexión a internet. Reintentando...');
      return null;
    }
    if (!user || user.error || user.role !== 'admin') {
      clearToken();
      window.location.href = 'login.html';
      return null;
    }
    setupLogout();
    return user;
  } catch (err) {
    clearToken();
    window.location.href = 'login.html';
    return null;
  }
}

function setupLogout() {
  const btn = document.getElementById('btn-logout');
  if (btn) {
    btn.addEventListener('click', () => {
      if (confirm('¿Cerrar sesión?')) {
        clearToken();
        window.location.href = 'login.html';
      }
    });
  }
}

// Franja fija arriba para avisos que NO deben desaparecer solos (sin internet,
// base de datos reconectando). Antes estos fallos se tragaban en silencio y la
// pantalla mostraba "0 socios", como si el gimnasio estuviera vacío.
function mostrarAvisoConexion(mensaje) {
  let barra = document.getElementById('aviso-conexion');
  if (!barra) {
    barra = document.createElement('div');
    barra.id = 'aviso-conexion';
    barra.className = 'aviso-conexion';
    document.body.appendChild(barra);
  }
  barra.textContent = mensaje;
  barra.hidden = false;
}

function ocultarAvisoConexion() {
  const barra = document.getElementById('aviso-conexion');
  if (barra) barra.hidden = true;
}

// Devuelve true si la respuesta trae un error; además pinta el aviso adecuado.
function hayProblema(res) {
  if (!res) return true;
  if (res.sinRed) { mostrarAvisoConexion('Sin conexión a internet. Los datos mostrados pueden estar desactualizados.'); return true; }
  if (res.reintentable) { mostrarAvisoConexion('La base de datos está reconectando. Espera unos segundos...'); return true; }
  if (res.error) return true;
  ocultarAvisoConexion();
  return false;
}

// "Hoy" para el negocio.
//
// El servidor decide cuál es el día del gimnasio (hora de Lima) y el frontend
// lo usa en cuanto lo conoce. Si se usara solo el reloj del navegador, un
// celular con la zona horaria mal puesta —o alguien mirando el panel desde
// otro país— vería la asistencia y las finanzas de un día equivocado.
// Mientras el servidor no haya respondido, se usa el reloj local, que en el
// gym es el correcto.
let FECHA_NEGOCIO = null;

function fijarFechaNegocio(fecha) {
  if (typeof fecha === 'string' && /^\d{4}-\d{2}-\d{2}$/.test(fecha)) FECHA_NEGOCIO = fecha;
}

function hoyISO() {
  if (FECHA_NEGOCIO) return FECHA_NEGOCIO;
  const d = new Date();
  return d.getFullYear() + '-' +
    String(d.getMonth() + 1).padStart(2, '0') + '-' +
    String(d.getDate()).padStart(2, '0');
}

// Suma días a una fecha 'YYYY-MM-DD' sin sorpresas de zona horaria.
function addDiasISO(iso, n) {
  const [y, m, d] = iso.split('-').map(Number);
  const dt = new Date(Date.UTC(y, m - 1, d));
  dt.setUTCDate(dt.getUTCDate() + n);
  return dt.toISOString().slice(0, 10);
}

// Diferencia en días entre dos fechas 'YYYY-MM-DD'.
function diasEntre(a, b) {
  const p = (s) => { const [y, m, d] = s.split('-').map(Number); return Date.UTC(y, m - 1, d); };
  return Math.round((p(a) - p(b)) / 86400000);
}

// Escapa texto antes de insertarlo en HTML: un nombre o nota con símbolos
// (<, >, comillas) se muestra tal cual en vez de romper o inyectar la página.
function esc(v) {
  return String(v === null || v === undefined ? '' : v)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
}

function formatDate(d) {
  if (!d) return '—';
  const date = new Date(d + (d.length === 10 ? 'T00:00:00' : ''));
  if (isNaN(date.getTime())) return d;
  return date.toLocaleDateString('es-PE', { day: '2-digit', month: 'short', year: 'numeric' });
}

function formatPrice(n) {
  return Number(n || 0).toFixed(2).replace(/\B(?=(\d{3})+(?!\d))/g, ',');
}

// Importes de los reportes CON céntimos. Antes se mostraban redondeados con
// Math.round: el desglose no cuadraba con el total y faltaban soles sueltos.
function formatMoney(n) {
  return 'S/ ' + Number(n || 0).toLocaleString('es-PE', {
    minimumFractionDigits: 2,
    maximumFractionDigits: 2,
  });
}

function formatSize(bytes) {
  if (bytes < 1024) return bytes + ' B';
  if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
  return (bytes / 1024 / 1024).toFixed(2) + ' MB';
}

function tagClass(estado) {
  return {
    'activa': 'tag-green', 'activo': 'tag-green', 'confirmada': 'tag-green', 'completado': 'tag-green',
    'pendiente': 'tag-orange', 'mantenimiento': 'tag-orange',
    'vencida': 'tag-red', 'vencido': 'tag-red', 'cancelada': 'tag-red',
  }[estado] || 'tag-gray';
}

// El mensaje se inserta como TEXTO, nunca como HTML: si un nombre de socio
// llevara símbolos raros, se muestran tal cual en vez de alterar la página.
function showToast(msg, type = 'success') {
  const t = document.getElementById('toast');
  if (!t) return;
  t.className = 'toast toast--' + type;
  t.textContent = (type === 'success' ? '✓ ' : type === 'error' ? '✕ ' : '') + msg;
  t.hidden = false;
  requestAnimationFrame(() => t.classList.add('is-visible'));
  clearTimeout(showToast._t);
  showToast._t = setTimeout(() => {
    t.classList.remove('is-visible');
    setTimeout(() => t.hidden = true, 300);
  }, 3500);
}
