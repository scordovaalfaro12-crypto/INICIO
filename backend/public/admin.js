// ==========================================================
//  admin.js — Sistema interno ZONA VIP GYM
// ==========================================================

(async function() {
  const user = await requireSession();
  if (!user) return;
  initAdmin();
})();

function initAdmin() {
  setupAdminNav();
  cargarCatalogo();
  renderDashboard();
  renderClases();
  renderMatriculas();
  renderFinanzas();
  renderExtras();
}

// ============ CATÁLOGO (precios, promos y categorías) ============
// El admin define sus propias opciones en la pestaña "Precios y promos";
// aquí se cargan y se inyectan en los formularios de pago/renovación/venta.
const CATALOGO = { matricula: [], extra: [] };

async function cargarCatalogo() {
  try {
    const rows = await api.getCatalogo();
    if (Array.isArray(rows)) {
      CATALOGO.matricula = rows.filter(r => r.tipo === 'matricula');
      CATALOGO.extra = rows.filter(r => r.tipo === 'extra');
    }
  } catch (e) {}
  poblarSelectsCatalogo();
  renderPrecios();
}

function poblarSelectsCatalogo() {
  const selMat = document.getElementById('select-concepto-matricula');
  if (selMat) {
    const actual = selMat.value;
    selMat.innerHTML = '<option value="">Selecciona...</option>' +
      CATALOGO.matricula.map(o =>
        `<option value="${esc(o.nombre)}">${esc(o.nombre)} (S/ ${formatPrice(o.precio)} — ${o.dias} día${o.dias === 1 ? '' : 's'})</option>`
      ).join('') +
      '<option value="Otro">Otro</option>';
    if (actual) selMat.value = actual;
  }

  const selPlan = document.getElementById('select-plan-renovar');
  if (selPlan) {
    selPlan.innerHTML = '<option value="">Personalizado</option>' +
      CATALOGO.matricula.map(o =>
        `<option value="${o.id}">${esc(o.nombre)} — S/ ${formatPrice(o.precio)} (${o.dias}d)</option>`
      ).join('');
  }

  const selCat = document.getElementById('select-categoria-extra');
  if (selCat) {
    const actual = selCat.value;
    const tieneOtros = CATALOGO.extra.some(o => o.nombre === 'Otros');
    selCat.innerHTML = '<option value="">Selecciona...</option>' +
      CATALOGO.extra.map(o =>
        `<option value="${esc(o.nombre)}">${esc(o.nombre)}${(o.precio || o.precio === 0) ? ` (S/ ${formatPrice(o.precio)})` : ''}</option>`
      ).join('') +
      (tieneOtros ? '' : '<option value="Otros">Otros</option>');
    if (actual) selCat.value = actual;
  }
}

function renderPrecios() {
  const planesBody = document.getElementById('planes-precios-body');
  if (planesBody) {
    planesBody.innerHTML = CATALOGO.matricula.length === 0
      ? '<tr><td colspan="4" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin planes todavía. Agrega el primero arriba.</td></tr>'
      : CATALOGO.matricula.map(o => `
          <tr>
            <td><strong>${esc(o.nombre)}</strong></td>
            <td><strong>S/ ${formatPrice(o.precio)}</strong></td>
            <td>${o.dias} día${o.dias === 1 ? '' : 's'}</td>
            <td><button class="link-orange" style="color:var(--red);" data-del-opcion="${o.id}">Quitar</button></td>
          </tr>
        `).join('');
  }

  const catsBody = document.getElementById('categorias-precios-body');
  if (catsBody) {
    catsBody.innerHTML = CATALOGO.extra.length === 0
      ? '<tr><td colspan="3" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin categorías todavía. Agrega la primera arriba.</td></tr>'
      : CATALOGO.extra.map(o => `
          <tr>
            <td><strong>${esc(o.nombre)}</strong></td>
            <td>${(o.precio || o.precio === 0) ? 'S/ ' + formatPrice(o.precio) : '—'}</td>
            <td><button class="link-orange" style="color:var(--red);" data-del-opcion="${o.id}">Quitar</button></td>
          </tr>
        `).join('');
  }

  document.querySelectorAll('#tab-precios [data-del-opcion]').forEach(btn => {
    btn.addEventListener('click', async () => {
      if (!confirm('¿Quitar esta opción?\n\nLos pagos ya registrados con ella NO se borran ni cambian; solo deja de aparecer para pagos nuevos.')) return;
      const r = await api.deleteCatalogoOption(parseInt(btn.dataset.delOpcion));
      if (r.error) { showToast(r.error, 'error'); return; }
      showToast('Opción quitada');
      cargarCatalogo();
    });
  });
}

