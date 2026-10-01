"""Basaltor — evolucion final de Vulcanid (Aetherials).
Construye la criatura por partes con metodologia de Blender:
metaballs -> remesh voxel -> desplazamiento de roca -> decimate (low poly) -> simetria,
mas placas, columnas de basalto en panal, garras, dientes y cuernos generados con bmesh.
Ejecutar: python3 build.py [salida.blend]
"""
import bpy, bmesh, math, random, sys, os
WORK = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, WORK)
from mathutils import Vector, Matrix, Euler, Quaternion
from lib import *
from lib import _bridge, _ring, _cap

OUT = sys.argv[-1] if sys.argv[-1].endswith('.blend') else os.path.join(WORK, 'basaltor.blend')

bpy.ops.wm.read_factory_settings(use_empty=True)
MATS = build_materials()
ROOT_COL = collection('Basaltor')
COLS = {k: collection(k, ROOT_COL) for k in
        ('Cuerpo', 'Cabeza', 'Patas_Delanteras', 'Patas_Traseras', 'Cola')}

K = 1 / 0.575  # radio de metaball -> radio de superficie
L = 0.16       # elevacion del cuerpo respecto a Vulcanid (patas mas largas)


def E(co, sx, sy, sz, rot=None, stiff=2.0, neg=False):
    return ellipsoid(co, K, sx, sy, sz, rot, stiff, neg)


def B(co, R, stiff=2.0, neg=False):
    return ball(co, R * K, stiff, neg)


def C(a, b, R, stiff=2.0, neg=False):
    return capsule(a, b, R * K, stiff, neg)


def part(name, coll, elems, tris, voxel=0.02, disp=None, res=0.02, sym=True):
    me = metaball_mesh(name, elems, res=res)
    o = new_object(name, me, coll, MATS)
    lowpoly(o, tris, voxel=voxel, disp=disp, sym=sym)
    set_material(o, MI['Cuerpo'])
    return o


def set_pivot(o, pivot):
    pv = Vector(pivot)
    o.data.transform(Matrix.Translation(-pv))
    o.location = pv


def radial_hit(bvh, y, theta, zc=1.0, side=1):
    """Rayo hacia el eje del cuerpo. theta: 0=arriba, 90=costado, 180=abajo."""
    t = math.radians(theta)
    d = Vector((side * math.sin(t), 0, math.cos(t)))
    c = Vector((0, y, zc))
    return surface_hit(bvh, c + d * 4, c)


def belly_paint(c, n):
    if n.z < -0.55:
        return MI['VientreBanda'] if int((c.y + 3) / 0.17) % 2 else MI['Vientre']
    if n.z < -0.15:
        return MI['CuerpoOscuro']
    return None


# ---------------------------------------------------------------- basalto en panal
def rift(bm, bvh, y0, y1, w=0.06, h=0.035, step=0.05, mat=MI['MagmaCaliente']):
    """Grieta de magma: prisma triangular sobre la linea media."""
    prev = None
    n = max(2, int((y1 - y0) / step))
    for i in range(n + 1):
        y = y0 + (y1 - y0) * i / n
        loc, nor = surface_hit(bvh, (0, y, 6), (0, y, -1))
        if loc is None:
            prev = None
            continue
        hh = h * (0.8 + 0.4 * abs(math.sin(i * 1.7)))
        Lv = bm.verts.new((-w, y, loc.z - 0.03))
        Cv = bm.verts.new((0, y, loc.z + hh))
        Rv = bm.verts.new((w, y, loc.z - 0.03))
        if prev:
            pL, pC, pR = prev
            bm.faces.new((pL, Lv, Cv, pC)).material_index = mat
            bm.faces.new((pC, Cv, Rv, pR)).material_index = mat
        prev = (Lv, Cv, Rv)


def basalt_field(bm, bvh, height_fn, y0, y1, x_min, x_max, R, rng, gap=0.014, lean=0.35,
                 min_h=0.035, steps=0.035, slant=(0.1, 0.45), col_rim=MI['Magma']):
    """Columnas hexagonales empacadas en panal (como la Calzada del Gigante) sobre la superficie.
    Solo lado +X; luego se simetriza."""
    dx = math.sqrt(3) * R + gap
    dy = 1.5 * R + gap * 0.87
    j = 0
    y = y0
    count = 0
    while y <= y1:
        xoff = dx / 2 if j % 2 else 0.0
        x = x_min + R * 0.866 + xoff
        while x <= x_max:
            h = height_fn(x, y)
            if h >= min_h:
                h = round(h * rng.uniform(0.82, 1.12) / steps) * steps  # alturas escalonadas
                loc, nor = surface_hit(bvh, (x, y, 6), (x, y, -1))
                if loc is not None and h > 0:
                    up = Vector((0, 0, 1))
                    axis = (nor * 0.55 + up * 0.45 + Vector((x * lean, 0, 0))).normalized()
                    M = align_matrix(loc - axis * 0.02, axis, 0.0, (R, R, R))
                    add_hex_column(bm, M, rng, height=h / R, slant=rng.uniform(*slant), phase=math.pi / 6,
                                   rim_w=0.10, chamfer=0.16, rim=col_rim, rim_h=0.06, depth=1.6)
                    count += 1
            x += dx
        y += dy
        j += 1
    return count


