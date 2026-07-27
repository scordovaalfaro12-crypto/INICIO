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
  enlazarAcciones();
  cargarCatalogo();
  renderDashboard();
  renderClases();
  renderMatriculas();
  renderFinanzas();
  renderExtras();
  renderPorVencer();
  renderAsistencia();
  renderGastos();
}

// El HTML ya no lleva onclick="..." escritos dentro (así el navegador puede
// bloquear cualquier script ajeno). Aquí se conectan esos botones por su
// atributo data-*, una sola vez al arrancar.
function enlazarAcciones() {
  document.querySelectorAll('[data-abrir]').forEach(b =>
    b.addEventListener('click', () => abrirModal(b.dataset.abrir)));
  document.querySelectorAll('[data-cerrar]').forEach(b =>
    b.addEventListener('click', () => closeModal(b.dataset.cerrar)));
  document.querySelectorAll('[data-ir-a]').forEach(b =>
    b.addEventListener('click', () => switchTab(b.dataset.irA)));

  const autos = {
    'monto-matricula': (v) => autocompletarMonto(v),
    'precio-extra': (v) => autocompletarPrecioExtra(v),
    'plan-renovacion': (v) => aplicarPlanRenovacion(v),
  };
  document.querySelectorAll('[data-auto]').forEach(sel =>
    sel.addEventListener('change', () => {
      const fn = autos[sel.dataset.auto];
      if (fn) fn(sel.value);
    }));

  // Cerrar el modal con la tecla Escape o pulsando fuera de la tarjeta.
  document.addEventListener('keydown', (e) => {
    if (e.key !== 'Escape') return;
    document.querySelectorAll('.modal:not([hidden])').forEach(m => closeModal(m.id));
  });
  document.querySelectorAll('.modal').forEach(m =>
    m.addEventListener('click', (e) => { if (e.target === m) closeModal(m.id); }));

  // Buscador y filtros de la lista de socios.
  const buscador = document.getElementById('search-matriculas');
  if (buscador) buscador.addEventListener('input', buscarConEspera);
  const filtro = document.getElementById('filter-estado');
  if (filtro) filtro.addEventListener('change', buscarConEspera);
  const diasVencer = document.getElementById('select-dias-vencer');
  if (diasVencer) diasVencer.addEventListener('change', renderPorVencer);

  // Asistencia: buscador del mostrador y selector de día.
  const buscarAsis = document.getElementById('buscar-socio-asistencia');
  if (buscarAsis) buscarAsis.addEventListener('input', buscarSocioParaEntrada);
  const fechaAsis = document.getElementById('fecha-asistencia');
  if (fechaAsis) fechaAsis.addEventListener('change', () => renderAsistencia(fechaAsis.value));
}

// Pone el cursor en el primer campo del modal: en el celular se abre el
// teclado al instante y se puede escribir sin tener que tocar la pantalla.
function enfocarPrimerCampo(idModal) {
  const m = document.getElementById(idModal);
  if (!m) return;
  const campo = m.querySelector('input:not([type=hidden]):not([readonly]), select, textarea');
  if (campo) setTimeout(() => { try { campo.focus(); } catch (e) {} }, 120);
}

// Prepara y muestra el modal indicado (antes cada botón llamaba a su función).
function abrirModal(id) {
  if (id === 'modal-gasto') {
    const m = document.getElementById('modal-gasto');
    if (m) { m.querySelector('[name=fecha]').value = hoyISO(); m.hidden = false; enfocarPrimerCampo(id); }
    return;
  }
  if (id === 'modal-matricula') return openModalNuevaMatricula();
  if (id === 'modal-extra') return openModalExtra();
  if (id === 'modal-clase-new') return openModalNuevaClase();
  const m = document.getElementById(id);
  if (m) { m.hidden = false; enfocarPrimerCampo(id); }
}

// ============ CATÁLOGO (precios, promos y categorías) ============
// El admin define sus propias opciones en la pestaña "Precios y promos";
// aquí se cargan y se inyectan en los formularios de pago/renovación/venta.
const CATALOGO = { matricula: [], extra: [], gasto: [] };

