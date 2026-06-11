// ==========================================================
//  app.js — Cliente API para sistema interno ZONA VIP GYM
// ==========================================================

// API_BASE relativo - el backend sirve tanto el frontend como las APIs en el mismo puerto
const API_BASE = '/api';
const TOKEN_KEY = 'zvg_token';

function getToken() { return localStorage.getItem(TOKEN_KEY); }
function setToken(t) { localStorage.setItem(TOKEN_KEY, t); }
function clearToken() { localStorage.removeItem(TOKEN_KEY); }

// Todas las llamadas autenticadas pasan por aquí: si la sesión expiró
// (401), se limpia el token y se vuelve al login en vez de quedar
// la pantalla "muerta" mostrando errores.
async function authFetch(url, options = {}) {
  const res = await fetch(url, {
    ...options,
    headers: {
      ...(options.headers || {}),
      'Authorization': `Bearer ${getToken()}`,
    },
  });
  if (res.status === 401) {
    clearToken();
    window.location.href = 'login.html';
    return { error: 'Sesión expirada' };
  }
  try {
    return await res.json();
  } catch (e) {
    return { error: 'Respuesta inválida del servidor' };
  }
}

const api = {
  // === AUTH ===
  login: async (email, password) => {
    const res = await fetch(`${API_BASE}/auth/login`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ email, password })
    });
    return res.json();
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
  getMemberships: () => authFetch(`${API_BASE}/memberships`),

  getFinanzas: () => authFetch(`${API_BASE}/memberships/finanzas`),

  // Libro de pagos individual (cada matrícula y renovación con su fecha real)
  getPagos: () => authFetch(`${API_BASE}/memberships/pagos`),

  createMembership: (data) => authFetch(`${API_BASE}/memberships`, {
    method: 'POST',
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
  getExtras: () => authFetch(`${API_BASE}/extras`),

  createExtra: (data) => authFetch(`${API_BASE}/extras`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
  }),

  deleteExtra: (id) => authFetch(`${API_BASE}/extras/${id}`, { method: 'DELETE' }),

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
    if (!res.ok) throw new Error('No se pudo generar el respaldo');
    return res.blob();
  }
};

// ===== Helpers compartidos =====
async function requireSession() {
  const token = getToken();
  if (!token) { window.location.href = 'login.html'; return null; }
  try {
    const user = await api.me();
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

// "Hoy" según el reloj LOCAL del navegador (el del gym), no UTC.
// new Date().toISOString() devolvía la fecha de mañana después de las 7pm en Perú.
function hoyISO() {
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

function showToast(msg, type = 'success') {
  const t = document.getElementById('toast');
  if (!t) return;
  t.className = 'toast toast--' + type;
  t.innerHTML = (type === 'success' ? '✓ ' : type === 'error' ? '✕ ' : '') + msg;
  t.hidden = false;
  requestAnimationFrame(() => t.classList.add('is-visible'));
  setTimeout(() => {
    t.classList.remove('is-visible');
    setTimeout(() => t.hidden = true, 300);
  }, 3500);
}
