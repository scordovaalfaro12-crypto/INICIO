"""Iconos de objeto, 24x24. El nombre de cada función es el ID del objeto."""
import math

from pixel import BLANCO, Lienzo, mezcla, rampa

N = 24
VIDRIO = ((92, 132, 170), (150, 198, 222), (212, 240, 250))
METAL = ((96, 104, 122), (156, 166, 182), (216, 224, 234))
ORO = ((168, 104, 26), (230, 170, 46), (255, 222, 112))
COBRE = ((138, 64, 34), (198, 108, 56), (240, 160, 100))
MADERA = ((98, 60, 36), (146, 94, 56), (190, 132, 82))
CREMA = ((196, 180, 160), (236, 228, 214), (252, 248, 240))

TIPO = {
    'magma': (230, 70, 30), 'fluido': (40, 140, 240), 'flora': (80, 190, 60), 'voltio': (240, 210, 40),
    'acero': (150, 160, 170), 'sonico': (170, 70, 220), 'cristal': (100, 230, 210), 'neutro': (200, 190, 180),
}


def patron(L, x0, y0, filas, colores):
    for j, fila in enumerate(filas):
        for i, ch in enumerate(fila):
            if ch in colores:
                L.px(x0 + i, y0 + j, colores[ch])


def destello(L, x, y, c=BLANCO, grande=False):
    L.px(x, y, c)
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        L.px(x + dx, y + dy, c)
    if grande:
        for dx, dy in ((2, 0), (-2, 0), (0, 2), (0, -2)):
            L.px(x + dx, y + dy, c)


def nube(L, puntos, rmp):
    for cx, cy, r in puntos:
        L.circulo(cx, cy, r, 'nube')
    L.bisel('nube', rmp)


def frasco_spray(L, x0, y0, x1, y1, liquido, bomba, cuello=2):
    """Frasco rectangular de vidrio con líquido y atomizador; (x0,y0)-(x1,y1) es el cuerpo."""
    L.rect(x0, y0, x1, y1, 'vid')
    for x, y in ((x0, y0), (x1, y0), (x0, y1), (x1, y1)):
        L.px(x, y, None)
    L.bisel('vid', VIDRIO)
    L.rect(x0 + 1, y0 + 3, x1 - 1, y1 - 1, 'liq')
    L.bisel('liq', liquido)
    L.px(x0 + 1, y0 + 1, BLANCO)
    L.px(x0 + 1, y0 + 2, BLANCO)
    cx = (x0 + x1) // 2
    L.rect(cx - cuello + 1, y0 - 2, cx + cuello - 1, y0 - 1, 'cue')
    L.bisel('cue', METAL)
    L.rect(cx - cuello, y0 - 5, cx + cuello, y0 - 3, 'bom')
    L.rect(cx - cuello + 1, y0 - 6, cx + cuello - 1, y0 - 6, 'bom')
    L.rect(cx + cuello + 1, y0 - 4, cx + cuello + 2, y0 - 4, 'bom')
    L.bisel('bom', bomba)


# ------------------------------------------------------------- repelentes

def repelente_etereo():
    L = Lienzo(N)
    nube(L, [(18.5, 7.5, 2.4), (20.8, 5.2, 1.8), (21, 9.6, 1.6)], ((150, 132, 232), (198, 186, 255), (240, 236, 255)))
    frasco_spray(L, 6, 12, 13, 21, ((52, 140, 196), (100, 196, 236), (170, 232, 252)), METAL)
    return L


def repelente_fuerte():
    L = Lienzo(N)
    nube(L, [(17.6, 7.4, 3.0), (20.6, 4.4, 2.2), (20.8, 10.4, 2.4), (16.8, 12.6, 1.4)],
         ((150, 50, 170), (214, 96, 226), (246, 176, 250)))
    frasco_spray(L, 3, 10, 13, 21, ((88, 34, 150), (140, 64, 214), (196, 140, 250)), ORO)
    L.rect(4, 15, 12, 16, 'eti')
    L.bisel('eti', ORO)
    return L


# ------------------------------------------------------------- premio del Registro

