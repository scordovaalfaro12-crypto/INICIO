// ==========================================================
//  Service Worker — ZONA VIP GYM
//
//  Estrategia: PRIMERO LA RED, la caché solo como red de seguridad.
//  Así el panel siempre muestra la versión más nueva; nunca se queda
//  atrapado en una versión vieja como pasa con otras PWA.
//
//  Antes este archivo nunca guardaba NADA en caché, así que la promesa
//  de "funciona sin internet" no existía: al perder señal, la app
//  mostraba un texto plano de error. Ahora la pantalla al menos abre y
//  explica lo que pasa.
// ==========================================================

const VERSION = 'zvg-v4';
const CACHE = `zvg-cache-${VERSION}`;

// Lo mínimo para que la aplicación abra sin internet.
const BASE = [
  '/login.html',
  '/admin.html',
  '/styles.css',
  '/assets/fonts.css',
  '/app.js',
  '/admin.js',
  '/login.js',
  '/manifest.json',
  '/assets/logo.png',
];

self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE)
      // Si un archivo falla, la instalación NO debe abortar: es preferible una
      // caché parcial a un service worker que nunca llega a instalarse.
      .then((c) => Promise.allSettled(BASE.map((u) => c.add(u))))
      .then(() => self.skipWaiting())
  );
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys()
      .then((claves) => Promise.all(
        claves.filter((k) => k !== CACHE).map((k) => caches.delete(k))
      ))
      .then(() => self.clients.claim())
  );
});

self.addEventListener('fetch', (event) => {
  const req = event.request;
  // La API nunca se cachea: el dinero y los socios se leen siempre frescos.
  if (req.url.includes('/api/')) return;
  if (req.method !== 'GET') return;
  if (!req.url.startsWith(self.location.origin)) return; // recursos externos (fuentes)

  event.respondWith(
    fetch(req)
      .then((res) => {
        // Copia al vuelo de lo que se descargó bien, para la próxima vez que
        // falte internet. Solo respuestas correctas y del propio sitio.
        if (res && res.ok && res.type === 'basic') {
          const copia = res.clone();
          caches.open(CACHE).then((c) => c.put(req, copia)).catch(() => {});
        }
        return res;
      })
      .catch(async () => {
        const guardado = await caches.match(req);
        if (guardado) return guardado;
        // Navegación sin caché previa: se devuelve la pantalla de acceso.
        if (req.mode === 'navigate') {
          const shell = await caches.match('/login.html');
          if (shell) return shell;
        }
        return new Response(
          'Sin conexión a internet. Vuelve a intentar cuando tengas señal.',
          { status: 503, headers: { 'Content-Type': 'text/plain; charset=utf-8' } }
        );
      })
  );
});
