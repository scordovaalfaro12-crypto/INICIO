"""Iconos de la barra de acciones (24x24, _reposo y _activo) y avisos (16x16)."""
import math

from objetos import CREMA, MADERA, METAL, ORO, VIDRIO, destello
from pixel import BLANCO, Lienzo, mezcla, rampa

N = 24
ACENTO = {
    'equipo': (90, 170, 255), 'objetos': (225, 140, 75), 'mapa': (80, 210, 200), 'tienda': (80, 200, 120),
    'caja': (170, 120, 240), 'pvp': (255, 90, 80), 'misiones': (230, 190, 70), 'registro': (240, 120, 180),
}
PAPEL = ((186, 150, 104), (230, 206, 156), (248, 234, 196))
OSCURO = (40, 30, 58)


def r(nombre, f=0.0):
    """Rampa del acento; f>0 la oscurece."""
    return rampa(mezcla(ACENTO[nombre], (30, 20, 50), f))


# ------------------------------------------------------------- equipo: mochila

def _mochila(L, abierta):
    a = r('equipo')
    if abierta:
        L.anillo(12, 3.6, 1.3, 2.5, 'asa')
        L.p = {k: v for k, v in L.p.items() if not (v == 'asa' and k[1] > 3)}
        L.bisel('asa', r('equipo', 0.35))
        L.poli([(5, 9), (19, 9), (17.5, 3.5), (6.5, 3.5)], 'sol')
        L.bisel('sol', r('equipo', 0.45))
        L.linea(7, 4, 16, 4, r('equipo', 0.2)[2])
    else:
        L.anillo(12, 6.4, 1.5, 2.7, 'asa')
        L.p = {k: v for k, v in L.p.items() if not (v == 'asa' and k[1] > 6)}
        L.bisel('asa', r('equipo', 0.35))
    L.rect(5, 8, 18, 21, 'cue')
    for x, y in ((5, 21), (18, 21)):
        L.px(x, y, None)
    L.bisel('cue', a)
    L.rect(8, 15, 15, 19, 'bol')
    L.bisel('bol', r('equipo', 0.15))
    L.linea(8, 15, 15, 15, a[2])
    if abierta:
        L.rect(6, 9, 17, 10, OSCURO)
        L.rect(9, 7, 10, 8, (120, 230, 240))
        L.px(9, 7, BLANCO)
        L.rect(13, 8, 14, 8, ORO[1])
        L.px(14, 7, ORO[2])
        destello(L, 21, 7)
    else:
        L.rect(5, 8, 18, 12, 'sol')
        L.px(5, 12, None)
        L.px(18, 12, None)
        L.bisel('sol', r('equipo', 0.3))
        L.rect(11, 11, 12, 14, 'heb')
        L.bisel('heb', ORO)

def equipo_reposo():
    L = Lienzo(N)
    _mochila(L, False)
    return L


def equipo_activo():
    L = Lienzo(N)
    _mochila(L, True)
    return L


# ------------------------------------------------------------- objetos: frasco

def _frasco(L, alto, saltando):
    L.rect(10, 5, 13, 9, 'vid')
    L.circulo(11.5, 15, 6.6, 'vid')
    L.bisel('vid', VIDRIO)
    nivel = 11 if alto else 15
    L.circulo(11.5, 15, 5.2, 'liq', dentro=lambda x, y: y >= nivel)
    if alto:
        L.rect(11, 8, 12, 9, 'liq')
    L.bisel('liq', r('objetos'))
    if not alto:
        L.linea(7, nivel, 16, nivel, r('objetos')[2])
    L.px(8, 12, BLANCO)
    L.px(8, 13, BLANCO)
    if saltando:
        L.poli([(13, 0.5), (17, 1.5), (16.2, 4.6), (12.2, 3.6)], 'cor')
        L.bisel('cor', MADERA)
        for x, y in ((10, 3), (12, 1), (9, 1)):
            L.px(x, y, r('objetos')[2])
        for x, y in ((10, 14), (13, 12), (11, 17)):
            L.px(x, y, BLANCO)
        destello(L, 20, 8)
    else:
        L.rect(10, 2, 13, 4, 'cor')
        L.bisel('cor', MADERA)


def objetos_reposo():
    L = Lienzo(N)
    _frasco(L, False, False)
    return L


def objetos_activo():
    L = Lienzo(N)
    _frasco(L, True, True)
    return L


# ------------------------------------------------------------- mapa: pergamino

