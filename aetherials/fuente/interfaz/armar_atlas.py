"""Junta los PNG de render.js en el atlas de 1024x1024 y en los sprites sueltos.

Uso (en esta carpeta): node render.js && python3 armar_atlas.py
"""
import json
import os

from PIL import Image

O = 'out/'
D = 'sprites/'
os.makedirs(D + 'iconos', exist_ok=True)
ICONOS = ['equipo', 'mapa', 'tienda', 'caja', 'pvp', 'misiones', 'registro', 'ubicacion', 'cerrar']

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

atlas.save(D + 'Aetherials_UI_Atlas.png')
json.dump(rects, open(D + 'rects.json', 'w'), indent=1)
print('logo', rects['logo'], '-> copia estos números en SPRITES.logo del script de Roblox')
