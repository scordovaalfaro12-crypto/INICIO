"""Lamina de una linea evolutiva: imagenes de cada forma con flechas.
Uso: python3 evolution_strip.py salida.png "TITULO" img1:NOMBRE:forma img2:NOMBRE:forma ...
     (el panel puede llevar recorte: img:NOMBRE:forma:x0,y0,x1,y1)"""
import sys, os
from PIL import Image, ImageDraw
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sheet import font, crop_center, rounded, BG, INK, MUTED, MAGMA

out, title = sys.argv[1], sys.argv[2]
panels = [a.split(':') for a in sys.argv[3:]]
N = len(panels)
PW, PH, GAP = 760, 660, 110
W = N * PW + (N - 1) * GAP + 140
H = PH + 360
S = Image.new('RGB', (W, H), BG)
d = ImageDraw.Draw(S)
t1, t2 = title.split('|')
d.text((70, 44), t1, font=font(64, True), fill=INK)
d.text((74, 124), t2, font=font(28), fill=MUTED)
d.rectangle([70, 172, 300, 177], fill=MAGMA)
y0 = 220
for i, pan in enumerate(panels):
    p, name, stage = pan[0], pan[1], pan[2]
    im = Image.open(p).convert('RGB')
    if len(pan) > 3:
        im = im.crop(tuple(int(v) for v in pan[3].split(',')))
    c = rounded(crop_center(im, PW, PH, 0.5, 0.55))
    x = 70 + i * (PW + GAP)
    S.paste(c, (x, y0), c)
    d.text((x + 6, y0 + PH + 18), name, font=font(40, True), fill=INK)
    d.text((x + 8, y0 + PH + 66), stage, font=font(26), fill=MUTED)
    if i < N - 1:
        ax = x + PW + GAP // 2
        ay = y0 + PH // 2
        d.polygon([(ax - 26, ay - 34), (ax + 30, ay), (ax - 26, ay + 34)], fill=MAGMA)
S.save(out)