async function cargarCatalogo() {
  const rows = await api.getCatalogo();
  if (Array.isArray(rows)) {
    CATALOGO.matricula = rows.filter(r => r.tipo === 'matricula');
    CATALOGO.extra = rows.filter(r => r.tipo === 'extra');
    CATALOGO.gasto = rows.filter(r => r.tipo === 'gasto');
    ocultarAvisoConexion();
  } else if (rows && rows.error) {
    // Sin catálogo los formularios quedan sin opciones: hay que avisar, no
    // dejar a la dueña mirando un desplegable vacío sin explicación.
    showToast('No se pudieron cargar los precios: ' + rows.error, 'error');
  }
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
    const actual = selPlan.value;
    selPlan.innerHTML = '<option value="">Personalizado</option>' +
      CATALOGO.matricula.map(o =>
        `<option value="${o.id}" data-nombre="${esc(o.nombre)}">${esc(o.nombre)} — S/ ${formatPrice(o.precio)} (${o.dias}d)</option>`
      ).join('');
    if (actual) selPlan.value = actual;
  }

  const selGasto = document.getElementById('select-categoria-gasto');
  if (selGasto) {
    const actual = selGasto.value;
    selGasto.innerHTML = '<option value="">Selecciona...</option>' +
      CATALOGO.gasto.map(o => `<option value="${esc(o.nombre)}">${esc(o.nombre)}</option>`).join('') +
      '<option value="Otros">Otros</option>';
    if (actual) selGasto.value = actual;
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

  const rubrosBody = document.getElementById('rubros-precios-body');
  if (rubrosBody) {
    rubrosBody.innerHTML = CATALOGO.gasto.length === 0
      ? '<tr><td colspan="2" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin rubros todavía. Agrega el primero arriba.</td></tr>'
      : CATALOGO.gasto.map(o => `
          <tr>
            <td><strong>${esc(o.nombre)}</strong></td>
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

const formRubro = document.getElementById('form-nuevo-rubro');
if (formRubro) {
  formRubro.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const res = await api.createCatalogoOption({ tipo: 'gasto', nombre: fd.get('nombre') });
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast('Rubro agregado · ya aparece al registrar gastos');
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
  try {
    await fn();
  } catch (e) {
    // Sin este catch un fallo inesperado no mostraba NADA: el modal quedaba
    // abierto y la dueña volvía a pulsar, cobrando dos veces al mismo socio.
    showToast('No se pudo confirmar el guardado. Revisa la lista antes de reintentar para no cobrar dos veces.', 'error');
  } finally {
    if (btn) btn.disabled = false;
  }
}

// ============ DASHBOARD ============
async function renderDashboard() {
  // Antes los errores se tragaban con `catch (e) {}` y la pantalla mostraba
  // "0 socios · S/ 0", como si el gimnasio estuviera vacío. Ahora un fallo se
  // ve como fallo y los datos guardados no se ponen en duda.
  const [clases, resumen, finanzas, ultimosPagos] = await Promise.all([
    api.getClasses(), api.getResumenSocios(), api.getFinanzas(), api.getPagos(6),
  ]);

  const el = (id) => document.getElementById(id);
  const falla = hayProblema(resumen) || hayProblema(finanzas);
  if (falla) {
    const msg = (resumen && resumen.error) || (finanzas && finanzas.error) || 'Sin conexión con el sistema';
    const cuerpo = el('dash-reservas-body');
    if (cuerpo) {
      cuerpo.innerHTML = `<tr><td colspan="8" style="text-align:center;color:var(--red);padding:2rem;">
        ${esc(msg)}<br><small style="color:var(--gray-mid);">Tus datos siguen guardados. Reintenta en unos segundos.</small></td></tr>`;
    }
    return;
  }

  if (el('kpi-usuarios')) el('kpi-usuarios').textContent = resumen.activos;
  if (el('kpi-clases')) el('kpi-clases').textContent = Array.isArray(clases) ? clases.length : 0;

  const reservasKpi = el('kpi-reservas');
  if (reservasKpi) {
    reservasKpi.textContent = resumen.vencidos;
    const parent = reservasKpi.closest('.stat-card');
    if (parent) {
      const lbl = parent.querySelector('.stat-card__label');
      if (lbl) lbl.textContent = 'Socios vencidos';
    }
  }

  const totalMes = (finanzas.mes && typeof finanzas.mes.total === 'number') ? finanzas.mes.total : 0;
  if (el('kpi-ingresos')) el('kpi-ingresos').textContent = formatMoney(totalMes);

  // Los ÚLTIMOS PAGOS de verdad. Antes esta tabla decía "Últimos pagos
  // registrados" pero traía socios ordenados por fecha de vencimiento: el
  // primero de la lista podía haber pagado hacía medio año.
  const dashBody = el('dash-reservas-body');
  if (dashBody) {
    const recent = Array.isArray(ultimosPagos) ? ultimosPagos.slice(0, 6) : [];
    if (recent.length === 0) {
      dashBody.innerHTML = '<tr><td colspan="6" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin pagos registrados todavía</td></tr>';
    } else {
      dashBody.innerHTML = recent.map(p => `
        <tr>
          <td><strong>${esc(p.nombre)}</strong></td>
          <td data-col="Fecha">${formatDate(p.fecha_pago)}</td>
          <td data-col="Tipo">${p.tipo === 'renovacion' ? 'Renovación' : 'Matrícula'}</td>
          <td data-col="Concepto">${esc(p.concepto)}</td>
          <td data-col="Método">${esc(p.metodo)}</td>
          <td data-col="Monto"><strong>S/ ${formatPrice(p.monto)}</strong></td>
        </tr>
      `).join('');
    }
  }

  // Reparto de socios por estado (con los contadores reales de la base).
  const planesEl = el('planes-chart');
  if (planesEl) {
    const filas = [
      ['Socios activos', resumen.activos, 'var(--green)'],
      ['Vencen en 7 días', resumen.pronto, 'var(--orange)'],
      ['Vencidos', resumen.vencidos, 'var(--red)'],
    ];
    const max = Math.max(1, ...filas.map(f => f[1]));
    planesEl.innerHTML = resumen.total === 0
      ? '<p style="color:var(--gray-mid);padding:1rem;">Sin socios todavía</p>'
      : filas.map(([etiqueta, valor, color]) => `
          <div style="margin-bottom:1rem;">
            <div style="display:flex;justify-content:space-between;font-size:0.85rem;margin-bottom:0.3rem;">
              <strong>${etiqueta}</strong>
              <span style="color:var(--gray-mid);">${valor} socio(s)</span>
            </div>
            <div style="height:10px;background:var(--gray-soft);border-radius:6px;overflow:hidden;">
              <div style="width:${(valor / max) * 100}%;height:100%;background:${color};"></div>
            </div>
          </div>
        `).join('');
  }

  // Productos más vendidos del mes: lo suma la base de datos, no el navegador.
  const clasesEl = el('clases-chart');
  if (clasesEl) {
    const resExtras = await api.getExtrasResumen(hoyISO().slice(0, 8) + '01');
    const cats = (resExtras && Array.isArray(resExtras.categorias)) ? resExtras.categorias.slice(0, 5) : [];
    const maxE = Math.max(1, ...cats.map(c => c.total));
    clasesEl.innerHTML = cats.length === 0
      ? '<p style="color:var(--gray-mid);padding:1rem;">Sin ventas extras este mes</p>'
      : cats.map(c => `
          <div style="margin-bottom:1rem;">
            <div style="display:flex;justify-content:space-between;font-size:0.85rem;margin-bottom:0.3rem;">
              <strong>${esc(c.categoria)}</strong>
              <span style="color:var(--gray-mid);">${formatMoney(c.total)}</span>
            </div>
            <div style="height:10px;background:var(--gray-soft);border-radius:6px;overflow:hidden;">
              <div style="width:${(c.total / maxE) * 100}%;height:100%;background:var(--gray-dark);"></div>
            </div>
          </div>
        `).join('');
  }
}

// ============ CLASES ============
async function renderClases() {
  const clases = await api.getClasses();
  const body = document.getElementById('clases-body');
  if (!body) return;
  if (!Array.isArray(clases)) {
    body.innerHTML = `<tr><td colspan="8" style="text-align:center;color:var(--red);padding:2rem;">
      ${esc((clases && clases.error) || 'No se pudieron cargar las clases')}</td></tr>`;
    return;
  }
  body.innerHTML = clases.length === 0
    ? '<tr><td colspan="8" style="text-align:center;color:var(--gray-mid);padding:2rem;">No hay clases. Crea la primera.</td></tr>'
    : clases.map(c => `
        <tr>
          <td><strong>${esc(c.titulo)}</strong></td>
          <td data-col="N.º">${c.id}</td>
          <td data-col="Instructor">${esc(c.instructor)}</td>
          <td data-col="Fecha">${formatDate(c.fecha)}</td>
          <td data-col="Hora">${esc(c.hora)}</td>
          <td data-col="Cupos">${esc(c.capacidad)}</td>
          <td data-col="Precio"><strong>S/ ${formatPrice(c.precio)}</strong></td>
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
// La búsqueda y el filtro los resuelve el servidor. Antes cada tecla del
// buscador se descargaba la ficha de TODOS los socios (medido: 379 KB y 13
// segundos con 1.500 socios) y las respuestas llegaban desordenadas, así que
// la tabla podía terminar mostrando el resultado de una búsqueda anterior.
const ESTADO_LISTA = { texto: '', estado: 'todos', peticion: 0 };
let temporizadorBusqueda = null;

async function renderMatriculas(filtroTexto, filtroEstado) {
  if (filtroTexto !== undefined) ESTADO_LISTA.texto = filtroTexto;
  if (filtroEstado !== undefined) ESTADO_LISTA.estado = filtroEstado;

  const body = document.getElementById('matriculas-body');
  const conteo = document.getElementById('matriculas-conteo');
  if (!body) return;

  // Cada petición lleva número: si llega una respuesta vieja, se descarta.
  const miNumero = ++ESTADO_LISTA.peticion;

  const [resumen, lista] = await Promise.all([
    api.getResumenSocios(),
    api.getMemberships({ q: ESTADO_LISTA.texto, estado: ESTADO_LISTA.estado }),
  ]);
  if (miNumero !== ESTADO_LISTA.peticion) return; // llegó tarde: ya hay otra búsqueda

  const setVal = (id, val) => { const e = document.getElementById(id); if (e) e.textContent = val; };
  if (!hayProblema(resumen)) {
    setVal('mat-activos', resumen.activos);
    setVal('mat-vencidos', resumen.vencidos);
    setVal('mat-pronto', resumen.pronto);
  }

  if (hayProblema(lista)) {
    body.innerHTML = `<tr><td colspan="9" style="text-align:center;color:var(--red);padding:2rem;">
      ${esc(lista && lista.error ? lista.error : 'No se pudo cargar la lista de socios.')}<br>
      <small style="color:var(--gray-mid);">Los datos siguen guardados. Vuelve a intentar en unos segundos.</small></td></tr>`;
    if (conteo) conteo.textContent = '';
    return;
  }

  const filtered = Array.isArray(lista.items) ? lista.items : [];
  const hoy = hoyISO();
  const en7Str = addDiasISO(hoy, 7);

  if (conteo) {
    conteo.textContent = filtered.length < lista.total
      ? `Mostrando ${filtered.length} de ${lista.total} socios. Afina la búsqueda para ver los demás.`
      : (lista.total === 0 ? '' : `${lista.total} socio${lista.total === 1 ? '' : 's'}`);
  }

  if (filtered.length === 0) {
    body.innerHTML = `<tr><td colspan="9" style="text-align:center;color:var(--gray-mid);padding:2rem;">${
      ESTADO_LISTA.texto || ESTADO_LISTA.estado !== 'todos'
        ? 'Ningún socio coincide con la búsqueda.'
        : 'Sin matrículas. Registra el primer pago.'}</td></tr>`;
    return;
  }

  body.innerHTML = filtered.map(m => {
    const proximo = m.estado === 'activa' && m.fecha_vence <= en7Str;
    const estadoColor = m.estado === 'activa' ? (proximo ? 'tag-orange' : 'tag-green') : 'tag-red';
    const estadoTxt = m.estado === 'activa' ? (proximo ? 'Vence pronto' : 'Activa') : 'Vencida';
    const diasRestantes = diasEntre(m.fecha_vence, hoy);
    const diasTxt = m.estado === 'activa'
      ? (diasRestantes <= 0 ? '<small style="color:var(--red);">hoy</small>' : `<small style="color:var(--gray-mid);">en ${diasRestantes}d</small>`)
      : `<small style="color:var(--red);">hace ${Math.abs(diasRestantes)}d</small>`;
    return `
      <tr>
        <td><strong>${esc(m.nombre)}</strong>${m.dni ? `<br><small style="color:var(--gray-mid);">DNI ${esc(m.dni)}</small>` : ''}</td>
        <td data-col="Teléfono">${esc(m.telefono) || '—'}</td>
        <td data-col="Concepto">${esc(m.concepto)}</td>
        <td data-col="Monto"><strong>S/ ${formatPrice(m.monto)}</strong></td>
        <td data-col="Método"><span class="tag">${esc(m.metodo)}</span></td>
        <td data-col="Pagó">${formatDate(m.fecha_pago)}</td>
        <td data-col="Vence">${formatDate(m.fecha_vence)} ${diasTxt}</td>
        <td data-col="Estado"><span class="tag ${estadoColor}">${estadoTxt}</span></td>
        <td class="acciones-socio">
          <button class="link-orange" data-action="renovar" data-id="${m.id}">Renovar</button>
          <button class="link-orange" data-action="historial" data-id="${m.id}">Historial</button>
          <button class="link-orange" data-action="editar" data-id="${m.id}">Corregir</button>
          <button class="link-orange" style="color:var(--red);" data-action="eliminar-matricula" data-id="${m.id}">Borrar</button>
        </td>
      </tr>
    `;
  }).join('');

  // Los datos del socio se guardan aquí, no en atributos del HTML.
  const porId = new Map(filtered.map(m => [String(m.id), m]));
  body.querySelectorAll('[data-action]').forEach(btn => {
    btn.addEventListener('click', function () {
      const socio = porId.get(this.dataset.id);
      const id = parseInt(this.dataset.id, 10);
      if (this.dataset.action === 'renovar') abrirRenovar(id, socio);
      else if (this.dataset.action === 'historial') abrirHistorial(id);
      else if (this.dataset.action === 'editar') abrirEditarSocio(socio);
      else if (this.dataset.action === 'eliminar-matricula') eliminarMatricula(id);
    });
  });
}

// Espera a que deje de escribir: una búsqueda por palabra, no una por letra.
function buscarConEspera() {
  clearTimeout(temporizadorBusqueda);
  temporizadorBusqueda = setTimeout(() => {
    const s = document.getElementById('search-matriculas');
    const f = document.getElementById('filter-estado');
    renderMatriculas(s ? s.value : '', f ? f.value : 'todos');
  }, 300);
}

// ============ AVISOS A LOS QUE VENCEN PRONTO ============
// El enlace de WhatsApp se arma en el navegador: no hay servicios externos
// contratados que puedan caerse o caducar.
async function renderPorVencer() {
  const panel = document.getElementById('panel-por-vencer');
  const lista = document.getElementById('lista-por-vencer');
  const sel = document.getElementById('select-dias-vencer');
  if (!panel || !lista) return;

  const dias = sel ? parseInt(sel.value, 10) || 7 : 7;
  const rows = await api.getPorVencer(dias);
  if (!Array.isArray(rows) || rows.length === 0) {
    panel.hidden = true;
    return;
  }
  panel.hidden = false;
  const hoy = hoyISO();
  lista.innerHTML = rows.map(m => {
    const faltan = diasEntre(m.fecha_vence, hoy);
    const cuando = faltan <= 0 ? 'vence HOY' : `vence en ${faltan} día${faltan === 1 ? '' : 's'}`;
    const tel = telefonoWhatsApp(m.telefono);
    const mensaje = `Hola ${m.nombre}, te saluda ZONA VIP GYM 💪 Tu ${m.concepto} ${faltan <= 0 ? 'vence hoy' : `vence el ${m.fecha_vence}`}. ¿Te esperamos para renovar?`;
    return `
      <div class="aviso-socio">
        <div>
          <strong>${esc(m.nombre)}</strong>
          <small style="color:${faltan <= 1 ? 'var(--red)' : 'var(--gray-mid)'};"> · ${cuando}</small>
          ${tel ? '' : '<small style="color:var(--gray-mid);"> · sin teléfono guardado</small>'}
        </div>
        <div style="display:flex;gap:0.4rem;flex-wrap:wrap;">
          ${tel ? `<a class="btn btn-outline btn-sm" target="_blank" rel="noopener"
               href="https://wa.me/${tel}?text=${encodeURIComponent(mensaje)}">WhatsApp</a>` : ''}
          <button class="btn btn-primary btn-sm" data-renovar-id="${m.id}">Cobrar renovación</button>
        </div>
      </div>`;
  }).join('');

  // Se puede cobrar sin salir del aviso: antes había que ir a Socios,
  // buscar a la persona y recién ahí pulsar Renovar.
  lista.querySelectorAll('[data-renovar-id]').forEach(btn =>
    btn.addEventListener('click', () => abrirRenovarPorId(parseInt(btn.dataset.renovarId, 10))));
}

// Celular peruano listo para el enlace de WhatsApp (o null si no sirve).
function telefonoWhatsApp(v) {
  const solo = String(v || '').replace(/\D/g, '');
  if (!solo) return null;
  if (solo.length === 9 && solo.startsWith('9')) return '51' + solo;
  if (solo.length === 11 && solo.startsWith('51')) return solo;
  if (solo.length >= 8 && solo.length <= 15) return solo;
  return null;
}

function openModalNuevaMatricula() {
  const m = document.getElementById('modal-matricula');
  if (!m) return;
  const hoy = hoyISO();
  m.querySelector('[name=fecha_pago]').value = hoy;
  m.querySelector('[name=fecha_vence]').value = addDiasISO(hoy, 30);
  m.hidden = false;
  enfocarPrimerCampo('modal-matricula');
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
      let res = await api.createMembership(data);

      // La persona ya es socia: se ofrece renovarle en vez de crear una
      // segunda ficha (que partiría su historial y duplicaría el conteo).
      if (res.socio_existente) {
        const s = res.socio_existente;
        const renovar = confirm(
          `${s.nombre} ya está registrado.\n\n` +
          `Su ${s.concepto} ${s.estado === 'activa' ? 'vence el ' + s.fecha_vence : 'está VENCIDA (' + s.fecha_vence + ')'}.\n\n` +
          'Aceptar = renovarle (lo recomendado)\n' +
          'Cancelar = crear una ficha aparte de todos modos'
        );
        closeModal('modal-matricula');
        if (renovar) {
          const lista = await api.getMemberships({ q: s.nombre, limit: 5 });
          const socio = (lista.items || []).find(m => m.id === s.id);
          if (socio) { abrirRenovar(s.id, socio); return; }
          showToast('Busca al socio en la lista y pulsa Renovar', 'error');
          return;
        }
        res = await api.createMembership({ ...data, confirmar_duplicado: true });
        if (res.error) { showToast(res.error, 'error'); return; }
        showToast('Ficha aparte creada');
        renderMatriculas(); renderFinanzas(); renderDashboard(); renderPorVencer();
        return;
      }

      if (res.duplicado) {
        // El servidor detectó un pago idéntico hace segundos: se pregunta
        // antes de duplicarlo, en vez de cobrar dos veces en silencio.
        if (!confirm(res.error + '\n\n¿Registrarlo igualmente?')) return;
        res = await api.createMembership({ ...data, confirmar_duplicado: true });
      }
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast(`Pago de ${data.nombre} registrado · S/ ${data.monto}`);
      closeModal('modal-matricula');
      e.target.reset();
      renderMatriculas(); renderFinanzas(); renderDashboard(); renderPorVencer();
    });
  });
}