def bump(x, y, cx, cy, sx, sy, peak):
    d = ((x - cx) / sx) ** 2 + ((y - cy) / sy) ** 2
    return peak * max(0.0, 1 - d) ** 0.8


# =========================================================================== TORSO
torso = part('Torso', COLS['Cuerpo'], [
    E((0, -0.42, 1.02 + L), 0.60, 0.56, 0.50),
    E((0, 0.12, 1.02 + L), 0.64, 0.62, 0.52),
    E((0, 0.68, 0.99 + L), 0.54, 0.50, 0.46),
    B((0.44, -0.50, 1.00 + L), 0.34), B((-0.44, -0.50, 1.00 + L), 0.34),
    B((0.40, 0.74, 0.98 + L), 0.32), B((-0.40, 0.74, 0.98 + L), 0.32),
    E((0, 0.05, 0.80 + L), 0.50, 0.72, 0.30),
    E((0, -0.86, 1.06 + L), 0.38, 0.30, 0.36),
    E((0, 1.08, 0.97 + L), 0.36, 0.30, 0.34),
], tris=1500, voxel=0.03, disp=dict(scale=2.0, strength=0.04, cells=0.03, cell_scale=3.0, seed=1))
paint_faces(torso, belly_paint)
TORSO_BVH = bvh_of(torso)

# --- Lomo: grieta de magma con orillas de columnas de basalto
lomo = new_object('Lomo', bpy.data.meshes.new('Lomo'), COLS['Cuerpo'], MATS)
bm = bm_from(lomo)
rift(bm, TORSO_BVH, -0.92, 1.18, w=0.07)


def back_h(x, y):
    h = max(bump(x, y, 0.10, -0.32, 0.40, 0.44, 0.62),   # cresta de los hombros
            bump(x, y, 0.10, 0.60, 0.36, 0.36, 0.48),    # cresta de la cadera
            bump(x, y, 0.06, 0.15, 0.24, 0.30, 0.20),    # puente
            bump(x, y, 0.06, -0.80, 0.18, 0.20, 0.14),   # base del cuello
            bump(x, y, 0.06, 1.02, 0.18, 0.20, 0.14))    # base de la cola
    return h


n = basalt_field(bm, TORSO_BVH, back_h, -0.95, 1.15, 0.085, 0.55, 0.085, random.Random(7))
print('columnas lomo', n)
bm_commit(lomo, bm)
symmetrize(lomo)
flat(lomo)

# --- placas de los costados (herencia de Vulcanid) + pecho
bm = bm_from(torso)
rng = random.Random(11)
placed = []
tries = 0
while len(placed) < 15 and tries < 3000:
    tries += 1
    y = rng.uniform(-0.80, 1.08)
    th = rng.uniform(55, 110)
    if (abs(y + 0.50) < 0.30 and th > 60) or (abs(y - 0.74) < 0.28 and th > 60):
        continue  # hombros/caderas: las cubren las hombreras de las patas
    r = rng.uniform(0.15, 0.22)
    loc, nor = radial_hit(TORSO_BVH, y, th, zc=0.98 + L)
    if loc is None or back_h(loc.x, loc.y) > 0.03:
        continue
    if any((loc - p).length < (r + pr) * 0.95 for p, pr in placed):
        continue
    placed.append((loc, r))
    M = align_matrix(loc - nor * 0.012, nor, rng.uniform(0, 6.28), (r, r, r))
    add_dome_plate(bm, M, rng, sides=rng.choice((6, 7, 7, 8)), height=rng.uniform(0.42, 0.62),
                   mat=MI['Placa'] if rng.random() < 0.7 else MI['PlacaClara'])
for (x, y, z, r, rot) in ((0.0, -0.98, 0.80 + L, 0.21, 0.0), (0.21, -0.93, 0.66 + L, 0.15, 0.5)):
    for sx in ((1, -1) if x else (1,)):
        loc, nor = surface_hit(TORSO_BVH, (sx * x * 1.3, y - 1.5, z), (sx * x * 0.2, y + 0.6, z + 0.1))
        if loc is None:
            continue
        M = align_matrix(loc - nor * 0.01, nor, rot, (r, r, r))
        add_dome_plate(bm, M, rng, sides=7, height=0.35, mat=MI['Placa'], rim_w=0.07)