const formPlan = document.getElementById('form-nuevo-plan');
if (formPlan) {
  formPlan.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const res = await api.createCatalogoOption({
        tipo: 'matricula',
        nombre: fd.get('nombre'),
        precio: fd.get('precio'),
        dias: parseInt(fd.get('dias'))
      });
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast('Plan agregado · ya aparece al registrar pagos y renovar');
      e.target.reset();
      cargarCatalogo();
    });
  });
}

const formCategoria = document.getElementById('form-nueva-categoria');
if (formCategoria) {
  formCategoria.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const res = await api.createCatalogoOption({
        tipo: 'extra',
        nombre: fd.get('nombre'),
        precio: fd.get('precio') || null
      });
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast('Categoría agregada · ya aparece al registrar ventas');
      e.target.reset();
      cargarCatalogo();
    });
  });
}

function setupAdminNav() {
  document.querySelectorAll('.sidebar__nav a').forEach(link => {
    link.addEventListener('click', (e) => {
      e.preventDefault();
      switchTab(link.dataset.tab);
    });
  });
}

function switchTab(name) {
  document.querySelectorAll('.admin-section').forEach(s => s.hidden = (s.id !== `tab-${name}`));
  document.querySelectorAll('.sidebar__nav a').forEach(l => l.classList.toggle('is-active', l.dataset.tab === name));
}

// Evita doble registro por doble clic: deshabilita el botón mientras se envía.
async function conBotonBloqueado(form, fn) {
  const btn = form.querySelector('button[type=submit]');
  if (btn) btn.disabled = true;
  try { await fn(); } finally { if (btn) btn.disabled = false; }
}

