"""Lamina de la linea evolutiva: Vulcanid (render original del usuario) -> Basaltor -> Obsidrax.
Uso: python3 evolution_strip.py vulcanid.png basaltor.png obsidrax.png salida.png"""
import sys, os
from PIL import Image, ImageDraw
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sheet import font, crop_center, rounded, BG, INK, MUTED, MAGMA

paths, out = sys.argv[1:4], sys.argv[4]
labels = [('VULCANID', '1.ª forma'), ('BASALTOR', '2.ª forma'), ('OBSIDRAX', 'Forma final')]
PW, PH, GAP = 760, 660, 110
W = 3 * PW + 2 * GAP + 140
H = PH + 360
S = Image.new('RGB', (W, H), BG)
d = ImageDraw.Draw(S)
d.text((70, 44), 'LÍNEA DE VULCANID', font=font(64, True), fill=INK)
d.text((74, 124), 'Aetherials  ·  Tipo Fuego / Roca', font=font(28), fill=MUTED)
d.rectangle([70, 172, 300, 177], fill=MAGMA)
y0 = 220
for i, (p, (name, stage)) in enumerate(zip(paths, labels)):
    im = Image.open(p).convert('RGB')
    c = rounded(crop_center(im, PW, PH, 0.5, 0.55))
    x = 70 + i * (PW + GAP)
    S.paste(c, (x, y0), c)
    d.text((x + 6, y0 + PH + 18), name, font=font(40, True), fill=INK)
    d.text((x + 8, y0 + PH + 66), stage, font=font(26), fill=MUTED)
    if i < 2:
        ax = x + PW + GAP // 2
        ay = y0 + PH // 2
        d.polygon([(ax - 26, ay - 34), (ax + 30, ay), (ax - 26, ay + 34)], fill=MAGMA)
S.save(out)
