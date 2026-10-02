"""Mini motor de pixel art para los iconos de Aetherials.

Reglas del paquete: alfa binario (0 o 255), contorno duro de 1 px y sin texto.
Cada icono se dibuja con figuras simples sobre una grilla; las piezas se pintan
con un marcador y `bisel` les pone luz arriba-izquierda y sombra abajo-derecha.
`contorno` agrega al final el borde de 1 px alrededor de la silueta.
"""
from PIL import Image

CONTORNO = (26, 22, 38)
BLANCO = (255, 255, 255)


def mezcla(a, b, t):
    return tuple(round(a[i] + (b[i] - a[i]) * t) for i in range(3))


def rampa(rgb):
    """(sombra, base, luz) a partir de un color base."""
    return (mezcla(rgb, (40, 22, 64), 0.38), tuple(rgb), mezcla(rgb, BLANCO, 0.42))


class Lienzo:
    def __init__(self, n):
        self.n = n
        self.p = {}

    # ---- figuras (rasterizadas por el centro de cada píxel, sin antialias)
    def px(self, x, y, c):
        if 0 <= x < self.n and 0 <= y < self.n:
            if c is None:
                self.p.pop((x, y), None)
            else:
                self.p[(x, y)] = c

    def rect(self, x0, y0, x1, y1, c):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.px(x, y, c)

    def elipse(self, cx, cy, rx, ry, c, dentro=None):
        for y in range(self.n):
            for x in range(self.n):
                dx, dy = (x + 0.5 - cx) / rx, (y + 0.5 - cy) / ry
                if dx * dx + dy * dy <= 1.0 and (dentro is None or dentro(x, y)):
                    self.px(x, y, c)

    def circulo(self, cx, cy, r, c, **kw):
        self.elipse(cx, cy, r, r, c, **kw)

    def anillo(self, cx, cy, r0, r1, c):
        for y in range(self.n):
            for x in range(self.n):
                d = ((x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2) ** 0.5
                if r0 <= d <= r1:
                    self.px(x, y, c)

    def poli(self, pts, c):
        for y in range(self.n):
            for x in range(self.n):
                if _dentro(pts, x + 0.5, y + 0.5):
                    self.px(x, y, c)

    def linea(self, x0, y0, x1, y1, c):
        dx, dy = abs(x1 - x0), -abs(y1 - y0)
        sx, sy = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1)
        err = dx + dy
        while True:
            self.px(x0, y0, c)
            if x0 == x1 and y0 == y1:
                return
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x0 += sx
            if e2 <= dx:
                err += dx
                y0 += sy

    def borrar(self, figura, *args):
        """Borra (deja transparente) los píxeles de una figura: getattr(self, figura)(*args, None)."""
        getattr(self, figura)(*args, None)

    # ---- color
    def bisel(self, marca, rmp, luz=True, sombra=True):
        """Pinta los píxeles con `marca`: luz si su vecino de arriba o de la izquierda no es de la pieza,
        sombra si lo es el de abajo o el de la derecha; base en el resto."""
        osc, base, cla = rmp
        nuevos = {}
        for (x, y), c in self.p.items():
            if c != marca:
                continue
            fuera = lambda xx, yy: self.p.get((xx, yy)) != marca
            if luz and (fuera(x, y - 1) or fuera(x - 1, y)):
                nuevos[(x, y)] = cla
            elif sombra and (fuera(x, y + 1) or fuera(x + 1, y)):
                nuevos[(x, y)] = osc
            else:
                nuevos[(x, y)] = base
        self.p.update(nuevos)

    def plano(self, marca, color):
        for k, c in list(self.p.items()):
            if c == marca:
                self.p[k] = color

    def contorno(self, color=CONTORNO):
        borde = []
        for y in range(self.n):
            for x in range(self.n):
                if (x, y) in self.p:
                    continue
                if any((x + dx, y + dy) in self.p for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                    borde.append((x, y))
        for k in borde:
            self.p[k] = color

    def imagen(self):
        im = Image.new('RGBA', (self.n, self.n), (0, 0, 0, 0))
        for (x, y), c in self.p.items():
            if isinstance(c, tuple) and len(c) == 3:
                im.putpixel((x, y), c + (255,))
            else:
                raise ValueError('marcador sin pintar en %s: %r' % ((x, y), c))
        return im


def _dentro(pts, x, y):
    dentro = False
    j = len(pts) - 1
    for i in range(len(pts)):
        xi, yi = pts[i]
        xj, yj = pts[j]
        if (yi > y) != (yj > y) and x < (xj - xi) * (y - yi) / (yj - yi) + xi:
            dentro = not dentro
        j = i
    return dentro


def validar(im, n):
    """Comprueba tamaño, alfa binario y que nada toque el borde sin contorno."""
    assert im.size == (n, n), im.size
    alfas = set(im.getchannel('A').getdata())
    assert alfas <= {0, 255}, 'alfa no binario: %s' % sorted(alfas)
    assert 255 in alfas, 'icono vacío'
    return True