// ============ DASHBOARD ============
async function renderDashboard() {
  let clases = [], matriculas = [], extras = [], finanzas = null;
  try {
    [clases, matriculas, extras, finanzas] = await Promise.all([
      api.getClasses(), api.getMemberships(), api.getExtras(), api.getFinanzas()
    ]);
  } catch (e) {}
  if (!Array.isArray(clases)) clases = [];
  if (!Array.isArray(matriculas)) matriculas = [];
  if (!Array.isArray(extras)) extras = [];

  const activos = matriculas.filter(m => m.estado === 'activa').length;
  const vencidos = matriculas.filter(m => m.estado === 'vencida').length;

  const el = (id) => document.getElementById(id);
  if (el('kpi-usuarios')) el('kpi-usuarios').textContent = activos;
  if (el('kpi-clases')) el('kpi-clases').textContent = clases.length;

  // KPI de "Reservas hoy" lo reemplazamos por "Vencidos"
  const reservasKpi = el('kpi-reservas');
  if (reservasKpi) {
    reservasKpi.textContent = vencidos;
    const parent = reservasKpi.closest('.stat-card');
    if (parent) {
      const lbl = parent.querySelector('.stat-card__label');
      if (lbl) lbl.textContent = 'Socios vencidos';
    }
  }

  // Ingresos del mes desde el reporte oficial (incluye renovaciones
  // en su fecha real, no el cálculo aproximado de antes).
  const totalMes = (finanzas && finanzas.mes && typeof finanzas.mes.total === 'number') ? finanzas.mes.total : 0;
  if (el('kpi-ingresos')) el('kpi-ingresos').textContent = 'S/ ' + Math.round(totalMes).toLocaleString('es-PE');

  // Dashboard mini-tabla: últimas matrículas (en lugar de reservas)
  const dashBody = el('dash-reservas-body');
  if (dashBody) {
    const recent = [...matriculas].sort((a, b) => b.id - a.id).slice(0, 6);
    if (recent.length === 0) {
      dashBody.innerHTML = '<tr><td colspan="8" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin matrículas registradas todavía</td></tr>';
    } else {
      dashBody.innerHTML = recent.map(m => `
        <tr>
          <td><strong>${m.id}</strong></td>
          <td>${esc(m.nombre)}</td>
          <td>${esc(m.concepto)}</td>
          <td>${esc(m.metodo)}</td>
          <td>${formatDate(m.fecha_pago)}</td>
          <td>—</td>
          <td><strong>S/ ${formatPrice(m.monto)}</strong></td>
          <td><span class="tag ${tagClass(m.estado)}">${esc(m.estado)}</span></td>
        </tr>
      `).join('');
    }
  }

  // Conceptos más comunes
  const conceptoCount = {};
  matriculas.forEach(m => { conceptoCount[m.concepto] = (conceptoCount[m.concepto] || 0) + 1; });
  const sortedConceptos = Object.entries(conceptoCount).sort((a, b) => b[1] - a[1]).slice(0, 5);
  const max = sortedConceptos[0]?.[1] || 1;
  const planesEl = el('planes-chart');
  if (planesEl) {
    planesEl.innerHTML = sortedConceptos.length === 0
      ? '<p style="color:var(--gray-mid);padding:1rem;">Sin datos aún</p>'
      : sortedConceptos.map(([c, cnt]) => `
          <div style="margin-bottom:1rem;">
            <div style="display:flex;justify-content:space-between;font-size:0.85rem;margin-bottom:0.3rem;">
              <strong>${esc(c)}</strong>
              <span style="color:var(--gray-mid);">${cnt} socio(s)</span>
            </div>
            <div style="height:10px;background:var(--gray-soft);border-radius:6px;overflow:hidden;">
              <div style="width:${(cnt/max)*100}%;height:100%;background:var(--orange);"></div>
            </div>
          </div>
        `).join('');
  }

  // Categorías de extras más vendidas este mes
  const inicioMes = hoyISO().slice(0, 8) + '01';
  const extrasCats = {};
  extras.filter(e => e.fecha >= inicioMes).forEach(e => {
    extrasCats[e.categoria] = (extrasCats[e.categoria] || 0) + e.monto;
  });
  const sortedExtras = Object.entries(extrasCats).sort((a, b) => b[1] - a[1]).slice(0, 5);
  const maxE = sortedExtras[0]?.[1] || 1;
  const clasesEl = el('clases-chart');
  if (clasesEl) {
    clasesEl.innerHTML = sortedExtras.length === 0
      ? '<p style="color:var(--gray-mid);padding:1rem;">Sin ventas extras este mes</p>'
      : sortedExtras.map(([cat, total]) => `
          <div style="margin-bottom:1rem;">
            <div style="display:flex;justify-content:space-between;font-size:0.85rem;margin-bottom:0.3rem;">
              <strong>${esc(cat)}</strong>
              <span style="color:var(--gray-mid);">S/ ${Math.round(total).toLocaleString('es-PE')}</span>
            </div>
            <div style="height:10px;background:var(--gray-soft);border-radius:6px;overflow:hidden;">
              <div style="width:${(total/maxE)*100}%;height:100%;background:var(--gray-dark);"></div>
            </div>
          </div>
        `).join('');
  }
}

// ============ CLASES ============
async function renderClases() {
  let clases = [];
  try { clases = await api.getClasses(); } catch (e) {}
  if (!Array.isArray(clases)) clases = [];
  const body = document.getElementById('clases-body');
  if (!body) return;
  body.innerHTML = clases.length === 0
    ? '<tr><td colspan="8" style="text-align:center;color:var(--gray-mid);padding:2rem;">No hay clases. Crea la primera.</td></tr>'
    : clases.map(c => `
        <tr>
          <td><strong>${c.id}</strong></td>
          <td><strong>${esc(c.titulo)}</strong></td>
          <td>${esc(c.instructor)}</td>
          <td>${formatDate(c.fecha)}</td>
          <td>${esc(c.hora)}</td>
          <td>${esc(c.capacidad)}</td>
          <td><strong>S/ ${formatPrice(c.precio)}</strong></td>
          <td><button class="link-orange" style="color:var(--red);" data-del-clase="${c.id}">Eliminar</button></td>
        </tr>
      `).join('');

  body.querySelectorAll('[data-del-clase]').forEach(btn => {
    btn.addEventListener('click', async () => {
      if (!confirm('¿Eliminar esta clase?')) return;
      const r = await api.deleteClass(parseInt(btn.dataset.delClase));
      if (r.error) { showToast(r.error, 'error'); return; }
      showToast('Clase eliminada');
      renderClases(); renderDashboard();
    });
  });
}

