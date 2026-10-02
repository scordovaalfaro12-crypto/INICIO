"""Hojas de revisión (no se suben): vista previa sobre fondo oscuro y claro, y rejilla ampliada."""
from PIL import Image, ImageDraw, ImageFont

OSCURO = (27, 30, 38)
CLARO = (226, 228, 232)


def hoja_preview(iconos, ruta, escala=4, columnas=6):
    """iconos: lista de (nombre, Image). Cada celda muestra el icono a 1x, a `escala`x sobre oscuro y sobre claro."""
    n = iconos[0][1].width
    fuente = ImageFont.load_default(size=13)
    cw, ch = n * escala * 2 + n + 40, n * escala + 34
    filas = (len(iconos) + columnas - 1) // columnas
    hoja = Image.new('RGB', (columnas * cw + 16, filas * ch + 16), (16, 18, 24))
    d = ImageDraw.Draw(hoja)
    for i, (nombre, im) in enumerate(iconos):
        x, y = 8 + (i % columnas) * cw, 8 + (i // columnas) * ch
        grande = im.resize((n * escala, n * escala), Image.NEAREST)
        for k, fondo in enumerate((OSCURO, CLARO)):
            bx = x + k * (n * escala + 6)
            d.rectangle([bx, y, bx + n * escala - 1, y + n * escala - 1], fill=fondo)
            hoja.paste(grande, (bx, y), grande)
        sx = x + 2 * (n * escala + 6)
        d.rectangle([sx, y, sx + n - 1, y + n - 1], fill=OSCURO)
        hoja.paste(im, (sx, y), im)
        d.text((x, y + n * escala + 4), nombre, fill=(200, 205, 215), font=fuente)
    hoja.save(ruta)


def hoja_rejilla(iconos, ruta, escala=12, columnas=5):
    n = iconos[0][1].width
    fuente = ImageFont.load_default(size=14)
    cw, ch = n * escala + 16, n * escala + 30
    filas = (len(iconos) + columnas - 1) // columnas
    hoja = Image.new('RGB', (columnas * cw + 16, filas * ch + 16), (16, 18, 24))
    d = ImageDraw.Draw(hoja)
    for i, (nombre, im) in enumerate(iconos):
        x, y = 8 + (i % columnas) * cw, 8 + (i // columnas) * ch
        for yy in range(n):
            for xx in range(n):
                c = (58, 62, 72) if (xx + yy) % 2 else (48, 52, 62)
                d.rectangle([x + xx * escala, y + yy * escala, x + (xx + 1) * escala - 1, y + (yy + 1) * escala - 1], fill=c)
        g = im.resize((n * escala, n * escala), Image.NEAREST)
        hoja.paste(g, (x, y), g)
        for k in range(n + 1):
            col = (90, 96, 110) if k % 4 else (130, 138, 155)
            d.line([x + k * escala, y, x + k * escala, y + n * escala], fill=col)
            d.line([x, y + k * escala, x + n * escala, y + k * escala], fill=col)
        d.text((x, y + n * escala + 6), nombre, fill=(200, 205, 215), font=fuente)
    hoja.save(ruta)
