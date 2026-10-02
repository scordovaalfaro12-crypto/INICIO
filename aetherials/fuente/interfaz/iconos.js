// Iconos de línea en una grilla de 24 unidades (trazo 2, puntas redondeadas).
// Todos en blanco: en Roblox se tiñen con ImageColor3.
// Espadas cruzadas: la B pasa por detrás de la A (sus filos se cortan donde cruza la hoja A).
function espadas() {
  const r2 = Math.SQRT2, f = (v) => v.toFixed(2);
  const Lb = 17, Lh = 5, hw = 1.5, tip = 2.5, g = 2.5;
  function espada(T, d, n, cut) {
    const P = (t, s) => [T[0] + d[0] * t + n[0] * s, T[1] + d[1] * t + n[1] * s];
    let p = '';
    for (const s of [hw, -hw]) {
      const a = P(tip, s);
      p += `M${f(T[0])} ${f(T[1])}L${f(a[0])} ${f(a[1])}`;
      const tramos = cut ? [[tip, cut[0]], [cut[1], Lb]] : [[tip, Lb]];
      for (const [t0, t1] of tramos) {
        if (t1 - t0 < 0.3) continue;
        const q0 = P(t0, s), q1 = P(t1, s);
        p += `M${f(q0[0])} ${f(q0[1])}L${f(q1[0])} ${f(q1[1])}`;
      }
    }
    const b = P(Lb, 0), g0 = P(Lb, -g), g1 = P(Lb, g), h = P(Lb + Lh, 0);
    p += `M${f(g0[0])} ${f(g0[1])}L${f(g1[0])} ${f(g1[1])}M${f(b[0])} ${f(b[1])}L${f(h[0])} ${f(h[1])}`;
    return `<path d="${p}"/>`;
  }
  // hoja A: eje x+y=24; la B se corta donde |x+y-24| <= 4.95 (media hoja + trazo + aire)
  const c0 = (17 - 4.95) / r2, c1 = (17 + 4.95) / r2;
  return espada([3.5, 3.5], [1 / r2, 1 / r2], [1 / r2, -1 / r2], [c0, c1]) +
         espada([20.5, 3.5], [-1 / r2, 1 / r2], [1 / r2, 1 / r2], null);
}

const ICONOS = {
  equipo: `
    <path d="M6 10a4 4 0 0 1 4-4h4a4 4 0 0 1 4 4v9a2 2 0 0 1-2 2H8a2 2 0 0 1-2-2z"/>
    <path d="M9.5 6V4.5a2.5 2.5 0 0 1 5 0V6"/>
    <path d="M9 14.5h6v3.5H9z"/>`,
  mapa: `
    <path d="M3.5 6.5 9 4l6 2.5L20.5 4v13.5L15 20l-6-2.5L3.5 20z"/>
    <path d="M9 4v13.5"/><path d="M15 6.5V20"/>`,
  tienda: `
    <path d="M4 9.5 5.5 4h13L20 9.5"/>
    <path d="M4 9.5a2.67 2.67 0 0 0 5.33 0 2.67 2.67 0 0 0 5.34 0 2.67 2.67 0 0 0 5.33 0"/>
    <path d="M5.5 13v7.5h13V13"/>
    <path d="M10 20.5V16h4v4.5"/>`,
  caja: `
    <path d="M4 9.5A3.5 3.5 0 0 1 7.5 6h9A3.5 3.5 0 0 1 20 9.5V19a1 1 0 0 1-1 1H5a1 1 0 0 1-1-1z"/>
    <path d="M4 12h6.5M13.5 12H20"/>
    <rect x="10.5" y="10.5" width="3" height="4" rx="1"/>`,
  pvp: espadas(),
  misiones: `
    <path d="M15 4.5h2a2 2 0 0 1 2 2V19a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6.5a2 2 0 0 1 2-2h2"/>
    <rect x="9" y="3" width="6" height="3" rx="1"/>
    <path d="m8.5 12 2 2 4-4"/>
    <path d="M8.5 17.5h7"/>`,
  registro: `
    <path d="M5 19V5.5A2.5 2.5 0 0 1 7.5 3H19v13.5H7.5A2.5 2.5 0 0 0 5 19a2.5 2.5 0 0 0 2.5 2.5H19v-5"/>
    <path d="M12 6.5l2.75 3.25L12 13l-2.75-3.25z"/>`,
  ubicacion: `
    <path d="M12 21s-6.5-5.6-6.5-11a6.5 6.5 0 0 1 13 0c0 5.4-6.5 11-6.5 11z"/>
    <circle cx="12" cy="10" r="2.3"/>`,
  cerrar: `<path d="M6.5 6.5l11 11M17.5 6.5l-11 11"/>`,
};

// Emblema del Nexo en una grilla de 64: anillo con los cinco Guardianes y el cristal al centro.
function anilloNexo() {
  const c = 32, r = 25, gap = 9; // gap en grados alrededor de cada punto
  let s = '';
  for (let i = 0; i < 5; i++) {
    const a0 = -90 + i * 72 + gap, a1 = -90 + (i + 1) * 72 - gap;
    const p = (a) => [c + r * Math.cos(a * Math.PI / 180), c + r * Math.sin(a * Math.PI / 180)];
    const [x0, y0] = p(a0), [x1, y1] = p(a1);
    s += `<path d="M${x0.toFixed(3)} ${y0.toFixed(3)}A${r} ${r} 0 0 1 ${x1.toFixed(3)} ${y1.toFixed(3)}" fill="none"/>`;
    const [dx, dy] = p(-90 + i * 72);
    s += `<circle cx="${dx.toFixed(3)}" cy="${dy.toFixed(3)}" r="3.1" fill="#fff" stroke="none"/>`;
  }
  return s;
}
const CRISTAL = `
  <path d="M32 17 41 28 32 46 23 28z" fill="none"/>
  <path d="M23 28h18" fill="none"/>
  <path d="M28.5 28 32 17 35.5 28" fill="none"/>`;

module.exports = { ICONOS, anilloNexo, CRISTAL };