// ============ MATRÍCULAS ============
async function renderMatriculas(filtroTexto = '', filtroEstado = 'todos') {
  let matriculas = [];
  try { matriculas = await api.getMemberships(); } catch (e) {}
  if (!Array.isArray(matriculas)) matriculas = [];

  const hoy = hoyISO();
  const en7Str = addDiasISO(hoy, 7);

  const activos = matriculas.filter(m => m.estado === 'activa');
  const vencidos = matriculas.filter(m => m.estado === 'vencida');
  const prontos = activos.filter(m => m.fecha_vence <= en7Str);
  const setVal = (id, val) => { const e = document.getElementById(id); if (e) e.textContent = val; };
  setVal('mat-activos', activos.length);
  setVal('mat-vencidos', vencidos.length);
  setVal('mat-pronto', prontos.length);

  let filtered = matriculas;
  if (filtroTexto) {
    const q = filtroTexto.toLowerCase();
    filtered = filtered.filter(m =>
      m.nombre.toLowerCase().includes(q) ||
      (m.dni || '').includes(q) ||
      (m.telefono || '').includes(q)
    );
  }
  if (filtroEstado === 'activa') filtered = filtered.filter(m => m.estado === 'activa');
  else if (filtroEstado === 'vencida') filtered = filtered.filter(m => m.estado === 'vencida');
  else if (filtroEstado === 'pronto') filtered = filtered.filter(m => m.estado === 'activa' && m.fecha_vence <= en7Str);

  const body = document.getElementById('matriculas-body');
  if (!body) return;
  if (filtered.length === 0) {
    body.innerHTML = '<tr><td colspan="9" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin matrículas. Registra el primer pago.</td></tr>';
    return;
  }

  body.innerHTML = filtered.map(m => {
    const proximo = m.estado === 'activa' && m.fecha_vence <= en7Str;
    const estadoColor = m.estado === 'activa' ? (proximo ? 'tag-orange' : 'tag-green') : 'tag-red';
    const estadoTxt = m.estado === 'activa' ? (proximo ? 'Vence pronto' : 'Activa') : 'Vencida';
    const diasRestantes = Math.round((new Date(m.fecha_vence + 'T00:00:00Z') - new Date(hoy + 'T00:00:00Z')) / 86400000);
    const diasTxt = m.estado === 'activa'
      ? (diasRestantes <= 0 ? '<small style="color:var(--red);">hoy</small>' : `<small style="color:var(--gray-mid);">en ${diasRestantes}d</small>`)
      : `<small style="color:var(--red);">hace ${Math.abs(diasRestantes)}d</small>`;
    return `
      <tr>
        <td><strong>${esc(m.nombre)}</strong>${m.dni ? `<br><small style="color:var(--gray-mid);">DNI ${esc(m.dni)}</small>` : ''}</td>
        <td>${esc(m.telefono) || '—'}</td>
        <td>${esc(m.concepto)}</td>
        <td><strong>S/ ${formatPrice(m.monto)}</strong></td>
        <td><span class="tag">${esc(m.metodo)}</span></td>
        <td>${formatDate(m.fecha_pago)}</td>
        <td>${formatDate(m.fecha_vence)} ${diasTxt}</td>
        <td><span class="tag ${estadoColor}">${estadoTxt}</span></td>
        <td>
          <button class="link-orange" data-action="renovar" data-id="${m.id}" data-nombre="${esc(m.nombre)}">Renovar</button> ·
          <button class="link-orange" style="color:var(--red);" data-action="eliminar-matricula" data-id="${m.id}">Borrar</button>
        </td>
      </tr>
    `;
  }).join('');

  body.querySelectorAll('[data-action]').forEach(btn => {
    btn.addEventListener('click', function() {
      const action = this.dataset.action;
      const id = parseInt(this.dataset.id);
      if (action === 'renovar') abrirRenovar(id, this.dataset.nombre);
      else if (action === 'eliminar-matricula') eliminarMatricula(id);
    });
  });
}