bm_commit(torso, bm)
symmetrize(torso)
flat(torso)
set_pivot(torso, (0, 0.1, 1.0 + L))
set_pivot(lomo, (0, 0.1, 1.0 + L))

# =========================================================================== CUELLO
cuello = part('Cuello', COLS['Cuerpo'], [
    C((0, -0.72, 1.04 + L), (0, -1.10, 1.02 + L), 0.32),
    E((0, -0.92, 1.18 + L), 0.28, 0.26, 0.16),
    E((0, -0.95, 0.86 + L), 0.26, 0.28, 0.16),
], tris=450, voxel=0.022, disp=dict(scale=3, strength=0.025, cells=0.02, cell_scale=4, seed=2))
paint_faces(cuello, lambda c, n: MI['Vientre'] if n.z < -0.5 else None)
NECK_BVH = bvh_of(cuello)
bm = bm_from(cuello)
rift(bm, NECK_BVH, -1.12, -0.70, w=0.06)
basalt_field(bm, NECK_BVH, lambda x, y: bump(x, y, 0.06, -0.86, 0.22, 0.22, 0.20), -1.08, -0.70,
             0.075, 0.35, 0.07, random.Random(21))
bm_commit(cuello, bm)
symmetrize(cuello)
flat(cuello)
set_pivot(cuello, (0, -0.74, 1.05 + L))

# =========================================================================== CABEZA
HZ = L  # la cabeza sube con el cuerpo
craneo = part('Craneo', COLS['Cabeza'], [
    E((0, -1.28, 1.12 + HZ), 0.42, 0.36, 0.32),
    E((0, -1.58, 1.04 + HZ), 0.38, 0.28, 0.22),
    E((0, -1.80, 1.00 + HZ), 0.30, 0.12, 0.17),
    B((0.32, -1.24, 1.00 + HZ), 0.23), B((-0.32, -1.24, 1.00 + HZ), 0.23),
    B((0, -1.08, 1.08 + HZ), 0.28),
    E((0.22, -1.42, 1.22 + HZ), 0.15, 0.14, 0.08), E((-0.22, -1.42, 1.22 + HZ), 0.15, 0.14, 0.08),
], tris=1300, voxel=0.018, res=0.016,
    disp=dict(scale=3.0, strength=0.015, cells=0.015, cell_scale=5, seed=3))
MOUTH_Z = 0.875 + HZ
MOUTH_TILT = 0.04


def mouth_z(y):
    """altura del corte de la boca (el plano esta levemente inclinado hacia el hocico)."""
    return MOUTH_Z - MOUTH_TILT * y


bisect(craneo, (0, 0, MOUTH_Z), (0, MOUTH_TILT, 1), keep_positive=True, fill=True, mat=MI['MagmaProfundo'])
symmetrize(craneo)


def head_paint(c, n):
    if c.z < mouth_z(c.y) + 0.01 and n.z < -0.9:
        return MI['MagmaProfundo']  # paladar
    if c.y < -1.80 and n.y < -0.25 and c.z < 1.08 + HZ:
        return MI['Hocico']
    if n.z > 0.40 and c.y < -1.46 and abs(c.x) < 0.27:
        return MI['CabezaGris']
    if n.z < -0.3:
        return MI['CuerpoOscuro']
    return MI['Cuerpo']


paint_faces(craneo, head_paint)
flat(craneo)
HEAD_BVH = bvh_of(craneo)

# --- ojos (lente hexagonal crema, pupila vertical, brillo) — igual que Vulcanid
ojos = new_object('Ojos', bpy.data.meshes.new('Ojos'), COLS['Cabeza'], MATS)
bm = bm_from(ojos)
eye_loc, eye_nor = surface_hit(HEAD_BVH, (1.15, -2.25, 1.30 + HZ), (0.0, -1.42, 1.04 + HZ))
print('ojo', eye_loc, eye_nor)


