// Service Worker básico para ZONA VIP GYM
// Permite instalar la PWA en el celular sin caché agresivo
// (para que siempre tenga la versión más nueva)

const VERSION = 'zvg-v2';

self.addEventListener('install', (event) => {
  self.skipWaiting();
});

self.addEventListener('activate', (event) => {
  event.waitUntil(self.clients.claim());
});

// Estrategia: network-first (siempre buscar lo nuevo, usar caché solo si no hay red)
self.addEventListener('fetch', (event) => {
  // No interceptar API calls (siempre datos frescos) ni descargas
  if (event.request.url.includes('/api/')) return;
  if (event.request.method !== 'GET') return;

  event.respondWith(
    fetch(event.request).catch(async () => {
      const cached = await caches.match(event.request);
      // Nunca responder undefined: eso rompe la página en vez de mostrar un aviso
      return cached || new Response('Sin conexión a internet', {
        status: 503,
        headers: { 'Content-Type': 'text/plain; charset=utf-8' },
      });
    })
  );
});