const searchMat = document.getElementById('search-matriculas');
const filterEst = document.getElementById('filter-estado');
if (searchMat) searchMat.addEventListener('input', e => renderMatriculas(e.target.value, filterEst.value));
if (filterEst) filterEst.addEventListener('change', e => renderMatriculas(searchMat.value, e.target.value));

function openModalNuevaMatricula() {
  const m = document.getElementById('modal-matricula');
  if (!m) return;
  const hoy = hoyISO();
  m.querySelector('[name=fecha_pago]').value = hoy;
  m.querySelector('[name=fecha_vence]').value = addDiasISO(hoy, 30);
  m.hidden = false;
}

function autocompletarMonto(concepto) {
  const form = document.getElementById('form-nueva-matricula');
  if (!form) return;
  const op = CATALOGO.matricula.find(o => o.nombre === concepto);
  if (!op) return; // "Otro": el admin escribe monto y vencimiento a mano
  if (op.precio !== null && op.precio !== undefined) {
    form.querySelector('[name=monto]').value = op.precio;
  }
  if (op.dias) {
    // La duración se cuenta desde la fecha de pago elegida (sirve para registrar con fecha pasada)
    const base = form.querySelector('[name=fecha_pago]').value || hoyISO();
    form.querySelector('[name=fecha_vence]').value = addDiasISO(base, op.dias);
  }
}

const formMat = document.getElementById('form-nueva-matricula');
if (formMat) {
  formMat.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const data = Object.fromEntries(fd);
      const res = await api.createMembership(data);
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast(`Pago de ${data.nombre} registrado · S/ ${data.monto}`);
      closeModal('modal-matricula');
      e.target.reset();
      renderMatriculas(); renderFinanzas(); renderDashboard();
    });
  });
}

function abrirRenovar(id, nombre) {
  if (!id || !nombre) { showToast('Faltan datos del socio', 'error'); return; }
  document.getElementById('renovar-nombre').textContent = nombre;
  const form = document.getElementById('form-renovar');
  form.querySelector('[name=id]').value = id;
  form.querySelector('[name=dias]').value = '30';
  form.querySelector('[name=monto]').value = '80';
  const sel = document.getElementById('select-plan-renovar');
  if (sel) sel.value = '';
  document.getElementById('modal-renovar').hidden = false;
}

// Al elegir un plan del catálogo se llenan días y monto (siempre editables).
function aplicarPlanRenovacion(idOpcion) {
  const form = document.getElementById('form-renovar');
  if (!form) return;
  const op = CATALOGO.matricula.find(o => String(o.id) === String(idOpcion));
  if (!op) return; // "Personalizado": se respetan los valores escritos
  form.querySelector('[name=dias]').value = op.dias;
  form.querySelector('[name=monto]').value = op.precio;
}

// Si la categoría tiene precio sugerido, se autocompleta el monto de la venta.
function autocompletarPrecioExtra(categoria) {
  const form = document.getElementById('form-nuevo-extra');
  if (!form) return;
  const op = CATALOGO.extra.find(o => o.nombre === categoria);
  if (op && op.precio !== null && op.precio !== undefined) {
    form.querySelector('[name=monto]').value = op.precio;
  }
}

const formRen = document.getElementById('form-renovar');
if (formRen) {
  formRen.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const id = fd.get('id');
      const res = await api.renewMembership(id, {
        dias: parseInt(fd.get('dias')),
        monto: parseFloat(fd.get('monto')),
        metodo: fd.get('metodo')
      });
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast('Membresía renovada hasta ' + res.nuevoVence);
      closeModal('modal-renovar');
      renderMatriculas(); renderFinanzas(); renderDashboard();
    });
  });
}