def add_eye(bm, loc, nor, scale=1.0):
    z = nor.normalized()
    fwd = Vector((0, -1, 0))
    x = (fwd - z * fwd.dot(z)).normalized()  # eje horizontal hacia el hocico
    y = z.cross(x).normalized()
    if y.z < 0:
        y = -y
    Mrot = Matrix((x, y, z)).transposed().to_4x4()
    M = Matrix.Translation(loc + z * 0.004) @ Mrot @ Matrix.Diagonal((scale, scale, scale, 1))
    hexpts = [(-0.085, 0.0), (-0.05, 0.035), (0.055, 0.042), (0.09, 0.0), (0.055, -0.036), (-0.05, -0.03)]
    front = [bm.verts.new((px, py, 0.014)) for px, py in hexpts]
    back = [bm.verts.new((px * 1.22, py * 1.32, -0.05)) for px, py in hexpts]
    newv = front + back
    f = bm.faces.new(front); f.material_index = MI['Ojo']
    fs = [f] + _bridge(bm, back, front, MI['Pupila'])
    pw, ph = 0.011, 0.034
    pv = [bm.verts.new(p) for p in ((-pw, -ph, 0.017), (pw, -ph, 0.017), (pw, ph, 0.017), (-pw, ph, 0.017))]
    bm.faces.new(pv).material_index = MI['Pupila']
    gv = [bm.verts.new(p) for p in ((-0.036, 0.012, 0.0185), (-0.022, 0.012, 0.0185),
                                    (-0.022, 0.026, 0.0185), (-0.036, 0.026, 0.0185))]
    bm.faces.new(gv).material_index = MI['Brillo']
    newv += pv + gv
    bmesh.ops.transform(bm, matrix=M, verts=newv)
    bmesh.ops.recalc_face_normals(bm, faces=fs)
    return x, y, z


ES = 1.45
ex, ey, ez = add_eye(bm, eye_loc, eye_nor, scale=ES)
bm_commit(ojos, bm)
symmetrize(ojos)
flat(ojos)

# --- cejas de roca (expresion feroz) con brillo de magma debajo
cejas = new_object('Cejas', bpy.data.meshes.new('Cejas'), COLS['Cabeza'], MATS)
bm = bm_from(cejas)
rng = random.Random(31)
brow_target = eye_loc + ey * (0.042 * ES + 0.05) + ex * 0.0
bl, bn = surface_hit(HEAD_BVH, brow_target + ez * 0.6 + Vector((0, 0, 0.3)), brow_target - ez * 0.1)
if bl is not None:
    zax = (bn * 0.5 + ez * 0.2 + Vector((0, 0, 0.6))).normalized()
    xax = (ex - zax * ex.dot(zax)).normalized()
    # inclinar: la punta delantera de la ceja baja hacia el ojo
    xax = (xax + Vector((0, 0, -0.42))).normalized()
    yax = zax.cross(xax).normalized()
    zax = xax.cross(yax).normalized()
    Mr = Matrix((xax, yax, zax)).transposed().to_4x4()
    M = Matrix.Translation(bl + ez * 0.02) @ Mr @ Matrix.Diagonal((0.16, 0.052, 0.075, 1))
    add_dome_plate(bm, M, rng, sides=6, height=0.9, mat=MI['Placa'], rim_w=0.14, peak=0.05)
bm_commit(cejas, bm)
symmetrize(cejas)
flat(cejas)

# --- cuernos principales + puas de mejilla
cuernos = new_object('Cuernos', bpy.data.meshes.new('Cuernos'), COLS['Cabeza'], MATS)
bm = bm_from(cuernos)
horn = [Vector(p) + Vector((0, 0, HZ)) for p in
        ((0.21, -1.20, 1.30), (0.31, -1.07, 1.45), (0.42, -0.95, 1.58), (0.53, -0.86, 1.69),
         (0.62, -0.76, 1.78), (0.67, -0.64, 1.86), (0.68, -0.52, 1.92))]
hr = [0.13, 0.12, 0.105, 0.088, 0.066, 0.04, 0.01]
# remuestrear la curva con anillos: cada anillo se ensancha un poco (cuerno estriado)
from mathutils import geometry as _g
dense, drad = [], []
NS = 19
seglen = [(horn[k + 1] - horn[k]).length for k in range(len(horn) - 1)]
tot = sum(seglen)
for s in range(NS):
    t = s / (NS - 1)
    d = t * tot
    k = 0
    while k < len(seglen) - 1 and d > seglen[k]:
        d -= seglen[k]
        k += 1
    f = min(1.0, d / seglen[k])
    dense.append(horn[k].lerp(horn[k + 1], f))
    r = hr[k] + (hr[k + 1] - hr[k]) * f
    drad.append(r * (1.12 if (s % 3 == 1 and t < 0.8) else 1.0))
add_tube(bm, dense, drad, sides=6, twist=0.05,
         mat_fn=lambda t: (MI['Hueso'] if t > 0.78 else
                           (MI['PlacaClara'] if int(t * 18) % 3 == 1 else
                            (MI['Basalto'] if t < 0.5 else MI['Placa']))))
hb = horn[0]
hl, hn = surface_hit(HEAD_BVH, hb + Vector((0.1, 0.05, 0.4)), hb - Vector((0.05, 0, 0.1)))
if hl is not None:
    M = align_matrix(hl - hn * 0.02, hn, 0, (0.15, 0.15, 0.1))
    add_hex_column(bm, M, random.Random(41), height=0.6, slant=0.1, mat=MI['Basalto'],
                   top=MI['Basalto'], rim_w=0.12)
