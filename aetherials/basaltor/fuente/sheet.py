"""Hoja de concepto y vista explotada etiquetada (composicion 2D con Pillow)."""
import json, sys, os
from PIL import Image, ImageDraw, ImageFont, ImageFilter
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lib import PALETTE

F = '/usr/share/fonts/truetype/dejavu/'
def font(sz, bold=False):
    return ImageFont.truetype(F + ('DejaVuSans-Bold.ttf' if bold else 'DejaVuSans.ttf'), sz)

BG = (24, 33, 46)
INK = (232, 226, 214)
MUTED = (150, 165, 184)
MAGMA = (255, 122, 51)


def crop_center(im, w, h, cx=0.5, cy=0.5):
    W, H = im.size
    s = max(w / W, h / H)
    im = im.resize((int(W * s), int(H * s)), Image.LANCZOS)
    W, H = im.size
    x = int((W - w) * cx)
    y = int((H - h) * cy)
    return im.crop((x, y, x + w, y + h))


def rounded(im, r=18):
    m = Image.new('L', im.size, 0)
    ImageDraw.Draw(m).rounded_rectangle([0, 0, im.size[0] - 1, im.size[1] - 1], r, fill=255)
    out = Image.new('RGBA', im.size)
    out.paste(im, (0, 0), m)
    return out


def concept_sheet(rdir, out):
    W, H = 2400, 1500
    S = Image.new('RGB', (W, H), BG)
    d = ImageDraw.Draw(S)
    d.text((70, 52), 'BASALTOR', font=font(92, True), fill=INK)
    d.text((76, 160), 'Aetherials  ·  Evolución final de Vulcanid  ·  Tipo Fuego / Roca', font=font(30), fill=MUTED)
    d.rectangle([70, 214, 330, 220], fill=MAGMA)
    hero = Image.open(os.path.join(rdir, 'Basaltor_01_tres_cuartos.png')).convert('RGB')
    S.paste(rounded(crop_center(hero, 1180, 1160, 0.5, 0.55)), (70, 260), rounded(crop_center(hero, 1180, 1160, 0.5, 0.55)))
    views = [('Basaltor_05_lado.png', 'Lado', 0.5, 0.5), ('Basaltor_06_frente.png', 'Frente', 0.5, 0.5),
             ('Basaltor_04_atras.png', 'Atrás', 0.5, 0.5), ('Basaltor_02_cabeza.png', 'Cabeza', 0.5, 0.5)]
    x0, y0, cw, ch = 1290, 260, 520, 347
    for i, (fn, lab, cx, cy) in enumerate(views):
        im = Image.open(os.path.join(rdir, fn)).convert('RGB')
        c = rounded(crop_center(im, cw, ch, cx, cy))
        x = x0 + (i % 2) * (cw + 30)
        y = y0 + (i // 2) * (ch + 73)
        S.paste(c, (x, y), c)
        d.text((x + 4, y + ch + 10), lab, font=font(26, True), fill=INK)
    # paleta
    py = 1150
    d.text((1290, py - 46), 'Paleta (heredada de Vulcanid)', font=font(26, True), fill=INK)
    keys = ['Cuerpo', 'Placa', 'PlacaClara', 'CabezaGris', 'Hocico', 'Basalto', 'Hueso', 'Ojo', 'Magma', 'MagmaCaliente']
    pal = {n: h for (n, h, *_r) in PALETTE}
    for k, n in enumerate(keys):
        hx = pal[n]
        col = tuple(int(hx[i:i + 2], 16) for i in (1, 3, 5))
        x = 1290 + k * 107
        d.rounded_rectangle([x, py, x + 92, py + 80], 12, fill=col, outline=(60, 72, 90), width=2)
        d.text((x, py + 88), hx, font=font(17), fill=MUTED)
    ty = 1310
    for line in ('Columnas de basalto en panal con una grieta de magma a lo largo del lomo',
                 'Mazo de basalto en la cola  ·  lengua de lava  ·  cuernos estriados',
                 '30 piezas  ·  22 huesos  ·  24.316 triángulos, listo para Roblox'):
        d.ellipse([1292, ty + 10, 1304, ty + 22], fill=MAGMA)
        d.text((1318, ty), line, font=font(24), fill=INK)
        ty += 40
    S.save(out, quality=95)


def exploded(raw, js, out):
    im = Image.open(raw).convert('RGB')
    pts = json.load(open(js))
    d = ImageDraw.Draw(im)
    f = font(26, True)
    W, H = im.size
    placed = []
    for n, (lab, x, y) in sorted(pts.items(), key=lambda kv: kv[1][2]):
        # etiqueta desplazada hacia afuera del centro de la imagen
        dx = 1 if x > W * 0.5 else -1
        tx = x + dx * 120
        ty = y - 40
        for (px, py) in placed:
            if abs(px - tx) < 230 and abs(py - ty) < 38:
                ty = py + 40
        placed.append((tx, ty))
        tw = d.textlength(lab, font=f)
        bx = tx if dx > 0 else tx - tw
        bx = max(22, min(W - tw - 26, bx))  # que la etiqueta no se salga del borde
        d.line([(x, y), (tx, ty + 16)], fill=(255, 255, 255), width=2)
        d.ellipse([x - 6, y - 6, x + 6, y + 6], fill=MAGMA, outline=(255, 255, 255), width=2)
        d.rounded_rectangle([bx - 10, ty - 4, bx + tw + 10, ty + 36], 8, fill=(20, 28, 40))
        d.text((bx, ty), lab, font=f, fill=INK)
    d.text((40, 30), 'BASALTOR  ·  piezas separadas', font=font(46, True), fill=(20, 28, 40))
    d.text((42, 90), 'Cada pieza tiene su pivote en la articulación (listo para huesos o Motor6D en Roblox)',
           font=font(24), fill=(40, 52, 70))
    im.save(out)


if __name__ == '__main__':
    mode = sys.argv[1]
    if mode == 'sheet':
        concept_sheet(sys.argv[2], sys.argv[3])
    else:
        exploded(sys.argv[2], sys.argv[3], sys.argv[4])