async function eliminarMatricula(id) {
  if (!confirm('¿Borrar la ficha de este socio?\n\nSus pagos ya registrados NO se borran: quedan guardados en el historial de ingresos.')) return;
  const res = await api.deleteMembership(id);
  if (res.error) { showToast(res.error, 'error'); return; }
  showToast('Ficha eliminada (los pagos quedan en el historial)');
  renderMatriculas(); renderFinanzas(); renderDashboard();
}

// ============ FINANZAS ============
async function renderFinanzas() {
  let f = null;
  try { f = await api.getFinanzas(); } catch (e) {}
  if (!f || !f.mes) return;

  const setVal = (id, valor, cnt) => {
    const el = document.getElementById(id);
    if (el) el.textContent = 'S/ ' + Math.round(valor).toLocaleString('es-PE');
    if (cnt !== undefined) {
      const c = document.getElementById(id + '-cnt');
      if (c) c.textContent = `${cnt} venta${cnt === 1 ? '' : 's'}`;
    }
  };
  setVal('fin-hoy', f.hoy.total, f.hoy.cantidad);
  setVal('fin-semana', f.semana.total, f.semana.cantidad);
  setVal('fin-quincena', f.quincena.total, f.quincena.cantidad);
  setVal('fin-mes', f.mes.total, f.mes.cantidad);
  setVal('fin-anio', f.anio.total, f.anio.cantidad);

  const mat = f.mes.matriculas.total;
  const extras = (f.mes.extras && f.mes.extras.total) || 0;
  const total = mat + extras || 1;

  const setText = (id, val) => { const e = document.getElementById(id); if (e) e.textContent = val; };
  const setStyle = (id, prop, val) => { const e = document.getElementById(id); if (e) e.style[prop] = val; };
  setText('desg-matriculas', 'S/ ' + Math.round(mat).toLocaleString('es-PE'));
  setText('desg-online', 'S/ ' + Math.round(extras).toLocaleString('es-PE'));
  setStyle('desg-mat-bar', 'width', ((mat / total) * 100) + '%');
  setStyle('desg-on-bar', 'width', ((extras / total) * 100) + '%');
  setText('desg-mat-cnt', `${f.mes.matriculas.cantidad} pago(s) registrados`);
  setText('desg-on-cnt', `${(f.mes.extras && f.mes.extras.cantidad) || 0} venta(s) extras`);
  setText('desg-total', 'S/ ' + Math.round(mat + extras).toLocaleString('es-PE'));

  // Renombro etiquetas (era "Reservas online")
  document.querySelectorAll('#tab-finanzas strong').forEach(s => {
    if (s.textContent === 'Reservas online (Yape web)') s.textContent = 'Ventas extras (productos)';
  });

  // Métodos
  const metEl = document.getElementById('metodos-pago');
  if (metEl) {
    if (!f.metodos || f.metodos.length === 0) {
      metEl.innerHTML = '<p style="color:var(--gray-mid);font-size:0.85rem;">Aún no hay pagos este mes.</p>';
    } else {
      const totalMet = f.metodos.reduce((s, m) => s + m.total, 0) || 1;
      metEl.innerHTML = f.metodos.map(m => {
        const pct = (m.total / totalMet) * 100;
        return `
          <div style="margin-bottom:1rem;">
            <div style="display:flex;justify-content:space-between;font-size:0.85rem;margin-bottom:0.3rem;">
              <strong>${esc(m.metodo)}</strong>
              <span class="text-orange">S/ ${Math.round(m.total).toLocaleString('es-PE')} <small style="color:var(--gray-mid);">(${pct.toFixed(0)}%)</small></span>
            </div>
            <div style="height:8px;background:var(--gray-soft);border-radius:4px;overflow:hidden;">
              <div style="width:${pct}%;height:100%;background:var(--orange);"></div>
            </div>
          </div>
        `;
      }).join('');
    }
  }

  // Proyecciones
  const sociosAct = (f.socios && f.socios.activos) || 0;
  const proyBuena = (sociosAct + ((f.socios && f.socios.vencidos) || 0)) * 80;
  const proyActual = sociosAct * 80;
  const diaActual = new Date().getDate();
  const promedioDiario = f.mes.total / Math.max(diaActual, 1);
  setText('proj-buena', 'S/ ' + Math.round(proyBuena).toLocaleString('es-PE'));
  setText('proj-actual', 'S/ ' + Math.round(proyActual).toLocaleString('es-PE'));
  setText('proj-diario', 'S/ ' + Math.round(promedioDiario).toLocaleString('es-PE'));
}