def mapa_reposo():
    L = Lienzo(N)
    L.rect(4, 9, 19, 15, 'pap')
    L.bisel('pap', PAPEL)
    L.elipse(4.5, 12.5, 1.6, 3.6, 'end')
    L.bisel('end', PAPEL)
    L.px(4, 12, PAPEL[0])
    L.elipse(19.5, 12.5, 1.6, 3.6, 'end2')
    L.bisel('end2', PAPEL)
    L.rect(11, 8, 12, 16, 'cin')
    L.bisel('cin', r('mapa'))
    L.poli([(11, 16), (12.5, 16), (11, 20), (9.5, 19)], 'col')
    L.poli([(11.5, 16), (13, 16), (14.5, 19), (13, 20)], 'col')
    L.bisel('col', r('mapa'))
    return L


def mapa_activo():
    L = Lienzo(N)
    L.rect(4, 4, 19, 19, 'pap')
    L.bisel('pap', PAPEL)
    for x0 in (2, 19):
        L.rect(x0, 3, x0 + 2, 20, 'rol%d' % x0)
        L.bisel('rol%d' % x0, PAPEL)
    # río, montes, camino y destino
    for x, y in ((6, 7), (7, 8), (7, 9), (8, 10), (8, 11), (7, 12), (7, 13), (8, 14)):
        L.px(x, y, r('mapa')[1])
    for cx in (14, 17):
        L.poli([(cx - 2, 18), (cx, 14.5), (cx + 2, 18)], (150, 140, 120))
    for x, y in ((10, 16), (11, 14), (12, 12), (13, 11), (14, 9)):
        L.px(x, y, r('mapa', 0.3)[0])
    L.circulo(16, 7.5, 1.9, (230, 70, 70))
    L.px(15, 7, (255, 170, 160))
    return L


# ------------------------------------------------------------- tienda: puesto con toldo