for pts, rads in ((((0.36, -1.12, 0.96), (0.46, -0.98, 1.0), (0.56, -0.84, 1.05), (0.63, -0.72, 1.12)),
                   (0.075, 0.06, 0.035, 0.008)),
                  (((0.38, -1.25, 0.93), (0.50, -1.14, 0.93), (0.60, -1.04, 0.95)), (0.055, 0.035, 0.006))):
    add_tube(bm, [Vector(p) + Vector((0, 0, HZ)) for p in pts], list(rads), sides=5,
             mat_fn=lambda t: MI['Basalto'] if t < 0.5 else MI['Hueso'])
bm_commit(cuernos, bm)
symmetrize(cuernos)
flat(cuernos)

# --- corona de basalto + grieta sobre la cabeza + fosas nasales
bm = bm_from(craneo)
rift(bm, HEAD_BVH, -1.50, -1.02, w=0.04, h=0.025)
basalt_field(bm, HEAD_BVH, lambda x, y: bump(x, y, 0.05, -1.18, 0.20, 0.20, 0.17), -1.40, -1.00,
             0.05, 0.30, 0.05, random.Random(51), lean=0.6)
nl, nn = surface_hit(HEAD_BVH, (0.12, -2.3, 1.4 + HZ), (0.10, -1.80, 0.98 + HZ))
if nl is not None:
    M = align_matrix(nl, nn, math.radians(70), (0.032, 0.015, 0.02))
    add_hex_column(bm, M, random.Random(5), height=0.25, slant=0.0, mat=MI['Hocico'],
                   top=MI['Magma'], rim=MI['Hocico'], rim_w=0.35, chamfer=0.3)
bm_commit(craneo, bm)
symmetrize(craneo)
flat(craneo)

# --- dientes superiores
dientes_sup = new_object('Dientes_Superiores', bpy.data.meshes.new('DS'), COLS['Cabeza'], MATS)
bm = bm_from(dientes_sup)
rng = random.Random(61)
for i in range(11):
    y = -1.20 - 0.065 * i
    zr = mouth_z(y) + 0.02
    loc, nor = surface_hit(HEAD_BVH, (1.5, y, zr), (0, y, zr))
    if loc is None:
        continue
    p = Vector((loc.x - 0.035, y, zr))
    big = i in (6, 7)
    Lt = 0.16 if big else rng.uniform(0.06, 0.09)
    M = Matrix.Translation(p) @ Matrix.Rotation(math.radians(-90), 4, 'X') \
        @ Matrix.Rotation(math.radians(rng.uniform(-8, 8)), 4, 'Z')
    add_claw(bm, M @ Matrix.Diagonal((Lt, Lt, Lt, 1)), rng, length=1.0, curve=0.18, base=0.26 if big else 0.3)
for x in (0.035, 0.105):
    zr = mouth_z(-1.9) + 0.02
    loc, nor = surface_hit(HEAD_BVH, (x, -2.4, zr), (x, -1.5, zr))
    if loc is not None:
        M = Matrix.Translation(loc + Vector((0, 0.035, 0))) @ Matrix.Rotation(math.radians(-90), 4, 'X')
        add_claw(bm, M @ Matrix.Diagonal((0.07, 0.07, 0.07, 1)), rng, length=1.0, curve=-0.2, base=0.3)
bm_commit(dientes_sup, bm)
symmetrize(dientes_sup)
assert len(dientes_sup.data.vertices) > 0, 'sin dientes superiores'
flat(dientes_sup)

HEAD_PIVOT = (0, -1.08, 1.02 + HZ)
for o in (craneo, ojos, cejas, cuernos, dientes_sup):
    set_pivot(o, HEAD_PIVOT)

# =========================================================================== MANDIBULA
mand = part('Mandibula', COLS['Cabeza'], [
    E((0, -1.42, 0.79 + HZ), 0.34, 0.38, 0.15),
    E((0, -1.70, 0.80 + HZ), 0.28, 0.12, 0.11),
    B((0.30, -1.12, 0.85 + HZ), 0.14), B((-0.30, -1.12, 0.85 + HZ), 0.14),
    E((0, -1.34, 0.68 + HZ), 0.24, 0.28, 0.10),
], tris=650, voxel=0.016, res=0.016, disp=dict(scale=3, strength=0.01, cells=0.01, cell_scale=5, seed=4))
JAW_TOP = 0.865 + HZ
bisect(mand, (0, 0, JAW_TOP), (0, 0, -1), keep_positive=True, fill=True, mat=MI['Hocico'])
symmetrize(mand)


