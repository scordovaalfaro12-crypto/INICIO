// ==========================================================
//  login.js — Pantalla de acceso
//  Antes este código vivía dentro del HTML. Al sacarlo a su propio
//  archivo, el navegador puede bloquear cualquier script que no
//  venga de este servidor (ver Content-Security-Policy).
// ==========================================================

// Si la sesión sigue vigente, se entra directo: abrir la app instalada en el
// celular ya no obliga a escribir la contraseña otra vez cada día.
(async function () {
  if (!getToken()) return;
  const user = await api.me();
  if (user && !user.error && ['admin', 'recepcion'].includes(user.role)) {
    window.location.replace('admin.html');
  }
})();

if ('serviceWorker' in navigator) {
  navigator.serviceWorker.register('/sw.js').catch(() => {});
}

const formLogin = document.getElementById('form-login');
if (formLogin) {
  formLogin.addEventListener('submit', async (e) => {
    e.preventDefault();
    const boton = formLogin.querySelector('button[type=submit]');
    const fd = new FormData(formLogin);
    if (boton) { boton.disabled = true; boton.textContent = 'Entrando...'; }
    try {
      const res = await api.login(fd.get('email').trim().toLowerCase(), fd.get('password'));
      if (res.error) { showToast(res.error, 'error'); return; }
      if (!res.user || !['admin', 'recepcion'].includes(res.user.role)) {
        showToast('Solo personal autorizado puede ingresar', 'error');
        return;
      }
      setToken(res.token);
      showToast('Bienvenido ' + res.user.firstname);
      setTimeout(() => window.location.href = 'admin.html', 600);
    } catch (err) {
      showToast('No se pudo conectar. Revisa tu internet.', 'error');
    } finally {
      if (boton) { boton.disabled = false; boton.textContent = 'Ingresar'; }
    }
  });
}