def amuleto_prismatico():
    L = Lienzo(N)
    for a in range(0, 181, 9):
        t = math.radians(a)
        x, y = 12 + 7.6 * math.cos(t), 2.5 + 7.2 * math.sin(t)
        if (a // 9) % 2 == 0:
            L.px(int(x), int(y), ORO[1])
    L.anillo(12, 10.5, 0.6, 1.9, 'asa')
    L.bisel('asa', ORO)
    L.circulo(12, 16, 5.9, 'bis')
    L.bisel('bis', ORO)
    colores = [(250, 92, 104), (255, 172, 62), (250, 232, 92), (112, 222, 122), (92, 178, 255), (178, 118, 246)]
    L.circulo(12, 16, 4.4, 'gema')
    for (x, y), c in list(L.p.items()):
        if c == 'gema':
            ang = (math.degrees(math.atan2(y + 0.5 - 16, x + 0.5 - 12)) + 360 + 15) % 360
            L.p[(x, y)] = colores[int(ang // 60)]
    for x, y in ((12, 16), (11, 15), (12, 15), (11, 16)):
        L.px(x, y, BLANCO)
    L.px(10, 14, (255, 250, 230))
    destello(L, 19, 11)
    return L

def brasa_viva():
    L = Lienzo(N)
    for x, y in ((10, 3), (11, 2), (13, 4), (14, 3), (12, 5), (11, 4)):
        L.px(x, y, (255, 200, 80))
    L.px(12, 3, (255, 236, 150))
    L.poli([(5, 13), (8, 8), (13, 6.5), (18, 9), (20, 14), (17, 19.5), (11, 21), (6, 18.5)], 'roca')
    L.bisel('roca', rampa(TIPO['magma']))
    # núcleo incandescente y grietas
    L.elipse(12.5, 14, 3.6, 3.2, (255, 214, 92))
    L.elipse(12, 13.6, 1.8, 1.5, (255, 246, 196))
    for a, b in (((8, 11), (10, 13)), ((16, 10), (15, 12)), ((9, 18), (11, 17)), ((17, 16), (16, 17))):
        L.linea(*a, *b, (255, 180, 70))
    for x, y in ((4, 9), (20, 6), (21, 11)):
        L.px(x, y, (255, 170, 60))
    return L


def perla_marea():
    L = Lienzo(N)
    azul = rampa(TIPO['fluido'])
    pts = []
    for k in range(13):
        a = math.radians(180 + k * 15)
        r = 9.4 if k % 2 == 0 else 8.3
        pts.append((12 + r * math.cos(a), 13.5 + r * math.sin(a)))
    pts += [(18, 17), (12, 19.5), (6, 17)]
    L.poli(pts, 'con')
    L.bisel('con', azul)
    for k in range(1, 6):
        a = math.radians(180 + k * 30)
        L.linea(12, 17, round(12 + 7.6 * math.cos(a)), round(13.5 + 7.6 * math.sin(a)), azul[0])
    L.circulo(12, 15.2, 3.5, 'per')
    L.bisel('per', CREMA)
    L.px(11, 13, BLANCO)
    L.px(10, 14, BLANCO)
    L.elipse(12, 18.6, 8.4, 2.9, 'lab', dentro=lambda x, y: y >= 17)
    L.bisel('lab', azul)
    L.linea(5, 18, 18, 18, azul[2])
    return L

def semilla_antigua():
    L = Lienzo(N)
    verde = rampa(TIPO['flora'])
    L.linea(12, 11, 12, 6, verde[0])
    L.linea(11, 11, 11, 7, verde[1])
    L.elipse(8.2, 6.2, 3.4, 1.8, 'hoja1')
    L.bisel('hoja1', verde)
    L.elipse(15.6, 5.2, 3.6, 1.9, 'hoja2')
    L.bisel('hoja2', verde)
    L.elipse(12, 16, 6.0, 5.6, 'sem')
    L.bisel('sem', ((92, 58, 34), (150, 104, 62), (204, 156, 100)))
    # marcas antiguas: espiral clara
    for x, y in ((12, 13), (13, 13), (14, 14), (14, 15), (13, 16), (12, 16), (11, 15), (10, 14), (10, 17), (11, 18), (13, 18), (15, 17)):
        L.px(x, y, (226, 190, 128))
    return L


def bobina_cargada():
    L = Lienzo(N)
    L.rect(4, 6, 15, 7, 'ala1')
    L.bisel('ala1', METAL)
    L.rect(4, 19, 15, 20, 'ala2')
    L.bisel('ala2', METAL)
    L.rect(6, 8, 13, 18, 'cob')
    L.bisel('cob', COBRE)
    for y in range(9, 18, 2):
        L.linea(7, y, 12, y, COBRE[0])
    L.linea(7, 8, 12, 8, COBRE[2])
    amarillo = rampa(TIPO['voltio'])
    rayo = [(19, 2), (22, 2), (19.5, 7), (22, 7), (16.5, 14), (18, 9), (16, 9)]
    L.poli(rayo, 'ray')
    L.bisel('ray', amarillo)
    L.px(19, 6, BLANCO)
    L.px(19, 7, BLANCO)
    return L


def engranaje_pulido():
    L = Lienzo(N)
    acero = rampa(TIPO['acero'])
    L.circulo(12, 12, 6.9, 'eng')
    for x0, y0, x1, y1 in ((10, 3, 13, 5), (10, 19, 13, 21), (3, 10, 5, 13), (19, 10, 21, 13),
                           (4, 4, 6, 6), (17, 4, 19, 6), (4, 17, 6, 19), (17, 17, 19, 19)):
        L.rect(x0, y0, x1, y1, 'eng')
    L.bisel('eng', acero)
    L.anillo(12, 12, 3.4, 4.2, acero[0])
    L.circulo(12, 12, 2.2, None)
    for x, y in ((8, 7), (7, 8), (9, 7)):
        L.px(x, y, BLANCO)
    destello(L, 21, 2)
    return L

def diapason():
    L = Lienzo(N)
    violeta = rampa(TIPO['sonico'])
    for r, tono in ((8.6, violeta[1]), (10.6, violeta[2])):
        for a in range(-26, 27, 2):
            t = math.radians(a)
            L.px(int(12 + r * math.cos(t)), int(6.5 + r * math.sin(t)), tono)
            L.px(int(12 - r * math.cos(t)), int(6.5 + r * math.sin(t)), tono)
    L.rect(7, 2, 8, 11, 'dia')
    L.rect(15, 2, 16, 11, 'dia')
    L.elipse(12, 11, 5.0, 4.6, 'dia', dentro=lambda x, y: y >= 11)
    L.elipse(12, 11, 3.0, 2.6, None, dentro=lambda x, y: y >= 11)
    L.rect(11, 15, 12, 19, 'dia')
    L.circulo(12, 20.5, 1.8, 'dia')
    L.bisel('dia', METAL)
    return L

def pluma_lisa():
    L = Lienzo(N)
    gris = ((168, 160, 154), (228, 224, 218), (252, 252, 250))
    B, T = (4.5, 20.5), (20.5, 3.0)
    lx, ly = T[0] - B[0], T[1] - B[1]
    largo = (lx * lx + ly * ly) ** 0.5
    d = (lx / largo, ly / largo)
    nrm = (d[1], -d[0])
    izq, der = [], []
    for k in range(0, 21):
        t = 0.18 + 0.82 * k / 20
        w = 3.6 * math.sin(math.pi * min(1, (t - 0.18) / 0.8)) ** 0.75
        cx, cy = B[0] + d[0] * largo * t, B[1] + d[1] * largo * t
        izq.append((cx - nrm[0] * w * 1.1, cy - nrm[1] * w * 1.1))
        der.append((cx + nrm[0] * w * 0.85, cy + nrm[1] * w * 0.85))
    L.poli(izq + der[::-1], 'plu')
    # muescas de las barbas
    for t, lado in ((0.42, -1), (0.62, 1), (0.74, -1)):
        cx, cy = B[0] + d[0] * largo * t, B[1] + d[1] * largo * t
        for k in range(2, 5):
            L.px(round(cx + lado * nrm[0] * k - d[0] * k * 0.6), round(cy + lado * nrm[1] * k - d[1] * k * 0.6), None)
    L.bisel('plu', gris)
    L.linea(6, 19, 18, 6, (186, 178, 170))
    L.linea(6, 19, 3, 22, (130, 120, 112))
    return L

def prisma_puro():
    """Obelisco fino e inclinado, transparente: no se parece a un prisma de captura."""
    L = Lienzo(N)
    agua = rampa(TIPO['cristal'])
    L.poli([(14, 1.5), (17.5, 6), (15.5, 20), (11, 22.5), (8.5, 19), (10.5, 5)], 'cri')
    L.bisel('cri', ((60, 170, 160), (150, 242, 230), (220, 255, 250)))
    # arista interna y reflejos: se ve "a través"
    L.linea(13, 3, 12, 20, (110, 214, 200))
    L.linea(11, 7, 11, 12, BLANCO)
    L.px(15, 8, BLANCO)
    L.px(15, 9, (220, 255, 250))
    for x, y in ((5, 6), (20, 13)):
        destello(L, x, y, agua[2])
    return L


def restos_etereos():
    L = Lienzo(N)
    eter = ((112, 80, 200), (166, 132, 246), (214, 196, 255))
    L.circulo(10.5, 14, 7.8, 'fru')
    L.bisel('fru', eter)
    mordidas = ((18.4, 10.6), (19.4, 15.6))
    for (x, y) in list(L.p):
        for cx, cy in mordidas:
            dd = (x + 0.5 - cx) ** 2 + (y + 0.5 - cy) ** 2
            if dd < 3.4 ** 2:
                L.p.pop((x, y), None)
            elif dd < 4.5 ** 2 and (x, y) in L.p:
                L.p[(x, y)] = (252, 238, 214)
    L.linea(10, 7, 11, 4, MADERA[0])
    L.elipse(13.8, 4.2, 2.4, 1.3, 'hoj')
    L.bisel('hoj', rampa(TIPO['flora']))
    L.px(6, 10, BLANCO)
    L.px(7, 9, BLANCO)
    L.px(6, 11, eter[2])
    for x, y in ((3, 4), (20, 21), (19, 5)):
        destello(L, x, y, (200, 255, 250))
    return L

def banda_focal():
    L = Lienzo(N)
    rojo = ((150, 30, 40), (222, 62, 62), (252, 134, 124))
    L.elipse(11, 11.5, 8.8, 5.6, 'atr', dentro=lambda x, y: y < 11)
    L.elipse(11, 11.5, 6.8, 3.6, None, dentro=lambda x, y: y < 11)
    L.bisel('atr', ((110, 22, 34), (168, 40, 48), (200, 70, 72)))
    L.elipse(11, 11.5, 8.8, 5.6, 'fre', dentro=lambda x, y: y >= 11)
    L.elipse(11, 10.5, 6.8, 3.2, None, dentro=lambda x, y: y >= 11)
    L.bisel('fre', rojo)
    L.poli([(16.5, 15), (21.5, 20.5), (19, 22), (15, 17)], 'col')
    L.poli([(18, 14), (22.5, 16.2), (21.6, 18.4), (17, 16)], 'col')
    L.bisel('col', rojo)
    L.rect(16, 14, 18, 16, 'nud')
    L.bisel('nud', rojo)
    return L

def cascabel_fortuna():
    L = Lienzo(N)
    L.anillo(12, 5.5, 1.2, 2.6, 'aro')
    L.bisel('aro', ORO)
    L.circulo(12, 14, 7.6, 'cas')
    L.bisel('cas', ORO)
    L.linea(6, 15, 18, 15, ORO[0])
    L.rect(11, 15, 12, 19, ORO[0])
    L.circulo(12, 18.5, 1.4, (60, 40, 30))
    L.px(9, 10, BLANCO)
    L.px(8, 11, BLANCO)
    L.px(9, 11, ORO[2])
    destello(L, 19, 5)
    destello(L, 4, 19)
    return L


# ------------------------------------------------------------- discos de movimiento

MARCAS = {
    'impacto': ["...X...", ".X.X.X.", "..XXX..", "XXXXXXX", "..XXX..", ".X.X.X.", "...X..."],
    'escudo': ["XXXXXXX", "X.XXXXX", "X.XXXXX", ".XXXXX.", ".XXXXX.", "..XXX..", "...X..."],
    'rapidez': ["X..X...", ".X..X..", "..X..X.", "...X..X", "..X..X.", ".X..X..", "X..X..."],
}


def disco(color, marca=None):
    L = Lienzo(N)
    r = rampa(color)
    L.circulo(12, 12, 10.2, 'dis')
    L.bisel('dis', r)
    L.anillo(12, 12, 7.1, 7.9, mezcla(r[1], r[0], 0.6))
    # reflejo en arco arriba a la izquierda
    for a in range(200, 256, 6):
        t = math.radians(a)
        L.px(int(12 + 8.9 * math.cos(t)), int(12 + 8.9 * math.sin(t)), mezcla(r[2], BLANCO, 0.6))
    L.anillo(12, 12, 5.0, 6.0, r[0])
    L.circulo(12, 12, 5.0, 'eti')
    L.bisel('eti', CREMA)
    if marca is None:
        L.rect(11, 11, 12, 12, None)
    else:
        filas = MARCAS[marca]
        w, h = len(filas[0]), len(filas)
        patron(L, 12 - w // 2, 12 - h // 2, filas, {'X': (92, 84, 78)})
    return L


def disco_lava_torrencial(): return disco(TIPO['magma'])
def disco_canon_hidro(): return disco(TIPO['fluido'])
def disco_drenaje_raiz(): return disco(TIPO['flora'])
def disco_descarga_masiva(): return disco(TIPO['voltio'])
def disco_embestida_blindada(): return disco(TIPO['acero'])
def disco_aullido_atronador(): return disco(TIPO['sonico'])
def disco_lanza_cristal(): return disco(TIPO['cristal'])
def disco_embestida(): return disco(TIPO['neutro'], 'impacto')
def disco_proteccion(): return disco(TIPO['neutro'], 'escudo')
def disco_agilidad(): return disco(TIPO['neutro'], 'rapidez')


# ------------------------------------------------------------- opcional

def huevo_nido():
    L = Lienzo(N)
    for x0, y0, x1, y1 in ((3, 19, 20, 21), (5, 18, 18, 18)):
        L.rect(x0, y0, x1, y1, 'nid')
    L.bisel('nid', MADERA)
    for x in range(4, 21, 3):
        L.px(x, 20, MADERA[2])
    L.elipse(12, 12, 6.2, 8.4, 'hue', dentro=lambda x, y: True)
    for y in range(3, 9):
        ancho = 6.2 * (0.55 + 0.45 * (y - 3) / 6)
        for x in range(24):
            if abs(x + 0.5 - 12) > ancho and L.p.get((x, y)) == 'hue':
                L.p.pop((x, y))
    L.bisel('hue', CREMA)
    for x, y in ((10, 7), (14, 10), (9, 13), (13, 15), (16, 14), (11, 17), (15, 6)):
        L.px(x, y, (70, 170, 160))
    L.px(9, 6, BLANCO)
    L.px(8, 7, BLANCO)
    return L


ORDEN = [
    'repelente_etereo', 'repelente_fuerte', 'amuleto_prismatico',
    'brasa_viva', 'perla_marea', 'semilla_antigua', 'bobina_cargada', 'engranaje_pulido', 'diapason',
    'pluma_lisa', 'prisma_puro', 'restos_etereos', 'banda_focal', 'cascabel_fortuna',
    'disco_lava_torrencial', 'disco_canon_hidro', 'disco_drenaje_raiz', 'disco_descarga_masiva',
    'disco_embestida_blindada', 'disco_aullido_atronador', 'disco_lanza_cristal',
    'disco_embestida', 'disco_proteccion', 'disco_agilidad',
    'huevo_nido',
]
