"""Junta los PNG de render.js en el atlas de 1024x1024 y en los sprites sueltos.

Uso (en esta carpeta): node render.js && python3 armar_atlas.py
"""
import json
import os

from PIL import Image, ImageDraw, ImageFilter

O = 'out/'
D = 'sprites/'
os.makedirs(D + 'iconos', exist_ok=True)
ICONOS = ['equipo', 'mapa', 'tienda', 'caja', 'pvp', 'misiones', 'registro', 'ubicacion', 'cerrar', 'jugar']

# fondo blanco transparente: al filtrar en Roblox los bordes no se oscurecen
atlas = Image.new('RGBA', (1024, 1024), (255, 255, 255, 0))
rects = {}
for i, n in enumerate(ICONOS):  # celdas de 128, 8 por fila
    x, y = (i % 8) * 128, (i // 8) * 128
    atlas.alpha_composite(Image.open(O + 'iconos_128/%s.png' % n), (x, y))
    rects[n] = [x, y, 128, 128]
    Image.open(O + 'iconos_256/%s.png' % n).save(D + 'iconos/%s.png' % n)
for j, n in enumerate(['nexo_anillo', 'nexo_cristal', 'nexo']):  # celdas de 256
    atlas.alpha_composite(Image.open(O + n + '_256.png'), (j * 256, 256))
    rects[n] = [j * 256, 256, 256, 256]
for f, n in [('nexo_anillo_512', 'Nexo_Anillo'), ('nexo_cristal_512', 'Nexo_Cristal'), ('nexo_512', 'Nexo_Emblema')]:
    Image.open(O + f + '.png').save(D + n + '.png')

# logotipo: recortado a su contenido y escalado a 960 px de ancho
logo = Image.open(O + 'logo_raw.png')
logo = logo.crop(logo.getbbox())
lw = 960
lh = round(lw * logo.height / logo.width)
logo = logo.resize((lw, lh), Image.LANCZOS)
atlas.alpha_composite(logo, (32, 520))
rects['logo'] = [28, 516, lw + 8, lh + 8]
suelto = Image.new('RGBA', (lw + 32, lh + 32), (255, 255, 255, 0))
suelto.alpha_composite(logo, (16, 16))
suelto.save(D + 'Logo_Aetherials.png')

def halo(alpha, radio, refuerzo):
    """Desenfoca un canal alfa y lo devuelve como sprite blanco (para halos que Roblox tiñe)."""
    a = alpha.filter(ImageFilter.GaussianBlur(radio)).point(lambda v: min(255, int(v * refuerzo)))
    img = Image.new('RGBA', alpha.size, (255, 255, 255, 0))
    img.putalpha(a)
    return img


# halo del logotipo: mismo tamaño de letra que el logo y 32 px de margen
y = 516 + rects['logo'][3] + 12
base = Image.new('L', (lw + 64, lh + 64), 0)
base.paste(logo.getchannel('A'), (32, 32))
atlas.alpha_composite(halo(base, 10, 1.8), (0, y))
rects['logo_brillo'] = [0, y, lw + 64, lh + 64]

# halo del botón JUGAR: una píldora de 288x64 desenfocada en 384x160
y += lh + 64 + 12
pildora = Image.new('L', (384, 160), 0)
ImageDraw.Draw(pildora).rounded_rectangle((48, 48, 335, 111), radius=32, fill=255)
atlas.alpha_composite(halo(pildora, 18, 1.3), (0, y))
rects['brillo_boton'] = [0, y, 384, 160]

# brillo redondo (detrás del cristal del Nexo)
redondo = Image.new('L', (128, 128), 0)
redondo.putdata([int(255 * max(0.0, 1 - ((x - 63.5) ** 2 + (yy - 63.5) ** 2) ** 0.5 / 62) ** 2)
                 for yy in range(128) for x in range(128)])
atlas.alpha_composite(halo(redondo, 0, 1.0), (768, 256))
rects['brillo_redondo'] = [768, 256, 128, 128]

atlas.save(D + 'Aetherials_UI_Atlas.png')
json.dump(rects, open(D + 'rects.json', 'w'), indent=1)
for n in ['logo', 'logo_brillo', 'brillo_boton', 'brillo_redondo', 'jugar']:
    print(n, rects[n])