def jaw_paint(c, n):
    if n.z > 0.9 and c.z > JAW_TOP - 0.01:
        return MI['Hocico']
    if c.z > JAW_TOP - 0.035 and abs(n.z) < 0.85:
        return MI['HuesoOscuro']  # labio claro (como Vulcanid)
    if n.z < -0.45:
        return MI['Vientre']
    return MI['Cuerpo']


paint_faces(mand, jaw_paint)
JAW_BVH = bvh_of(mand)
bm = bm_from(mand)
rng = random.Random(71)
tongue = bmesh.ops.create_icosphere(bm, subdivisions=2, radius=1.0,
                                    matrix=Matrix.Translation((0, -1.36, JAW_TOP - 0.006)) @
                                    Matrix.Diagonal((0.12, 0.26, 0.03, 1)))
for v in tongue['verts']:
    v.co.z += 0.008 * math.sin(v.co.y * 40)
for f in {f for v in tongue['verts'] for f in v.link_faces}:
    f.material_index = MI['Magma']
for i in range(10):
    y = -1.20 - 0.062 * i
    loc, nor = surface_hit(JAW_BVH, (1.5, y, JAW_TOP - 0.015), (0, y, JAW_TOP - 0.015))
    if loc is None:
        continue
    p = Vector((loc.x - 0.04, y, JAW_TOP - 0.003))
    Lt = rng.uniform(0.05, 0.075)
    M = Matrix.Translation(p) @ Matrix.Rotation(math.radians(90), 4, 'X') \
        @ Matrix.Rotation(math.radians(rng.uniform(-8, 8)), 4, 'Z')
    add_claw(bm, M @ Matrix.Diagonal((Lt, Lt, Lt, 1)), rng, length=1.0, curve=-0.15, base=0.3)
# colmillos que salen por fuera del labio superior
tl, tn = surface_hit(JAW_BVH, (0.6, -1.95, JAW_TOP - 0.005), (0.12, -1.70, JAW_TOP - 0.025))
if tl is not None:
    base = tl + Vector((-0.015, 0.02, 0.0))
    add_tube(bm, [base, base + Vector((0.03, -0.03, 0.09)), base + Vector((0.05, -0.02, 0.18)),
                  base + Vector((0.045, 0.03, 0.25))], [0.042, 0.034, 0.02, 0.004], sides=5, mat=MI['Hueso'])
bm_commit(mand, bm)
symmetrize(mand)
flat(mand)
set_pivot(mand, (0, -1.10, 0.86 + HZ))
mand.rotation_euler.x = math.radians(13)

HEAD_SCALE = 1.1
HP = Vector(HEAD_PIVOT)
for o in (craneo, ojos, cejas, cuernos, dientes_sup, mand):
    o.data.transform(Matrix.Diagonal((HEAD_SCALE, HEAD_SCALE, HEAD_SCALE, 1)))
    o.location = HP + (o.location - HP) * HEAD_SCALE