def _toldo(L):
    L.poli([(4, 9), (5.5, 3), (18.5, 3), (20, 9)], 'tol')
    for k in range(6):
        L.circulo(5.33 + k * 2.67, 9, 1.4, 'tol', dentro=lambda x, y: y >= 9)
    v = r('tienda')
    for (x, y), c in list(L.p.items()):
        if c == 'tol':
            L.p[(x, y)] = v[1] if ((x - 4) // 3) % 2 == 0 else (244, 248, 240)
    L.linea(5, 3, 18, 3, v[2])


def tienda_reposo():
    L = Lienzo(N)
    L.rect(4, 9, 5, 21, MADERA[0])
    L.rect(18, 9, 19, 21, MADERA[0])
    L.rect(6, 11, 17, 20, 'per')
    L.bisel('per', METAL)
    for y in range(12, 20, 2):
        L.linea(6, y, 17, y, METAL[0])
    _toldo(L)
    return L


def tienda_activo():
    L = Lienzo(N)
    L.rect(4, 9, 5, 21, MADERA[0])
    L.rect(18, 9, 19, 21, MADERA[0])
    L.rect(6, 11, 17, 15, OSCURO)
    L.rect(4, 16, 19, 20, 'mos')
    L.bisel('mos', MADERA)
    # mercancía: frasco y monedas
    L.rect(8, 12, 9, 13, VIDRIO[1])
    L.rect(7, 14, 10, 15, r('tienda')[1])
    L.circulo(14, 14.5, 1.6, ORO[1])
    L.circulo(15.5, 13.2, 1.4, ORO[2])
    _toldo(L)
    destello(L, 21, 13)
    return L


# ------------------------------------------------------------- caja: cofre con núcleo

def _cofre(L, abierta):
    m = r('caja')
    if abierta:
        L.poli([(4, 11), (20, 11), (18, 4), (6, 4)], 'tap')
        L.bisel('tap', r('caja', 0.5))
        L.linea(7, 5, 16, 5, r('caja', 0.3)[2])
    L.rect(4, 11, 19, 20, 'cue')
    L.bisel('cue', m)
    for x in (7, 16):
        L.rect(x, 12, x, 20, ORO[0])
    if abierta:
        L.rect(5, 11, 18, 12, OSCURO)
        L.circulo(12, 9.5, 3.3, 'nuc')
        L.bisel('nuc', ((176, 120, 250), (222, 196, 255), (255, 255, 255)))
        L.rect(11, 8, 12, 9, BLANCO)
        for x, y in ((12, 4), (12, 3), (6, 7), (5, 6), (18, 7), (19, 6)):
            L.px(x, y, (232, 214, 255))
        L.rect(10, 14, 13, 16, 'cer')
        L.bisel('cer', ORO)
    else:
        L.rect(4, 6, 19, 11, 'tap')
        L.px(4, 6, None)
        L.px(19, 6, None)
        L.bisel('tap', r('caja', 0.15))
        L.linea(4, 11, 19, 11, m[0])
        for x in (7, 16):
            L.rect(x, 7, x, 10, ORO[0])
        L.rect(10, 9, 13, 13, 'cer')
        L.bisel('cer', ORO)
        L.px(11, 11, (232, 214, 255))
        L.px(12, 11, (232, 214, 255))
        L.px(11, 12, (200, 160, 255))

def caja_reposo():
    L = Lienzo(N)
    _cofre(L, False)
    return L


def caja_activo():
    L = Lienzo(N)
    _cofre(L, True)
    return L


# ------------------------------------------------------------- pvp: espadas

def _espada_vertical(L, x, marca):
    L.rect(x - 1, 4, x + 1, 14, marca)
    L.poli([(x - 1.5, 4.5), (x + 0.5, 1), (x + 2.5, 4.5)], marca)
    L.bisel(marca, METAL)
    L.linea(x, 3, x, 13, BLANCO)
    L.rect(x - 3, 15, x + 3, 16, 'gua')
    L.rect(x, 17, x, 20, 'emp')
    L.circulo(x + 0.5, 21.2, 1.2, ORO[1])


def pvp_reposo():
    L = Lienzo(N)
    _espada_vertical(L, 8, 'h1')
    _espada_vertical(L, 15, 'h2')
    L.bisel('gua', r('pvp'))
    L.plano('emp', r('pvp', 0.4)[0])
    return L


def pvp_activo():
    L = Lienzo(N)
    rojo = r('pvp')
    from pixel import CONTORNO

    def espada(punta, guarda, pomo, marca):
        dx, dy = guarda[0] - punta[0], guarda[1] - punta[1]
        lg = (dx * dx + dy * dy) ** 0.5
        ux, uy = dx / lg, dy / lg
        nx, ny = -uy, ux
        w = 1.25
        hoja = [punta, (punta[0] + ux * 2.4 + nx * w, punta[1] + uy * 2.4 + ny * w),
                (guarda[0] + nx * w, guarda[1] + ny * w), (guarda[0] - nx * w, guarda[1] - ny * w),
                (punta[0] + ux * 2.4 - nx * w, punta[1] + uy * 2.4 - ny * w)]
        return hoja, (ux, uy, nx, ny)

    def pintar(punta, guarda, pomo, marca, separar):
        hoja, (ux, uy, nx, ny) = espada(punta, guarda, pomo, marca)
        tmp = Lienzo(N)
        tmp.poli(hoja, 1)
        if separar:  # contorno de 1 px donde esta hoja pasa sobre la otra espada
            for (x, y) in tmp.p:
                for ddx, ddy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    q = (x + ddx, y + ddy)
                    if q not in tmp.p and q in L.p:
                        L.p[q] = CONTORNO
        L.poli(hoja, marca)
        L.bisel(marca, METAL)
        gx, gy = guarda
        g, e = 3.6, 1.0
        L.poli([(gx + nx * g - ux * e, gy + ny * g - uy * e), (gx + nx * g + ux * e, gy + ny * g + uy * e),
                (gx - nx * g + ux * e, gy - ny * g + uy * e), (gx - nx * g - ux * e, gy - ny * g - uy * e)], 'gua' + marca)
        L.bisel('gua' + marca, rojo)
        L.linea(round(gx + ux * 2), round(gy + uy * 2), round(pomo[0]), round(pomo[1]), r('pvp', 0.45)[0])
        L.circulo(pomo[0] + 0.5, pomo[1] + 0.5, 1.3, ORO[1])

    pintar((3, 2.5), (15.5, 15), (19, 18.5), 'b', False)
    pintar((21, 2.5), (8.5, 15), (5, 18.5), 'a', True)
    # brillo en el cruce
    for x, y in ((12, 6), (12, 7), (9, 9), (15, 9), (12, 13)):
        L.px(x, y, (255, 214, 150))
    destello(L, 12, 9, (255, 248, 230), grande=True)
    return L

def _tablon(L, marcadas):
    L.rect(3, 3, 20, 20, 'mad')
    L.bisel('mad', MADERA)
    L.rect(5, 5, 18, 19, 'hoj')
    L.bisel('hoj', PAPEL)
    L.circulo(12, 5, 1.5, 'chi')
    L.bisel('chi', r('misiones'))
    visto = ["....X", "...XX", "X.XX.", "XXX..", ".X..."]
    for y in (8, 12, 16):
        L.rect(7, y, 9, y + 2, (96, 74, 62))
        L.px(8, y + 1, PAPEL[2])
        L.linea(11, y + 1, 16, y + 1, PAPEL[0])
        if marcadas:
            for j, fila in enumerate(visto):
                for i, ch in enumerate(fila):
                    if ch == 'X':
                        L.px(7 + i, y - 2 + j, (60, 170, 70) if j < 3 else (40, 120, 52))
    if marcadas:
        destello(L, 20, 2, r('misiones')[2])

def misiones_reposo():
    L = Lienzo(N)
    _tablon(L, False)
    return L


def misiones_activo():
    L = Lienzo(N)
    _tablon(L, True)
    return L


# ------------------------------------------------------------- registro: libro con gema

def _gema(L, cx, cy, marca, rmp):
    L.poli([(cx, cy - 3.5), (cx + 2.6, cy), (cx, cy + 3.5), (cx - 2.6, cy)], marca)
    L.bisel(marca, rmp)
    L.px(int(cx) - 1, int(cy) - 1, BLANCO)


def registro_reposo():
    L = Lienzo(N)
    rosa = r('registro')
    L.rect(6, 4, 19, 20, 'pag')
    L.bisel('pag', CREMA)
    L.rect(4, 3, 17, 20, 'tap')
    L.bisel('tap', rosa)
    L.rect(4, 3, 5, 20, r('registro', 0.4)[1])
    for x, y in ((16, 3), (17, 4), (16, 20), (17, 19)):
        L.px(x, y, ORO[1])
    _gema(L, 11, 11, 'gem', ((90, 180, 210), (150, 230, 245), (230, 255, 255)))
    return L


def registro_activo():
    L = Lienzo(N)
    rosa = r('registro')
    L.poli([(2, 9), (12, 11), (22, 9), (22, 21), (12, 22), (2, 21)], 'tap')
    L.bisel('tap', rosa)
    L.poli([(3, 8), (11.5, 10), (11.5, 20), (3, 19)], 'izq')
    L.bisel('izq', CREMA)
    L.poli([(12.5, 10), (21, 8), (21, 19), (12.5, 20)], 'der')
    L.bisel('der', CREMA)
    for y in (12, 14, 16):
        L.linea(5, y - 1, 9, y, CREMA[0])
        L.linea(14, y, 19, y - 1, CREMA[0])
    _gema(L, 12, 4.5, 'gem', ((90, 180, 210), (150, 230, 245), (230, 255, 255)))
    destello(L, 5, 3, rosa[2])
    destello(L, 19, 3, rosa[2])
    return L


ORDEN_DOCK = [
    'equipo_reposo', 'equipo_activo', 'objetos_reposo', 'objetos_activo', 'mapa_reposo', 'mapa_activo',
    'tienda_reposo', 'tienda_activo', 'caja_reposo', 'caja_activo', 'pvp_reposo', 'pvp_activo',
    'misiones_reposo', 'misiones_activo', 'registro_reposo', 'registro_activo',
]


# ------------------------------------------------------------- avisos 16x16

def aviso_registro():
    L = Lienzo(16)
    rosa = rampa((240, 120, 180))
    L.poli([(4.5, 3), (11.5, 3), (14.5, 6.5), (8, 14), (1.5, 6.5)], 'gem')
    L.bisel('gem', rosa)
    L.linea(2, 6, 13, 6, rosa[0])
    L.linea(5, 6, 8, 12, rosa[2])
    L.px(5, 4, BLANCO)
    L.px(6, 4, BLANCO)
    return L


def aviso_prisma():
    L = Lienzo(16)
    oro = rampa((240, 200, 50))
    L.poli([(8, 0.8), (9.6, 6.4), (15.2, 8), (9.6, 9.6), (8, 15.2), (6.4, 9.6), (0.8, 8), (6.4, 6.4)], 'des')
    L.bisel('des', oro)
    L.rect(7, 7, 8, 8, BLANCO)
    return L


def aviso_exp():
    L = Lienzo(16)
    azul = rampa((90, 170, 255))
    L.poli([(8, 1.5), (14.5, 8), (10.5, 8), (10.5, 14.5), (5.5, 14.5), (5.5, 8), (1.5, 8)], 'fle')
    L.bisel('fle', azul)
    return L


def aviso_clima():
    L = Lienzo(16)
    tq = rampa((80, 210, 200))
    for cx, cy, rr in ((5.2, 8.5, 3.0), (9, 6.2, 3.8), (12, 9, 2.6)):
        L.circulo(cx, cy, rr, 'nub')
    L.rect(4, 8, 12, 11, 'nub')
    L.bisel('nub', tq)
    for x, y in ((5, 13), (8, 14), (11, 13)):
        L.px(x, y, tq[1])
        if y + 1 < 15:
            L.px(x, y + 1, tq[0])
    return L


def aviso_premio():
    L = Lienzo(16)
    oro = rampa((240, 200, 50))
    pts = []
    for k in range(10):
        a = math.radians(-90 + k * 36)
        rr = 7.0 if k % 2 == 0 else 2.9
        pts.append((8 + rr * math.cos(a), 8.6 + rr * math.sin(a)))
    L.poli(pts, 'est')
    L.bisel('est', oro)
    L.px(7, 6, BLANCO)
    return L


ORDEN_AVISOS = ['aviso_registro', 'aviso_prisma', 'aviso_exp', 'aviso_clima', 'aviso_premio']