function abrirRenovar(id, socio) {
  if (!id || !socio) { showToast('Faltan datos del socio', 'error'); return; }
  document.getElementById('renovar-nombre').textContent = socio.nombre;
  const form = document.getElementById('form-renovar');
  form.querySelector('[name=id]').value = id;
  form.querySelector('[name=fecha_pago]').value = hoyISO();

  // Se propone el MISMO plan que ya tenía el socio, con su precio y duración
  // del catálogo actual. Antes siempre proponía 30 días y S/ 80 a fuego, así
  // que renovar a un socio de aeróbicos cobraba de más si nadie lo corregía.
  const sel = document.getElementById('select-plan-renovar');
  const suPlan = CATALOGO.matricula.find(o => o.nombre === socio.concepto);
  if (suPlan) {
    if (sel) sel.value = String(suPlan.id);
    form.querySelector('[name=dias]').value = suPlan.dias;
    form.querySelector('[name=monto]').value = suPlan.precio;
  } else {
    if (sel) sel.value = '';
    form.querySelector('[name=dias]').value = '30';
    form.querySelector('[name=monto]').value = socio.monto != null ? socio.monto : '';
  }
  if (socio.metodo) {
    const selMet = form.querySelector('[name=metodo]');
    if (selMet) selMet.value = socio.metodo;
  }
  document.getElementById('modal-renovar').hidden = false;
}