// ============ EXTRAS ============
async function renderExtras() {
  let extras = [];
  try { extras = await api.getExtras(); } catch (e) {}
  if (!Array.isArray(extras)) extras = [];

  const inicioMes = hoyISO().slice(0, 8) + '01';
  const extrasMes = extras.filter(e => e.fecha >= inicioMes);

  const setVal = (id, v) => { const e = document.getElementById(id); if (e) e.textContent = v; };
  const totalMes = extrasMes.reduce((s, e) => s + e.monto, 0);
  setVal('extras-total-mes', 'S/ ' + Math.round(totalMes).toLocaleString('es-PE'));
  setVal('extras-count-mes', `${extrasMes.length} ventas`);

  // Tarjetas dinámicas: las 3 categorías que más vendieron este mes,
  // sean cuales sean las que el admin haya creado.
  const porCat = {};
  extrasMes.forEach(e => { porCat[e.categoria] = (porCat[e.categoria] || 0) + e.monto; });
  const topEl = document.getElementById('extras-top-cats');
  if (topEl) {
    const top = Object.entries(porCat).sort((a, b) => b[1] - a[1]).slice(0, 3);
    topEl.innerHTML = top.length === 0
      ? `<div class="extra-cat-card" style="background:var(--gray-soft);padding:1rem;border-radius:var(--r-sm);">
           <div style="font-size:0.75rem;letter-spacing:0.1em;text-transform:uppercase;color:var(--gray-mid);">Sin ventas este mes</div>
           <div style="font-family:var(--font-display);font-size:1.5rem;">S/ 0</div>
         </div>`
      : top.map(([cat, total]) => `
          <div class="extra-cat-card" style="background:var(--gray-soft);padding:1rem;border-radius:var(--r-sm);">
            <div style="font-size:0.75rem;letter-spacing:0.1em;text-transform:uppercase;color:var(--gray-mid);">${esc(cat)}</div>
            <div style="font-family:var(--font-display);font-size:1.5rem;">S/ ${Math.round(total).toLocaleString('es-PE')}</div>
          </div>
        `).join('');
  }

  const recent = extras.slice(0, 20);
  const body = document.getElementById('extras-body');
  if (!body) return;
  if (recent.length === 0) {
    body.innerHTML = '<tr><td colspan="6" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin ventas extras todavía.</td></tr>';
    return;
  }
  body.innerHTML = recent.map(e => `
    <tr>
      <td>${formatDate(e.fecha)}</td>
      <td><strong>${esc(e.categoria)}</strong></td>
      <td>${esc(e.descripcion) || '—'}</td>
      <td><strong>S/ ${formatPrice(e.monto)}</strong></td>
      <td><span class="tag">${esc(e.metodo)}</span></td>
      <td><button class="link-orange" style="color:var(--red);" data-extra-del="${e.id}">Borrar</button></td>
    </tr>
  `).join('');

  body.querySelectorAll('[data-extra-del]').forEach(btn => {
    btn.addEventListener('click', async () => {
      if (!confirm('¿Borrar este ingreso? Esta venta dejará de contar en las finanzas.')) return;
      const r = await api.deleteExtra(parseInt(btn.dataset.extraDel));
      if (r.error) { showToast(r.error, 'error'); return; }
      showToast('Borrado');
      renderExtras(); renderFinanzas(); renderDashboard();
    });
  });
}

function openModalExtra() {
  const m = document.getElementById('modal-extra');
  if (!m) return;
  m.querySelector('[name=fecha]').value = hoyISO();
  m.hidden = false;
}

const formExtra = document.getElementById('form-nuevo-extra');
if (formExtra) {
  formExtra.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const data = Object.fromEntries(fd);
      const res = await api.createExtra(data);
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast(`Ingreso de S/ ${data.monto} registrado · ${data.categoria}`);
      closeModal('modal-extra');
      e.target.reset();
      renderExtras(); renderFinanzas(); renderDashboard();
    });
  });
}