# =========================================================================== PATAS
def leg(prefix, coll, S, Kn, W, foot_c, r_up, r_lo, foot_size, front, seed):
    rng = random.Random(seed)
    S, Kn, W, foot_c = map(Vector, (S, Kn, W, foot_c))
    up = part(prefix + ('_Brazo' if front else '_Muslo'), coll, [
        B(S, r_up * 1.25), C(S, Kn, r_up),
        E((S + Kn) / 2 + Vector((0.06, 0, 0.02)), r_up * 1.05, r_up * 1.0, r_up * 1.35),
    ], tris=380, voxel=0.024, sym=False,
        disp=dict(scale=3, strength=0.025, cells=0.025, cell_scale=4, seed=seed))
    bvh = bvh_of(up)
    bm = bm_from(up)
    up_plates = [((0.18, 0.0, 0.16), 0.30, 0.6), ((0.21, 0.02, -0.22), 0.17, 0.5)]
    if not front:
        up_plates.append(((0.05, 0.24, -0.05), 0.17, 0.5))  # placa trasera del muslo
    for (off, rr, hh) in up_plates:
        tgt = S + Vector(off) * (1 if front else 1.1)
        loc, nor = surface_hit(bvh, tgt + Vector((0.8, 0, 0.35)), S + Vector((0, off[1], off[2] * 0.5)))
        if loc is None:
            continue
        M = align_matrix(loc - nor * 0.01, nor, rng.uniform(0, 6.28), (rr, rr, rr))
        add_dome_plate(bm, M, rng, sides=rng.choice((7, 8)), height=hh, mat=MI['Placa'])
    bm_commit(up, bm)
    paint_faces(up, lambda c, n: MI['CuerpoOscuro'] if n.z < -0.4 else None)
    flat(up)
    set_pivot(up, S)

    lo = part(prefix + ('_Antebrazo' if front else '_Pierna'), coll, [
        B(Kn, r_lo * 1.12), C(Kn, W, r_lo * 0.95), B(W + Vector((0, 0, 0.03)), r_lo * 1.0),
    ], tris=320, voxel=0.022, sym=False,
        disp=dict(scale=3, strength=0.02, cells=0.02, cell_scale=4, seed=seed + 1))
    bvh = bvh_of(lo)
    bm = bm_from(lo)
    mid = (Kn + W) / 2
    for (off, rr, hh) in (((0.0, -0.22, 0.04), 0.17, 0.5), ((0.20, -0.05, 0.02), 0.16, 0.48)):
        loc, nor = surface_hit(bvh, mid + Vector(off) * 4, mid + Vector((0, 0, off[2])))
        if loc is None:
            continue
        M = align_matrix(loc - nor * 0.01, nor, rng.uniform(0, 6.28), (rr, rr, rr))
        add_dome_plate(bm, M, rng, sides=7, height=hh, mat=MI['Placa'] if rng.random() < 0.6 else MI['PlacaClara'])
    bm_commit(lo, bm)
    paint_faces(lo, lambda c, n: MI['CuerpoOscuro'] if n.z < -0.3 else None)
    flat(lo)
    set_pivot(lo, Kn)

    fs = foot_size
    toe_dirs = [-0.42, 0.0, 0.42]
    elems = [E(foot_c, fs[0], fs[1], fs[2]), B(W + Vector((0, 0, -0.02)), r_lo * 0.95)]
    toes = []
    for a in toe_dirs:
        d = Vector((math.sin(a), -math.cos(a), 0))
        tp = foot_c + d * fs[1] * 0.92 + Vector((0, 0, -fs[2] * 0.1))
        toes.append((tp, d))
        elems.append(B(tp, fs[2] * 0.9))
    ft = part(prefix + ('_Mano' if front else '_Pie'), coll, elems, tris=300, voxel=0.018, sym=False,
              disp=dict(scale=4, strength=0.012, cells=0.015, cell_scale=6, seed=seed + 2))
    bisect(ft, (0, 0, 0.0), (0, 0, 1), keep_positive=True, fill=True, mat=MI['CuerpoOscuro'])
    paint_faces(ft, lambda c, n: MI['CuerpoOscuro'] if n.z < -0.2 else None)
    fbvh = bvh_of(ft)
    bm = bm_from(ft)
    loc, nor = surface_hit(fbvh, foot_c + Vector((0.05, -0.1, 1.0)), foot_c)
    if loc is not None:
        M = align_matrix(loc - nor * 0.01, nor, 0.3, (fs[0] * 0.75,) * 3)
        add_dome_plate(bm, M, rng, sides=7, height=0.45, mat=MI['Placa'])
    bm_commit(ft, bm)
    flat(ft)
    set_pivot(ft, W)

    gar = new_object(prefix + '_Garras', bpy.data.meshes.new(prefix + '_Garras'), coll, MATS)
    bm = bm_from(gar)
    for i, (tp, d) in enumerate(toes):
        Lc = fs[1] * (1.0 if i == 1 else 0.88)
        base = tp + d * fs[2] * 0.45
        yaw = math.atan2(-d.x, d.y)
        M = Matrix.Translation(base) @ Matrix.Rotation(yaw, 4, 'Z') @ Matrix.Rotation(math.radians(12), 4, 'X')
        add_claw(bm, M @ Matrix.Diagonal((Lc, Lc, Lc, 1)), rng, length=1.0, curve=0.38, base=0.3)
    back = foot_c + Vector((-0.10, fs[1] * 0.70, 0.12))
    M = Matrix.Translation(back) @ Matrix.Rotation(math.radians(25), 4, 'Z') @ Matrix.Rotation(math.radians(-5), 4, 'X')
    add_claw(bm, M @ Matrix.Diagonal((0.12, 0.12, 0.12, 1)), rng, length=1.0, curve=0.5, base=0.3)
    bm_commit(gar, bm)
    flat(gar)
    set_pivot(gar, W)
    return [up, lo, ft, gar]


front_L = leg('PataDel_Izq', COLS['Patas_Delanteras'],
              S=(0.52, -0.50, 0.96 + L), Kn=(0.72, -0.58, 0.58), W=(0.72, -0.64, 0.20),
              foot_c=(0.73, -0.74, 0.10), r_up=0.25, r_lo=0.21, foot_size=(0.23, 0.27, 0.115),
              front=True, seed=100)
back_L = leg('PataTra_Izq', COLS['Patas_Traseras'],
             S=(0.48, 0.76, 0.98 + L), Kn=(0.68, 0.56, 0.62), W=(0.68, 0.86, 0.22),
             foot_c=(0.69, 0.72, 0.10), r_up=0.29, r_lo=0.2, foot_size=(0.24, 0.31, 0.115),
             front=False, seed=200)