// Abre la renovación teniendo solo el id (desde los avisos y la asistencia).
async function abrirRenovarPorId(id) {
  const h = await api.getHistorialSocio(id);
  if (hayProblema(h) || !h.socio) { showToast('No se pudo cargar el socio', 'error'); return; }
  abrirRenovar(id, h.socio);
}

// ============ CORREGIR DATOS DEL SOCIO ============
function abrirEditarSocio(socio) {
  if (!socio) { showToast('Faltan datos del socio', 'error'); return; }
  const form = document.getElementById('form-editar-socio');
  if (!form) return;
  form.querySelector('[name=id]').value = socio.id;
  form.querySelector('[name=nombre]').value = socio.nombre || '';
  form.querySelector('[name=telefono]').value = socio.telefono || '';
  form.querySelector('[name=dni]').value = socio.dni || '';
  form.querySelector('[name=concepto]').value = socio.concepto || '';
  form.querySelector('[name=fecha_vence]').value = socio.fecha_vence || '';
  form.querySelector('[name=notas]').value = socio.notas || '';
  document.getElementById('modal-editar-socio').hidden = false;
}

const formEditarSocio = document.getElementById('form-editar-socio');
if (formEditarSocio) {
  formEditarSocio.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const res = await api.updateMembership(fd.get('id'), Object.fromEntries(fd));
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast('Datos corregidos ✓');
      closeModal('modal-editar-socio');
      renderMatriculas(); renderPorVencer(); renderDashboard();
    });
  });
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
        dias: fd.get('dias'),
        monto: fd.get('monto'),
        metodo: fd.get('metodo'),
        fecha_pago: fd.get('fecha_pago') || hoyISO(),
        concepto: document.getElementById('select-plan-renovar') && document.getElementById('select-plan-renovar').selectedOptions[0]
          ? document.getElementById('select-plan-renovar').selectedOptions[0].dataset.nombre || ''
          : ''
      });
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast('Membresía renovada hasta ' + res.nuevoVence);
      closeModal('modal-renovar');
      renderMatriculas(); renderFinanzas(); renderDashboard(); renderPorVencer(); renderAsistencia();
    });
  });
}

async function eliminarMatricula(id) {
  if (!confirm('¿Borrar la ficha de este socio?\n\nSus pagos ya registrados NO se borran: quedan guardados en el historial de ingresos.')) return;
  const res = await api.deleteMembership(id);
  if (res.error) { showToast(res.error, 'error'); return; }
  showToast('Ficha eliminada (los pagos quedan en el historial)');
  renderMatriculas(); renderFinanzas(); renderDashboard(); renderPorVencer();
}

