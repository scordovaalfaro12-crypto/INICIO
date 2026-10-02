// Renderiza los sprites (SVG -> PNG transparente) con Chromium.
const { chromium } = require('playwright-core');
const fs = require('fs');
const path = require('path');
const { ICONOS, anilloNexo, CRISTAL } = require('./iconos');

const OUT = path.join(__dirname, 'out');
// Chromium de Playwright; cambia la ruta si lo tienes en otro lugar
const CHROME = process.env.CHROME || '/opt/pw-browsers/chromium-1194/chrome-linux/chrome';
const FONT = fs.readFileSync(require.resolve('@fontsource-variable/montserrat/files/montserrat-latin-wght-normal.woff2')).toString('base64');

function iconSVG(name, size, stroke = 2) {
  const pad = size * 0.09; // margen transparente: evita que se mezclen celdas del atlas
  const inner = size - 2 * pad;
  return `<svg xmlns="http://www.w3.org/2000/svg" width="${size}" height="${size}" viewBox="0 0 ${size} ${size}">
    <g transform="translate(${pad} ${pad}) scale(${inner / 24})" fill="none" stroke="#fff"
       stroke-width="${stroke}" stroke-linecap="round" stroke-linejoin="round">${ICONOS[name]}</g></svg>`;
}

function nexoSVG(size, parts, stroke = 2.6) {
  let g = '';
  if (parts.includes('anillo')) g += anilloNexo();
  if (parts.includes('cristal')) g += CRISTAL;
  return `<svg xmlns="http://www.w3.org/2000/svg" width="${size}" height="${size}" viewBox="0 0 64 64">
    <g fill="none" stroke="#fff" stroke-width="${stroke}" stroke-linecap="round" stroke-linejoin="round">${g}</g></svg>`;
}

function wordmarkHTML(text, px, weight, tracking) {
  return `<html><head><style>
    @font-face { font-family: M; src: url(data:font/woff2;base64,${FONT}) format('woff2'); font-weight: 100 900; }
    html,body { margin:0; background: transparent; }
    #t { font-family: M; font-weight: ${weight}; font-size: ${px}px; letter-spacing: ${tracking}em; color: #fff;
         white-space: nowrap; padding: ${px * 0.3}px ${px * 0.5}px; display: inline-block; line-height: 1; }
  </style></head><body><div id="t">${text}</div></body></html>`;
}

(async () => {
  fs.mkdirSync(OUT, { recursive: true });
  const browser = await chromium.launch({ executablePath: CHROME });
  const page = await browser.newPage({ deviceScaleFactor: 1 });
  async function shot(svg, w, h, file) {
    await page.setViewportSize({ width: w, height: h });
    await page.setContent(`<html><body style="margin:0;background:transparent">${svg}</body></html>`);
    await page.screenshot({ path: path.join(OUT, file), omitBackground: true, clip: { x: 0, y: 0, width: w, height: h } });
  }
  for (const size of [128, 256]) {
    fs.mkdirSync(path.join(OUT, `iconos_${size}`), { recursive: true });
    for (const name of Object.keys(ICONOS)) await shot(iconSVG(name, size), size, size, `iconos_${size}/${name}.png`);
  }
  for (const size of [256, 512]) {
    await shot(nexoSVG(size, ['anillo']), size, size, `nexo_anillo_${size}.png`);
    await shot(nexoSVG(size, ['cristal']), size, size, `nexo_cristal_${size}.png`);
    await shot(nexoSVG(size, ['anillo', 'cristal']), size, size, `nexo_${size}.png`);
  }
  // logotipo en grande; el recorte y el escalado final se hacen con PIL
  await page.setViewportSize({ width: 3000, height: 600 });
  await page.setContent(wordmarkHTML(process.env.LOGO || 'AETHERIALS', 220, process.env.PESO || 800, process.env.TRACK || 0.16));
  await page.evaluate(() => document.fonts.ready);
  const el = await page.$('#t');
  await el.screenshot({ path: path.join(OUT, 'logo_raw.png'), omitBackground: true });
  await browser.close();
  console.log('ok');
})();