for src in front_L + back_L:
    o = mirror_object_x(src, src.name.replace('_Izq', '_Der'), src.users_collection[0])
    o.location = Vector((-src.location.x, src.location.y, src.location.z))

# =========================================================================== COLA
T = [Vector(p) for p in ((0, 1.02, 0.96 + L), (0, 1.42, 0.90), (0, 1.80, 0.76), (0, 2.16, 0.63), (0, 2.48, 0.55))]
TR = [0.36, 0.30, 0.24, 0.19, 0.15]
CLUB = Vector((0, 2.78, 0.54))
for i in range(4):
    a, b = T[i], T[i + 1]
    ext = (b - a).normalized() * 0.06
    seg = part('Cola_%d' % (i + 1), COLS['Cola'], [
        B(a, TR[i]), C(a, b + ext, (TR[i] + TR[i + 1]) / 2), B(b, TR[i + 1] * 1.02),
    ], tris=320, voxel=0.02, disp=dict(scale=3, strength=0.02, cells=0.02, cell_scale=4, seed=300 + i))
    paint_faces(seg, belly_paint)
    sb = bvh_of(seg)
    rng = random.Random(400 + i)
    bm = bm_from(seg)
    rift(bm, sb, a.y - 0.02, b.y + 0.02, w=0.06 - 0.01 * i)
    pk = [0.24, 0.18, 0.13, 0.09][i]
    basalt_field(bm, sb, lambda x, y, pk=pk, a=a, b=b, i=i: bump(x, y, 0.07, (a.y + b.y) / 2, 0.22 - 0.03 * i, 0.32, pk),
                 a.y, b.y, 0.07 - 0.01 * i, 0.3, 0.075 - 0.008 * i, rng)
    for j in range(1):
        t = 0.5
        loc, nor = radial_hit(sb, a.y + (b.y - a.y) * t, rng.uniform(66, 84), zc=a.z + (b.z - a.z) * t)
        if loc is not None:
            r = TR[i] * rng.uniform(0.5, 0.6)
            M = align_matrix(loc - nor * 0.01, nor, rng.uniform(0, 6.28), (r, r, r))
            add_dome_plate(bm, M, rng, sides=7, height=0.55, mat=MI['Placa'])
    bm_commit(seg, bm)
    symmetrize(seg)
    flat(seg)
    set_pivot(seg, a)

mazo = part('Cola_Mazo', COLS['Cola'], [
    C(T[4], CLUB, 0.16), B(CLUB, 0.29), E(CLUB + Vector((0, 0.08, 0.03)), 0.27, 0.25, 0.23),
], tris=450, voxel=0.02, disp=dict(scale=4, strength=0.03, cells=0.04, cell_scale=5, seed=500))
paint_faces(mazo, belly_paint)
mb_bvh = bvh_of(mazo)
bm = bm_from(mazo)
rng = random.Random(501)
golden = math.pi * (3 - math.sqrt(5))
n_dirs = 40
for k in range(n_dirs):
    zz = 1 - 2 * (k + 0.5) / n_dirs
    rr = math.sqrt(1 - zz * zz)
    th = golden * k
    d = Vector((rr * math.cos(th), rr * math.sin(th), zz))
    if d.y < -0.2 or d.z < -0.5 or d.x < -0.02:
        continue
    loc, nor = surface_hit(mb_bvh, CLUB + d * 2, CLUB)
    if loc is None:
        continue
    r = rng.uniform(0.065, 0.09)
    h = rng.uniform(0.16, 0.36) * (0.6 + 0.4 * max(0, d.y))
    axis = (d * 0.7 + nor * 0.3).normalized()
    M = align_matrix(loc - axis * 0.02, axis, rng.uniform(0, 6.28), (r, r, r))
    add_hex_column(bm, M, rng, height=h / r, slant=rng.uniform(0.3, 0.6), rim_w=0.12, rim_h=0.06)
bm_commit(mazo, bm)
symmetrize(mazo)
flat(mazo)
set_pivot(mazo, T[4])

# =========================================================================== raiz
root = bpy.data.objects.new('Basaltor', None)
ROOT_COL.objects.link(root)
root.empty_display_type = 'ARROWS'
for o in ROOT_COL.all_objects:
    if o is not root and o.parent is None:
        o.parent = root

total = 0
for o in sorted(ROOT_COL.all_objects, key=lambda o: o.name):
    if o.type == 'MESH':
        t = tri_count(o)
        total += t
        print('%-28s %6d tris' % (o.name, t))
print('TOTAL', total)
bpy.ops.wm.save_as_mainfile(filepath=OUT)
print('guardado', OUT)