// ============ FINANZAS ============
async function renderFinanzas() {
  const f = await api.getFinanzas();
  if (hayProblema(f) || !f.mes) {
    if (f && f.error) showToast(f.error, 'error');
    return;
  }

  // Los importes se muestran CON céntimos: antes se redondeaban con Math.round
  // y el desglose no cuadraba con el total (faltaban soles sueltos).
  const setVal = (id, valor, cnt) => {
    const el = document.getElementById(id);
    if (el) el.textContent = formatMoney(valor);
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
  setText('desg-matriculas', formatMoney(mat));
  setText('desg-online', formatMoney(extras));
  setStyle('desg-mat-bar', 'width', ((mat / total) * 100) + '%');
  setStyle('desg-on-bar', 'width', ((extras / total) * 100) + '%');
  setText('desg-mat-cnt', `${f.mes.matriculas.cantidad} pago(s) registrados`);
  setText('desg-on-cnt', `${(f.mes.extras && f.mes.extras.cantidad) || 0} venta(s) extras`);
  setText('desg-total', formatMoney(mat + extras));

  // Renombro etiquetas (era "Reservas online")
  document.querySelectorAll('#tab-finanzas strong').forEach(s => {
    if (s.textContent === 'Reservas online (Yape web)') s.textContent = 'Ventas extras (productos)';
  });

  // Aviso de dinero registrado con fecha futura (casi siempre un dedazo en el
  // año). No se pierde, pero deja de descuadrar los totales y aquí se ve.
  mostrarAvisoFuturos(f.futuros);

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
              <span class="text-orange">${formatMoney(m.total)} <small style="color:var(--gray-mid);">(${pct.toFixed(0)}%)</small></span>
            </div>
            <div style="height:8px;background:var(--gray-soft);border-radius:4px;overflow:hidden;">
              <div style="width:${pct}%;height:100%;background:var(--orange);"></div>
            </div>
          </div>
        `;
      }).join('');
    }
  }

  // Proyecciones con el precio REAL del plan mensual del catálogo. Antes se
  // multiplicaba por 80 a fuego: subir los precios del gym no cambiaba nada.
  const precio = f.precio_referencia || 0;
  const sociosAct = (f.socios && f.socios.activos) || 0;
  const sociosVenc = (f.socios && f.socios.vencidos) || 0;
  setText('proj-buena', formatMoney((sociosAct + sociosVenc) * precio));
  setText('proj-actual', formatMoney(sociosAct * precio));
  const diaActual = new Date().getDate();
  setText('proj-diario', formatMoney(f.mes.total / Math.max(diaActual, 1)));

  renderPagosRecientes();
  renderEstadoRespaldo();
}

function mostrarAvisoFuturos(futuros) {
  const cont = document.getElementById('fin-hoy');
  const panel = cont ? cont.closest('.cards-row') : null;
  let aviso = document.getElementById('aviso-futuros');
  if (!futuros || !futuros.cantidad) { if (aviso) aviso.remove(); return; }
  if (!aviso && panel && panel.parentNode) {
    aviso = document.createElement('div');
    aviso.id = 'aviso-futuros';
    aviso.className = 'aviso-inline';
    panel.parentNode.insertBefore(aviso, panel.nextSibling);
  }
  if (aviso) {
    aviso.textContent = `⚠ Hay ${futuros.cantidad} pago(s) por ${formatMoney(futuros.total)} con fecha futura. ` +
      'Suelen ser un error de tecleo en el año: búscalos abajo y anúlalos para que las cuentas cuadren.';
  }
}

// Últimos pagos, con la opción de anular el que se registró mal.
async function renderPagosRecientes() {
  const body = document.getElementById('pagos-recientes-body');
  if (!body) return;
  const pagos = await api.getPagos(25);
  if (!Array.isArray(pagos)) {
    body.innerHTML = '<tr><td colspan="6" style="text-align:center;color:var(--gray-mid);padding:1.5rem;">No se pudieron cargar los pagos.</td></tr>';
    return;
  }
  const recientes = pagos;
  if (recientes.length === 0) {
    body.innerHTML = '<tr><td colspan="6" style="text-align:center;color:var(--gray-mid);padding:1.5rem;">Todavía no hay pagos registrados.</td></tr>';
    return;
  }
  const hoy = hoyISO();
  body.innerHTML = recientes.map(p => `
    <tr${p.fecha_pago > hoy ? ' style="background:rgba(255,107,26,0.07);"' : ''}>
      <td><strong>${esc(p.nombre)}</strong> <small style="color:var(--gray-mid);">${esc(p.concepto)}</small></td>
      <td data-col="Fecha">${formatDate(p.fecha_pago)}${p.fecha_pago > hoy ? ' <small style="color:var(--orange);">(futura)</small>' : ''}</td>
      <td data-col="Tipo">${p.tipo === 'renovacion' ? 'Renovación' : 'Matrícula'}</td>
      <td data-col="Método"><span class="tag">${esc(p.metodo)}</span></td>
      <td data-col="Monto"><strong>S/ ${formatPrice(p.monto)}</strong></td>
      <td><button class="link-orange" style="color:var(--red);" data-anular="${p.id}">Anular</button></td>
    </tr>
  `).join('');

  body.querySelectorAll('[data-anular]').forEach(btn => {
    btn.addEventListener('click', async () => {
      const motivo = prompt('¿Por qué se anula este pago?\n(Queda guardado como nota, el registro no se borra)', 'Monto equivocado');
      if (motivo === null) return;
      btn.disabled = true;
      const r = await api.anularPago(parseInt(btn.dataset.anular, 10), motivo);
      if (r.error) { showToast(r.error, 'error'); btn.disabled = false; return; }
      showToast('Pago anulado ✓ Ya no cuenta en las finanzas');
      renderFinanzas(); renderDashboard();
    });
  });
}

// ============ RESPALDO Y RESTAURACIÓN ============
async function renderEstadoRespaldo() {
  const el = document.getElementById('backup-estado');
  if (!el) return;
  const e = await api.getBackupEstado();
  if (!e || e.error) { el.textContent = ''; return; }
  el.textContent = `Ahora mismo hay ${e.socios} socio(s), ${e.pagos} pago(s), ${e.ventas} venta(s) extra y ${e.opciones} precio(s) guardados.`;
}

// ============ EXTRAS ============
async function renderExtras() {
  const inicioMes = hoyISO().slice(0, 8) + '01';
  const [resumen, extras] = await Promise.all([
    api.getExtrasResumen(inicioMes),
    api.getExtras(300),
  ]);

  const setVal = (id, v) => { const e = document.getElementById(id); if (e) e.textContent = v; };
  if (!hayProblema(resumen)) {
    setVal('extras-total-mes', formatMoney(resumen.total));
    setVal('extras-count-mes', `${resumen.cantidad} venta${resumen.cantidad === 1 ? '' : 's'}`);
  }

  // Las 3 categorías que más vendieron este mes, sumadas por la base de datos.
  const topEl = document.getElementById('extras-top-cats');
  if (topEl && !hayProblema(resumen)) {
    const top = (resumen.categorias || []).slice(0, 3);
    topEl.innerHTML = top.length === 0
      ? `<div class="extra-cat-card" style="background:var(--gray-soft);padding:1rem;border-radius:var(--r-sm);">
           <div style="font-size:0.75rem;letter-spacing:0.1em;text-transform:uppercase;color:var(--gray-mid);">Sin ventas este mes</div>
           <div style="font-family:var(--font-display);font-size:1.5rem;">S/ 0.00</div>
         </div>`
      : top.map(c => `
          <div class="extra-cat-card" style="background:var(--gray-soft);padding:1rem;border-radius:var(--r-sm);">
            <div style="font-size:0.75rem;letter-spacing:0.1em;text-transform:uppercase;color:var(--gray-mid);">${esc(c.categoria)}</div>
            <div style="font-family:var(--font-display);font-size:1.5rem;">${formatMoney(c.total)}</div>
          </div>
        `).join('');
  }

  const body = document.getElementById('extras-body');
  if (!body) return;
  if (!Array.isArray(extras)) {
    body.innerHTML = `<tr><td colspan="6" style="text-align:center;color:var(--red);padding:2rem;">
      ${esc((extras && extras.error) || 'No se pudieron cargar las ventas')}</td></tr>`;
    return;
  }
  const recent = extras.slice(0, 50);
  if (recent.length === 0) {
    body.innerHTML = '<tr><td colspan="6" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin ventas extras todavía.</td></tr>';
    return;
  }
  body.innerHTML = recent.map(e => `
    <tr>
      <td><strong>${esc(e.categoria)}</strong></td>
      <td data-col="Fecha">${formatDate(e.fecha)}</td>
      <td data-col="Detalle">${esc(e.descripcion) || '—'}</td>
      <td data-col="Monto"><strong>S/ ${formatPrice(e.monto)}</strong></td>
      <td data-col="Método"><span class="tag">${esc(e.metodo)}</span></td>
      <td><button class="link-orange" style="color:var(--red);" data-extra-del="${e.id}">Anular</button></td>
    </tr>
  `).join('');

  body.querySelectorAll('[data-extra-del]').forEach(btn => {
    btn.addEventListener('click', async () => {
      if (!confirm('¿Anular esta venta?\n\nDejará de contar en las finanzas, pero el registro queda guardado en el historial.')) return;
      btn.disabled = true;
      const r = await api.deleteExtra(parseInt(btn.dataset.extraDel, 10));
      if (r.error) { showToast(r.error, 'error'); btn.disabled = false; return; }
      showToast('Venta anulada');
      renderExtras(); renderFinanzas(); renderDashboard(); renderGastos();
    });
  });
}

function openModalExtra() {
  const m = document.getElementById('modal-extra');
  if (!m) return;
  m.querySelector('[name=fecha]').value = hoyISO();
  m.hidden = false;
  enfocarPrimerCampo('modal-extra');
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



// ============ ASISTENCIA (control de entrada) ============
let temporizadorSocioAsis = null;
let peticionSocioAsis = 0;

async function renderAsistencia(fecha) {
  const inputFecha = document.getElementById('fecha-asistencia');
  // Sin fecha explícita se deja decidir al servidor: él sabe qué día es en el
  // gimnasio (hora de Lima) aunque el celular tenga otra zona horaria.
  const dia = fecha || (inputFecha && inputFecha.value) || '';

  const [datos, resumen] = await Promise.all([
    api.getAsistencia(dia),
    api.getAsistenciaResumen(),
  ]);

  if (datos && datos.fecha) {
    fijarFechaNegocio(datos.fecha);
    if (inputFecha && !inputFecha.value) inputFecha.value = datos.fecha;
  }

  const setVal = (id, v) => { const e = document.getElementById(id); if (e) e.textContent = v; };
  if (!hayProblema(resumen)) {
    setVal('asis-hoy', resumen.hoy);
    setVal('asis-semana', resumen.semana);
    setVal('asis-socios', resumen.socios_semana);
    renderAusentes(resumen.ausentes || []);
  }

  const body = document.getElementById('asistencia-body');
  if (!body) return;
  if (hayProblema(datos) || !Array.isArray(datos.items)) {
    body.innerHTML = `<tr><td colspan="5" style="text-align:center;color:var(--red);padding:2rem;">
      ${esc((datos && datos.error) || 'No se pudo cargar la asistencia')}</td></tr>`;
    return;
  }
  if (datos.items.length === 0) {
    body.innerHTML = '<tr><td colspan="5" style="text-align:center;color:var(--gray-mid);padding:2rem;">Nadie ha marcado entrada este día.</td></tr>';
    return;
  }
  body.innerHTML = datos.items.map(a => `
    <tr>
      <td>${esc(a.nombre)}</td>
      <td data-col="Hora"><strong>${esc(a.hora)}</strong></td>
      <td data-col="Membresía">${esc(a.concepto) || '—'}${a.fecha_vence ? `<br><small style="color:var(--gray-mid);">vence ${formatDate(a.fecha_vence)}</small>` : ''}</td>
      <td data-col="Al entrar"><span class="tag ${a.estado_al_entrar === 'activa' ? 'tag-green' : 'tag-red'}">${a.estado_al_entrar === 'activa' ? 'Al día' : 'Vencida'}</span></td>
      <td><button class="link-orange" style="color:var(--red);" data-borrar-entrada="${a.id}">Quitar</button></td>
    </tr>
  `).join('');

  body.querySelectorAll('[data-borrar-entrada]').forEach(btn => {
    btn.addEventListener('click', async () => {
      if (!confirm('¿Quitar esta entrada del registro?')) return;
      btn.disabled = true;
      const r = await api.borrarEntrada(parseInt(btn.dataset.borrarEntrada, 10));
      if (r.error) { showToast(r.error, 'error'); btn.disabled = false; return; }
      showToast('Entrada quitada');
      renderAsistencia();
    });
  });
}

function renderAusentes(ausentes) {
  const panel = document.getElementById('panel-ausentes');
  const lista = document.getElementById('lista-ausentes');
  if (!panel || !lista) return;
  if (!ausentes.length) { panel.hidden = true; return; }
  panel.hidden = false;
  lista.innerHTML = ausentes.map(a => {
    const tel = telefonoWhatsApp(a.telefono);
    const cuando = a.ultima_visita ? `última vez el ${formatDate(a.ultima_visita)}` : 'nunca ha marcado entrada';
    const mensaje = `Hola ${a.nombre}, te saluda ZONA VIP GYM 💪 Hace tiempo que no te vemos. Tu ${a.concepto} sigue activa hasta el ${a.fecha_vence}. ¡Te esperamos!`;
    return `
      <div class="aviso-socio">
        <div><strong>${esc(a.nombre)}</strong> <small style="color:var(--gray-mid);">· ${cuando}</small></div>
        ${tel ? `<a class="btn btn-outline btn-sm" target="_blank" rel="noopener"
             href="https://wa.me/${tel}?text=${encodeURIComponent(mensaje)}">Escribirle</a>` : ''}
      </div>`;
  }).join('');
}

// Buscador del mostrador: se escribe el nombre y se marca la entrada de un clic.
function buscarSocioParaEntrada() {
  clearTimeout(temporizadorSocioAsis);
  temporizadorSocioAsis = setTimeout(async () => {
    const input = document.getElementById('buscar-socio-asistencia');
    const cont = document.getElementById('resultados-asistencia');
    if (!input || !cont) return;
    const q = input.value.trim();
    if (q.length < 2) { cont.innerHTML = ''; return; }

    // Igual que en la lista de socios: si llega una respuesta de una búsqueda
    // anterior, se descarta en vez de pintar resultados que ya no corresponden.
    const miNumero = ++peticionSocioAsis;
    const lista = await api.getMemberships({ q, limit: 8 });
    if (miNumero !== peticionSocioAsis) return;
    if (hayProblema(lista) || !Array.isArray(lista.items)) { cont.innerHTML = ''; return; }
    if (lista.items.length === 0) {
      cont.innerHTML = '<p style="color:var(--gray-mid);font-size:0.88rem;margin:0;">Ningún socio con ese nombre o DNI.</p>';
      return;
    }
    cont.innerHTML = lista.items.map(m => `
      <div class="aviso-socio">
        <div>
          <strong>${esc(m.nombre)}</strong>
          <small style="color:${m.estado === 'activa' ? 'var(--gray-mid)' : 'var(--red)'};">
            · ${esc(m.concepto)} · ${m.estado === 'activa' ? 'vence ' + formatDate(m.fecha_vence) : 'VENCIDA'}
          </small>
        </div>
        <button class="btn btn-primary btn-sm" data-marcar="${m.id}">Marcar entrada</button>
      </div>`).join('');

    cont.querySelectorAll('[data-marcar]').forEach(btn => {
      btn.addEventListener('click', async () => {
        btn.disabled = true;
        const r = await api.marcarEntrada(parseInt(btn.dataset.marcar, 10));
        if (r.error) { showToast(r.error, 'error'); btn.disabled = false; return; }
        mostrarAvisoEntrada(r);
        input.value = '';
        cont.innerHTML = '';
        renderAsistencia();
      });
    });
  }, 300);
}

// Cartel grande con la situación del socio que acaba de entrar. Es lo que la
// dueña mira en el mostrador para decidir si le cobra la renovación.
function mostrarAvisoEntrada(r) {
  const el = document.getElementById('aviso-entrada');
  if (!el) return;
  const vencida = r.socio.estado !== 'activa';
  const porVencer = !vencida && r.dias_restantes <= 3;
  el.innerHTML = `<div class="entrada-aviso ${vencida ? 'es-vencida' : porVencer ? 'es-pronto' : 'es-ok'}">
      <span>${esc(r.mensaje)}${r.ya_marcado ? ' <small>(ya había marcado hoy)</small>' : ''}</span>
      ${(vencida || porVencer) ? `<button class="btn btn-primary btn-sm" data-cobrar="${r.socio.id}">Cobrar renovación</button>` : ''}
    </div>`;
  // El momento de cobrar es justo cuando la persona está delante del mostrador:
  // el botón lleva a la renovación sin tener que ir a buscarla a otra pestaña.
  const btnCobrar = el.querySelector('[data-cobrar]');
  if (btnCobrar) btnCobrar.addEventListener('click', () => abrirRenovarPorId(parseInt(btnCobrar.dataset.cobrar, 10)));
  clearTimeout(mostrarAvisoEntrada._t);
  mostrarAvisoEntrada._t = setTimeout(() => { el.innerHTML = ''; }, 12000);
}

// ============ GASTOS (egresos) ============
async function renderGastos() {
  const inicioMes = hoyISO().slice(0, 8) + '01';
  const hoy = hoyISO();
  const [resumen, lista, finanzas] = await Promise.all([
    api.getGastosResumen(inicioMes, hoy),
    api.getGastos(inicioMes, hoy),
    api.getFinanzas(),
  ]);

  const setVal = (id, v) => { const e = document.getElementById(id); if (e) e.textContent = v; };
  const ingresos = (!hayProblema(finanzas) && finanzas.mes) ? finanzas.mes.total : 0;
  const totalGastos = hayProblema(resumen) ? 0 : resumen.total;
  const utilidad = Math.round((ingresos - totalGastos) * 100) / 100;

  setVal('gas-ingresos', formatMoney(ingresos));
  setVal('gas-total', formatMoney(totalGastos));
  setVal('gas-cantidad', hayProblema(resumen) ? '' : `${resumen.cantidad} gasto(s)`);
  setVal('gas-utilidad', formatMoney(utilidad));
  const nota = document.getElementById('gas-utilidad-nota');
  if (nota) {
    nota.textContent = utilidad >= 0 ? 'Ganancia del mes' : 'Pérdida: los gastos superan los ingresos';
    nota.style.color = utilidad >= 0 ? 'var(--green)' : 'var(--red)';
  }
  const valorUtilidad = document.getElementById('gas-utilidad');
  if (valorUtilidad) valorUtilidad.style.color = utilidad >= 0 ? 'var(--green)' : 'var(--red)';

  // En qué se va el dinero
  const chart = document.getElementById('gastos-chart');
  if (chart && !hayProblema(resumen)) {
    const cats = resumen.categorias || [];
    const max = Math.max(1, ...cats.map(c => c.total));
    chart.innerHTML = cats.length === 0
      ? '<p style="color:var(--gray-mid);">Sin gastos registrados este mes.</p>'
      : cats.map(c => `
          <div style="margin-bottom:1rem;">
            <div style="display:flex;justify-content:space-between;font-size:0.85rem;margin-bottom:0.3rem;">
              <strong>${esc(c.categoria)}</strong>
              <span style="color:var(--gray-mid);">${formatMoney(c.total)}</span>
            </div>
            <div style="height:10px;background:var(--gray-soft);border-radius:6px;overflow:hidden;">
              <div style="width:${(c.total / max) * 100}%;height:100%;background:var(--red);"></div>
            </div>
          </div>`).join('');
  }

  const body = document.getElementById('gastos-body');
  if (!body) return;
  if (!Array.isArray(lista)) {
    body.innerHTML = `<tr><td colspan="5" style="text-align:center;color:var(--red);padding:2rem;">
      ${esc((lista && lista.error) || 'No se pudieron cargar los gastos')}</td></tr>`;
    return;
  }
  if (lista.length === 0) {
    body.innerHTML = '<tr><td colspan="5" style="text-align:center;color:var(--gray-mid);padding:2rem;">Sin gastos este mes.</td></tr>';
    return;
  }
  body.innerHTML = lista.map(g => `
    <tr>
      <td><strong>${esc(g.categoria)}</strong></td>
      <td data-col="Fecha">${formatDate(g.fecha)}</td>
      <td data-col="Detalle">${esc(g.descripcion) || '—'} <small style="color:var(--gray-mid);">(${esc(g.metodo)})</small></td>
      <td data-col="Monto"><strong style="color:var(--red);">− S/ ${formatPrice(g.monto)}</strong></td>
      <td><button class="link-orange" style="color:var(--red);" data-gasto-del="${g.id}">Anular</button></td>
    </tr>
  `).join('');

  body.querySelectorAll('[data-gasto-del]').forEach(btn => {
    btn.addEventListener('click', async () => {
      if (!confirm('¿Anular este gasto?\n\nDejará de descontarse de las finanzas, pero el registro queda guardado.')) return;
      btn.disabled = true;
      const r = await api.deleteGasto(parseInt(btn.dataset.gastoDel, 10));
      if (r.error) { showToast(r.error, 'error'); btn.disabled = false; return; }
      showToast('Gasto anulado');
      renderGastos(); renderFinanzas(); renderDashboard();
    });
  });
}

const formGasto = document.getElementById('form-nuevo-gasto');
if (formGasto) {
  formGasto.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const res = await api.createGasto(Object.fromEntries(fd));
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast(`Gasto de S/ ${fd.get('monto')} registrado · ${fd.get('categoria')}`);
      closeModal('modal-gasto');
      e.target.reset();
      renderGastos(); renderFinanzas(); renderDashboard();
    });
  });
}

// ============ HISTORIAL DE UN SOCIO ============
async function abrirHistorial(id) {
  const modal = document.getElementById('modal-historial');
  const cont = document.getElementById('historial-contenido');
  if (!modal || !cont) return;
  cont.innerHTML = '<p style="color:var(--gray-mid);padding:1rem;">Cargando...</p>';
  modal.hidden = false;

  const h = await api.getHistorialSocio(id);
  if (hayProblema(h) || !h.socio) {
    cont.innerHTML = `<p style="color:var(--red);padding:1rem;">${esc((h && h.error) || 'No se pudo cargar el historial')}</p>`;
    return;
  }

  const titulo = document.getElementById('historial-titulo');
  if (titulo) titulo.textContent = h.socio.nombre;

  cont.innerHTML = `
    <div class="cards-row" style="grid-template-columns:repeat(3,1fr);gap:0.6rem;margin-bottom:1rem;">
      <div class="stat-card"><div class="stat-card__label">Total pagado</div>
        <div class="stat-card__value" style="font-size:1.4rem;">${formatMoney(h.resumen.total_pagado)}</div></div>
      <div class="stat-card"><div class="stat-card__label">Veces que pagó</div>
        <div class="stat-card__value" style="font-size:1.4rem;">${h.resumen.veces_pago}</div></div>
      <div class="stat-card"><div class="stat-card__label">Visitas</div>
        <div class="stat-card__value" style="font-size:1.4rem;">${h.resumen.total_visitas}</div>
        <div class="stat-card__trend">${h.resumen.ultima_visita ? 'última: ' + formatDate(h.resumen.ultima_visita) : 'nunca marcó entrada'}</div></div>
    </div>
    <p style="font-size:0.9rem;color:var(--gray-mid);margin-bottom:0.6rem;">
      ${esc(h.socio.concepto)} · vence ${formatDate(h.socio.fecha_vence)} ·
      ${h.socio.telefono ? 'tel. ' + esc(h.socio.telefono) : 'sin teléfono'}
    </p>
    <h3 style="font-size:0.95rem;margin:1rem 0 0.5rem;">Pagos</h3>
    <div class="table-wrap"><table><thead>
      <tr><th>Fecha</th><th>Tipo</th><th>Concepto</th><th>Monto</th></tr></thead><tbody>
      ${h.pagos.length === 0
        ? '<tr><td colspan="4" style="color:var(--gray-mid);padding:1rem;">Sin pagos registrados.</td></tr>'
        : h.pagos.map(p => `<tr${p.anulado ? ' style="opacity:0.5;text-decoration:line-through;"' : ''}>
            <td>${formatDate(p.fecha_pago)}</td>
            <td>${p.tipo === 'renovacion' ? 'Renovación' : 'Matrícula'}</td>
            <td>${esc(p.concepto)}${p.anulado ? ` <small>(anulado: ${esc(p.motivo_anulacion)})</small>` : ''}</td>
            <td><strong>S/ ${formatPrice(p.monto)}</strong></td></tr>`).join('')}
    </tbody></table></div>
    <h3 style="font-size:0.95rem;margin:1.2rem 0 0.5rem;">Últimas visitas</h3>
    <p style="font-size:0.88rem;color:var(--gray-mid);">
      ${h.visitas.length === 0 ? 'Todavía no ha marcado ninguna entrada.'
        : h.visitas.map(v => `${formatDate(v.fecha)}${v.hora ? ' ' + esc(v.hora) : ''}`).join(' · ')}
    </p>`;
}

// ============ MI CUENTA (cambiar contraseña) ============
// La API para cambiar la contraseña existía desde siempre, pero no había
// ningún botón que la usara: la dueña no podía cambiarla desde el panel.
const formPass = document.getElementById('form-cambiar-password');
if (formPass) {
  formPass.addEventListener('submit', (e) => {
    e.preventDefault();
    conBotonBloqueado(e.target, async () => {
      const fd = new FormData(e.target);
      const nueva = fd.get('nueva');
      if (nueva !== fd.get('repetir')) {
        showToast('La contraseña nueva no coincide en los dos campos', 'error');
        return;
      }
      const res = await api.changePassword(fd.get('actual'), nueva);
      if (res.error) { showToast(res.error, 'error'); return; }
      showToast('Contraseña actualizada ✓ Anótala en un lugar seguro');
      e.target.reset();
      closeModal('modal-cuenta');
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
        // El CSV lleva el historial completo, no solo lo que se ve en pantalla.
        const [pagos, extras] = await Promise.all([api.getPagos(), api.getExtras(5000)]);
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

  const btnElegir = document.getElementById('btn-elegir-respaldo');
  const inputRest = document.getElementById('input-restaurar');
  if (btnElegir && inputRest) {
    btnElegir.addEventListener('click', () => inputRest.click());
    inputRest.addEventListener('change', async () => {
      const archivo = inputRest.files && inputRest.files[0];
      if (!archivo) return;
      const etiqueta = document.getElementById('archivo-elegido');
      if (etiqueta) etiqueta.textContent = archivo.name;

      let texto;
      try {
        texto = await archivo.text();
        const prueba = JSON.parse(texto);
        if (!prueba || !prueba.tablas) throw new Error('sin tablas');
      } catch (e) {
        showToast('Ese archivo no es un respaldo válido de ZONA VIP GYM', 'error');
        inputRest.value = '';
        return;
      }

      const ok = confirm(
        'RESTAURAR RESPALDO\n\n' +
        'Los socios, pagos, ventas y precios actuales se REEMPLAZAN por los del archivo.\n' +
        'Tu contraseña no cambia.\n\n' +
        'Consejo: descarga primero un respaldo de lo que hay ahora.\n\n¿Continuar?'
      );
      if (!ok) { inputRest.value = ''; return; }

      btnElegir.disabled = true;
      const antes = btnElegir.textContent;
      btnElegir.textContent = 'Restaurando...';
      const r = await api.restaurarBackup(texto);
      btnElegir.disabled = false;
      btnElegir.textContent = antes;
      inputRest.value = '';
      if (r.error) { showToast(r.error, 'error'); return; }
      showToast('Respaldo restaurado ✓');
      cargarCatalogo(); renderMatriculas(); renderFinanzas(); renderDashboard(); renderExtras(); renderPorVencer();
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