// ============ MODALES ============
function openModalNuevaClase() {
  const m = document.getElementById('modal-clase-new');
  if (!m) return;
  m.querySelector('[name=fecha]').value = addDiasISO(hoyISO(), 1);
  m.hidden = false;
}

function closeModal(id) {
  const m = document.getElementById(id);
  if (!m) return;
  m.hidden = true;
  const f = m.querySelector('form');
  if (f) f.reset();
}

const formClase = document.getElementById('form-nueva-clase');
if (formClase) {
  formClase.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const data = {
        titulo: fd.get('titulo').trim(),
        instructor: fd.get('instructor').trim(),
        fecha: fd.get('fecha'),
        hora: fd.get('hora'),
        descripcion: fd.get('descripcion').trim(),
        imagen: fd.get('imagen').trim() || '',
        capacidad: parseInt(fd.get('capacidad')) || 20,
        precio: parseFloat(fd.get('precio')) || 15
      };
      const res = await api.createClass(data);
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast(`Clase ${data.titulo} creada`);
      closeModal('modal-clase-new');
      renderClases(); renderDashboard();
    });
  });
}

// ============ Botones Finanzas (Actualizar + Exportar CSV + Respaldo) ============

// Celda CSV segura: comillas escapadas y sin riesgo de fórmulas en Excel.
function csvCell(v) {
  let s = String(v === null || v === undefined ? '' : v);
  if (/^[=+\-@]/.test(s)) s = "'" + s;
  return '"' + s.replace(/"/g, '""') + '"';
}

document.addEventListener('DOMContentLoaded', () => {
  const btnAct = document.getElementById('btn-actualizar-fin');
  if (btnAct) {
    btnAct.addEventListener('click', async () => {
      btnAct.disabled = true;
      const o = btnAct.innerHTML;
      btnAct.innerHTML = '<span class="spinner"></span> Actualizando...';
      await renderFinanzas(); await renderMatriculas(); await renderDashboard(); await renderExtras();
      btnAct.innerHTML = o;
      btnAct.disabled = false;
      showToast('Finanzas actualizadas ✓');
    });
  }

  const btnExp = document.getElementById('btn-exportar-fin');
  if (btnExp) {
    btnExp.addEventListener('click', async () => {
      try {
        // El CSV sale del libro de pagos: cada matrícula Y cada renovación
        // con su fecha real, más las ventas extras.
        const [pagos, extras] = await Promise.all([api.getPagos(), api.getExtras()]);
        const filas = [['Tipo', 'Fecha', 'Cliente/Concepto', 'Detalle', 'Método', 'Monto'].map(csvCell).join(',')];
        if (Array.isArray(pagos)) {
          pagos.forEach(p => {
            const tipo = p.tipo === 'renovacion' ? 'Renovación' : 'Matrícula';
            filas.push([tipo, p.fecha_pago, p.nombre, p.concepto || '', p.metodo, p.monto].map(csvCell).join(','));
          });
        }
        if (Array.isArray(extras)) {
          extras.forEach(e => {
            filas.push(['Venta extra', e.fecha, e.categoria, e.descripcion || '', e.metodo, e.monto].map(csvCell).join(','));
          });
        }
        const blob = new Blob(['\uFEFF' + filas.join('\n')], { type: 'text/csv;charset=utf-8;' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `finanzas-zonavip-${hoyISO()}.csv`;
        document.body.appendChild(a); a.click(); document.body.removeChild(a);
        URL.revokeObjectURL(url);
        showToast('Reporte descargado ✓');
      } catch (e) {
        showToast('Error al exportar', 'error');
      }
    });
  }

  const btnBak = document.getElementById('btn-respaldo');
  if (btnBak) {
    btnBak.addEventListener('click', async () => {
      btnBak.disabled = true;
      try {
        const blob = await api.descargarBackup();
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = `respaldo-zonavip-${hoyISO()}.json`;
        document.body.appendChild(a); a.click(); document.body.removeChild(a);
        URL.revokeObjectURL(url);
        showToast('Respaldo descargado ✓ Guárdalo en un lugar seguro');
      } catch (e) {
        showToast('Error al generar respaldo', 'error');
      } finally {
        btnBak.disabled = false;
      }
    });
  }
});
