"""Obsidrax — tercera y ultima forma de la linea de Vulcanid (Aetherials).
Vulcanid -> Basaltor -> Obsidrax. El magma de Basaltor hace erupcion y se enfria de golpe en obsidiana.
Mismo flujo que Basaltor: metaballs -> remesh voxel -> tallado de roca -> decimate simetrico,
mas cristales de obsidiana, placas, columnas, ojos, dientes y cuernos colocados por ray casting.
Ejecutar: python3 build_obsidrax.py [salida.blend]
"""
import bpy, bmesh, math, random, sys, os
WORK = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, WORK)
from mathutils import Vector, Matrix, Euler, Quaternion
from mathutils.bvhtree import BVHTree
from lib import *
from lib import _bridge, _ring, _cap

OUT = sys.argv[-1] if sys.argv[-1].endswith('.blend') else os.path.join(WORK, 'obsidrax.blend')
NAME = 'Obsidrax'

bpy.ops.wm.read_factory_settings(use_empty=True)
MATS = build_materials()
ROOT_COL = collection(NAME)
COLS = {k: collection(k + '_O', ROOT_COL) for k in
        ('Cuerpo', 'Cabeza', 'Patas_Delanteras', 'Patas_Traseras', 'Cola')}

K = 1 / 0.575


def E(co, sx, sy, sz, rot=None, stiff=2.0, neg=False):
    return ellipsoid(co, K, sx, sy, sz, rot, stiff, neg)


def B(co, R, stiff=2.0, neg=False):
    return ball(co, R * K, stiff, neg)


def C(a, b, R, stiff=2.0, neg=False):
    return capsule(a, b, R * K, stiff, neg)


def part(name, coll, elems, tris, voxel=0.025, disp=None, res=0.025, sym=True):
    me = metaball_mesh(name, elems, res=res)
    o = new_object(name, me, coll, MATS)
    lowpoly(o, tris, voxel=voxel, disp=disp, sym=sym)
    set_material(o, MI['Cuerpo'])
    return o


def set_pivot(o, pivot):
    pv = Vector(pivot)
    o.data.transform(Matrix.Translation(-pv))
    o.location = pv


def frame(loc, z, xh):
    """Matriz con eje Z = z y eje X lo mas parecido posible a xh (direccion de la curva del cristal)."""
    z = Vector(z).normalized()
    xh = Vector(xh)
    x = xh - z * xh.dot(z)
    if x.length < 1e-4:
        x = Vector((1, 0, 0)) - z * z.x
    x.normalize()
    y = z.cross(x)
    return Matrix.Translation(Vector(loc)) @ Matrix((x, y, z)).transposed().to_4x4()


def bvh_multi(objs):
    bm = bmesh.new()
    for o in objs:
        n0 = len(bm.verts)
        bm.from_mesh(o.data)
        bm.verts.ensure_lookup_table()
        bmesh.ops.transform(bm, matrix=o.matrix_world, verts=bm.verts[n0:])
    t = BVHTree.FromBMesh(bm)
    bm.free()
    return t


def radial_hit(bvh, y, theta, zc):
    t = math.radians(theta)
    d = Vector((math.sin(t), 0, math.cos(t)))
    c = Vector((0, y, zc))
    return surface_hit(bvh, c + d * 5, c)


def belly_paint(c, n):
    if n.z < -0.55:
        return MI['VientreBanda'] if int((c.y + 3) / 0.22) % 2 else MI['Vientre']
    if n.z < -0.15:
        return MI['CuerpoOscuro']
    return None


def vein_mask(c, n):
    return -0.25 < n.z < 0.8


def rift(bm, bvh, y0, y1, w=0.08, h=0.05, step=0.06, mat=MI['MagmaCaliente']):
    prev = None
    n = max(2, int((y1 - y0) / step))
    for i in range(n + 1):
        y = y0 + (y1 - y0) * i / n
        loc, nor = surface_hit(bvh, (0, y, 8), (0, y, -1))
        if loc is None:
            prev = None
            continue
        hh = h * (0.7 + 0.6 * abs(math.sin(i * 1.3)))
        ww = w * (0.85 + 0.3 * abs(math.sin(i * 0.7)))
        Lv = bm.verts.new((-ww, y, loc.z - 0.04))
        Cv = bm.verts.new((0, y, loc.z + hh))
        Rv = bm.verts.new((ww, y, loc.z - 0.04))
        if prev:
            pL, pC, pR = prev
            bm.faces.new((pL, Lv, Cv, pC)).material_index = mat
            bm.faces.new((pC, Cv, Rv, pR)).material_index = mat
        prev = (Lv, Cv, Rv)


def blade_row(bm, bvh, y0, y1, step, height_fn, x_off, rng, lean_out=22, lean_back=28, radius=0.075,
              bend=0.18, nor_mix=0.25):
    """Fila de hojas de obsidiana a lo largo del lomo, inclinadas hacia afuera y hacia atras."""
    y = y0
    while y <= y1:
        h = height_fn(y) * rng.uniform(0.85, 1.12)
        if h > 0.06:
            x = x_off * rng.uniform(0.92, 1.08)
            loc, nor = surface_hit(bvh, (x, y, 8), (x, y, -1))
            if loc is not None:
                out = Vector((math.sin(math.radians(lean_out)), 0, math.cos(math.radians(lean_out))))
                axis = (out + Vector((0, math.tan(math.radians(lean_back)), 0)) + nor * nor_mix).normalized()
                M = frame(loc - axis * 0.02, axis, (0, 1, 0.2))
                add_shard(bm, M, rng, length=h, radius=radius * rng.uniform(0.85, 1.15), sides=rng.choice((5, 6)),
                          bend=bend, tip=0.35, rim_w=0.22)
        y += step


def bump(x, y, cx, cy, sx, sy, peak):
    d = ((x - cx) / sx) ** 2 + ((y - cy) / sy) ** 2
    return peak * max(0.0, 1 - d) ** 0.8


def basalt_field(bm, bvh, height_fn, y0, y1, x_min, x_max, R, rng, gap=0.016, lean=0.35, min_h=0.04, steps=0.04):
    dx = math.sqrt(3) * R + gap
    dy = 1.5 * R + gap * 0.87
    j, y = 0, y0
    while y <= y1:
        x = x_min + R * 0.866 + (dx / 2 if j % 2 else 0.0)
        while x <= x_max:
            h = height_fn(x, y)
            if h >= min_h:
                h = round(h * rng.uniform(0.82, 1.12) / steps) * steps
                loc, nor = surface_hit(bvh, (x, y, 8), (x, y, -1))
                if loc is not None and h > 0:
                    axis = (nor * 0.55 + Vector((0, 0, 0.45)) + Vector((x * lean, 0, 0))).normalized()
                    M = align_matrix(loc - axis * 0.02, axis, 0.0, (R, R, R))
                    add_hex_column(bm, M, rng, height=h / R, slant=rng.uniform(0.1, 0.45), phase=math.pi / 6,
                                   rim_w=0.10, chamfer=0.16, rim_h=0.06, depth=1.6)
            x += dx
        y += dy
        j += 1


# =========================================================================== TORSO
torso = part('Torso', COLS['Cuerpo'], [
    E((0, -0.55, 1.72), 0.78, 0.70, 0.66),
    E((0, 0.20, 1.58), 0.80, 0.78, 0.62),
    E((0, 0.95, 1.48), 0.68, 0.60, 0.56),
    B((0.62, -0.62, 1.70), 0.45), B((-0.62, -0.62, 1.70), 0.45),
    B((0.55, 1.00, 1.48), 0.40), B((-0.55, 1.00, 1.48), 0.40),
    E((0, 0.10, 1.25), 0.60, 0.85, 0.36),
    E((0, -1.05, 1.90), 0.48, 0.38, 0.46),
    E((0, 1.40, 1.42), 0.44, 0.38, 0.42),
], tris=2200, voxel=0.035, disp=dict(scale=1.6, strength=0.05, cells=0.035, cell_scale=2.4, seed=11))
paint_faces(torso, belly_paint)
TORSO_BVH = bvh_of(torso)  # antes de las grietas: los rayos deben dar con la superficie real
print('grietas torso', add_cracks(torso, n_walks=16, steps=9, width=0.026, mask=lambda c, n: vein_mask(c, n) and c.z > 1.2, seed=2))

# --- cuello (antes que la melena: la melena se apoya en torso + cuello)
cuello = part('Cuello', COLS['Cuerpo'], [
    C((0, -0.92, 1.92), (0, -1.62, 2.24), 0.40),
    E((0, -1.25, 2.20), 0.32, 0.34, 0.20),
    E((0, -1.30, 1.86), 0.30, 0.34, 0.20),
], tris=700, voxel=0.028, disp=dict(scale=2.5, strength=0.03, cells=0.02, cell_scale=3.5, seed=12))
paint_faces(cuello, lambda c, n: MI['Vientre'] if n.z < -0.5 else None)
NECK_BVH = bvh_of(cuello)
add_cracks(cuello, n_walks=6, steps=7, width=0.022, mask=lambda c, n: vein_mask(c, n), seed=3)
bm = bm_from(cuello)
rift(bm, NECK_BVH, -1.62, -0.95, w=0.07)
blade_row(bm, NECK_BVH, -1.55, -1.0, 0.13, lambda y: 0.16 + 0.25 * (y + 1.55) / 0.55, 0.11,
          random.Random(13), lean_out=18, lean_back=35, radius=0.06)
bm_commit(cuello, bm)
symmetrize(cuello)
flat(cuello)

# --- lomo: rio de magma con hojas de obsidiana + panal de basalto en la cadera (herencia de Basaltor)
lomo = new_object('Lomo', bpy.data.meshes.new('Lomo'), COLS['Cuerpo'], MATS)
bm = bm_from(lomo)
rift(bm, TORSO_BVH, -0.80, 1.55, w=0.10, h=0.055)


def back_blade_h(y):
    return 0.20 + 0.62 * math.exp(-((y + 0.40) / 0.50) ** 2) + 0.28 * math.exp(-((y - 0.65) / 0.35) ** 2)


rng = random.Random(21)
blade_row(bm, TORSO_BVH, -0.72, 1.35, 0.17, back_blade_h, 0.17, rng, lean_out=16, lean_back=26, radius=0.085)
blade_row(bm, TORSO_BVH, -0.64, 1.25, 0.21, lambda y: back_blade_h(y) * 0.55, 0.36, rng, lean_out=38,
          lean_back=22, radius=0.075)
basalt_field(bm, TORSO_BVH, lambda x, y: bump(x, y, 0.52, 0.85, 0.22, 0.38, 0.26), 0.45, 1.25, 0.32, 0.75,
             0.075, random.Random(22))
bm_commit(lomo, bm)
symmetrize(lomo)
flat(lomo)

# --- melena de obsidiana alrededor del cuello
melena = new_object('Torso__Melena', bpy.data.meshes.new('Melena'), COLS['Cuerpo'], MATS)
NT_BVH = bvh_multi([torso, cuello])
bm = bm_from(melena)
rng = random.Random(31)
neck_a = Vector((0, -0.75, 0.32)).normalized()       # hacia la cabeza
e1 = Vector((1, 0, 0))
e2 = neck_a.cross(e1).normalized()
if e2.z < 0:
    e2 = -e2
for row, (cy, base_len, rad, th0) in enumerate(((-1.02, 1.35, 0.13, 0.0), (-0.80, 1.0, 0.11, 11.0), (-0.60, 0.62, 0.09, 0.0))):
    center = Vector((0, cy, 1.98 + 0.12 * row))
    th = th0
    while th <= 118:
        t = math.radians(th)
        r = e2 * math.cos(t) + e1 * math.sin(t)
        loc, nor = surface_hit(NT_BVH, center + r * 4, center)
        if loc is not None:
            L = base_len * (1.0 - 0.45 * (th / 118) ** 1.6) * rng.uniform(0.85, 1.1)
            d = (r * 0.80 - neck_a * 0.80 + Vector((0, 0, 0.22))).normalized()
            M = frame(loc - d * 0.03, d, -neck_a + Vector((0, 0, 0.3)))
            shard_on_surface(bm, loc, nor, M, rng, length=L, radius=rad * rng.uniform(0.85, 1.15),
                             sides=rng.choice((5, 6)), bend=0.10, tip=0.38)
        th += 22
bm_commit(melena, bm)
symmetrize(melena)
flat(melena)

# --- placas de los costados (Placa y obsidiana)
bm = bm_from(torso)
rng = random.Random(41)
placed, tries = [], 0
while len(placed) < 13 and tries < 4000:
    tries += 1
    y = rng.uniform(-0.6, 1.35)
    th = rng.uniform(58, 112)
    if (abs(y + 0.62) < 0.40 and th > 62) or (abs(y - 1.0) < 0.36 and th > 62):
        continue
    r = rng.uniform(0.18, 0.27)
    loc, nor = radial_hit(TORSO_BVH, y, th, 1.55)
    if loc is None:
        continue
    if any((loc - p).length < (r + pr) * 0.95 for p, pr in placed):
        continue
    placed.append((loc, r))
    M = align_matrix(loc - nor * r * 0.18, nor, rng.uniform(0, 6.28), (r, r, r))
    if rng.random() < 0.45:
        add_dome_plate(bm, M, rng, sides=5, height=0.38, mat=MI['Obsidiana'], peak=0.7,
                       rim=MI['MagmaProfundo'], rim_w=0.03)
    else:
        add_dome_plate(bm, M, rng, sides=rng.choice((6, 7, 8)), height=rng.uniform(0.42, 0.6),
                       mat=MI['Placa'] if rng.random() < 0.7 else MI['PlacaClara'])
bm_commit(torso, bm)
symmetrize(torso)
flat(torso)

# --- corazon de magma en el pecho, sujeto por costillas de obsidiana
nucleo = new_object('Torso__Nucleo', bpy.data.meshes.new('Nucleo'), COLS['Cuerpo'], MATS)
bm = bm_from(nucleo)
rng = random.Random(51)
cl, cn = surface_hit(TORSO_BVH, (0, -3.5, 1.52), (0, 0, 1.62))
print('pecho', cl, cn)
cn = (cn + Vector((0, -0.4, 0))).normalized()
core = bmesh.ops.create_icosphere(bm, subdivisions=1, radius=1.0,
                                  matrix=frame(cl + cn * 0.04, cn, (1, 0, 0)) @ Matrix.Diagonal((0.24, 0.30, 0.2, 1)))
for f in {f for v in core['verts'] for f in v.link_faces}:  # gema facetada: tonos alternados
    r_ = rng.random()
    f.material_index = MI['MagmaCaliente'] if r_ < 0.3 else (MI['Magma'] if r_ < 0.75 else MI['MagmaProfundo'])
ex_ = Vector((1, 0, 0))
ey_ = cn.cross(ex_).normalized()
for ang in (25, 75, 125, 170):
    a = math.radians(ang)
    radial = ex_ * math.cos(a) + ey_ * math.sin(a)
    if radial.x < -0.01:
        continue
    p = cl + radial * 0.40
    hit, hn = surface_hit(TORSO_BVH, p + cn * 1.5, p - cn * 0.5)
    if hit is None:
        continue
    axis = (cn * 0.55 - radial * 0.45).normalized()
    M = frame(hit - axis * 0.03, axis, -radial)
    shard_on_surface(bm, hit, hn, M, rng, pool=1.1, length=0.46, radius=0.07, sides=5, bend=0.4, tip=0.35)
bm_commit(nucleo, bm)
symmetrize(nucleo)
flat(nucleo)

TORSO_PIVOT = (0, 0.2, 1.6)
for o in (torso, lomo, melena, nucleo):
    set_pivot(o, TORSO_PIVOT)
set_pivot(cuello, (0, -0.95, 1.92))

# =========================================================================== CABEZA
HP = Vector((0, -1.62, 2.24))
craneo = part('Craneo', COLS['Cabeza'], [
    E((0, -1.95, 2.36), 0.50, 0.44, 0.38),
    E((0, -2.35, 2.22), 0.42, 0.36, 0.25),
    E((0, -2.66, 2.16), 0.33, 0.15, 0.19),
    B((0.40, -1.90, 2.20), 0.28), B((-0.40, -1.90, 2.20), 0.28),
    B((0, -1.68, 2.30), 0.34),
    E((0.27, -2.12, 2.52), 0.17, 0.17, 0.10), E((-0.27, -2.12, 2.52), 0.17, 0.17, 0.10),
], tris=1600, voxel=0.022, res=0.02, disp=dict(scale=2.4, strength=0.018, cells=0.018, cell_scale=4, seed=61))
MZ, MTILT, MY = 2.08, 0.04, -2.2


def mouth_z(y):
    return MZ - MTILT * (y - MY)


bisect(craneo, (0, MY, MZ), (0, MTILT, 1), keep_positive=True, fill=True, mat=MI['MagmaProfundo'])
symmetrize(craneo)


def head_paint(c, n):
    if c.z < mouth_z(c.y) + 0.012 and n.z < -0.9:
        return MI['MagmaProfundo']
    if c.y < -2.62 and n.y < -0.25 and c.z < 2.3:
        return MI['Hocico']
    if n.z > 0.40 and c.y < -2.15 and abs(c.x) < 0.33:
        return MI['CabezaGris']
    if n.z < -0.3:
        return MI['CuerpoOscuro']
    return MI['Cuerpo']


paint_faces(craneo, head_paint)
flat(craneo)
HEAD_BVH = bvh_of(craneo)
add_cracks(craneo, n_walks=3, steps=5, width=0.016, mask=lambda c, n: 0.0 < n.z < 0.75 and c.y > -2.2 and c.z > 2.25, seed=4)

# --- ojos de magma fundido (forma hexagonal de la familia, pupila vertical oscura)
ojos = new_object('Ojos', bpy.data.meshes.new('Ojos'), COLS['Cabeza'], MATS)
bm = bm_from(ojos)
eye_loc, eye_nor = surface_hit(HEAD_BVH, (1.5, -3.05, 2.62), (0.0, -2.12, 2.30))
print('ojo', eye_loc, eye_nor)


def add_eye(bm, loc, nor, scale):
    z = nor.normalized()
    fwd = Vector((0, -1, 0))
    x = (fwd - z * fwd.dot(z)).normalized()
    y = z.cross(x).normalized()
    if y.z < 0:
        y = -y
    M = Matrix.Translation(loc + z * 0.004) @ Matrix((x, y, z)).transposed().to_4x4() @ \
        Matrix.Diagonal((scale, scale, scale, 1))
    hexpts = [(-0.085, 0.0), (-0.05, 0.035), (0.055, 0.042), (0.09, 0.0), (0.055, -0.036), (-0.05, -0.03)]
    front = [bm.verts.new((px, py, 0.014)) for px, py in hexpts]
    back = [bm.verts.new((px * 1.22, py * 1.32, -0.05)) for px, py in hexpts]
    f = bm.faces.new(front)
    f.material_index = MI['Magma']
    fs = [f] + _bridge(bm, back, front, MI['Obsidiana'])
    pw, ph = 0.010, 0.036
    pv = [bm.verts.new(p) for p in ((-pw, -ph, 0.017), (pw, -ph, 0.017), (pw, ph, 0.017), (-pw, ph, 0.017))]
    bm.faces.new(pv).material_index = MI['Pupila']
    gv = [bm.verts.new(p) for p in ((-0.036, 0.012, 0.0185), (-0.022, 0.012, 0.0185),
                                    (-0.022, 0.026, 0.0185), (-0.036, 0.026, 0.0185))]
    bm.faces.new(gv).material_index = MI['Brillo']
    bmesh.ops.transform(bm, matrix=M, verts=front + back + pv + gv)
    bmesh.ops.recalc_face_normals(bm, faces=fs)
    return x, y, z


ES = 1.6
ex, ey, ez = add_eye(bm, eye_loc, eye_nor, ES)
bm_commit(ojos, bm)
symmetrize(ojos)
flat(ojos)

# --- cejas de obsidiana
cejas = new_object('Cejas', bpy.data.meshes.new('Cejas'), COLS['Cabeza'], MATS)
bm = bm_from(cejas)
rng = random.Random(71)
bt = eye_loc + ey * (0.042 * ES + 0.06)
bl, bn = surface_hit(HEAD_BVH, bt + ez * 0.6 + Vector((0, 0, 0.3)), bt - ez * 0.1)
if bl is not None:
    zax = (bn * 0.5 + ez * 0.2 + Vector((0, 0, 0.6))).normalized()
    xax = (ex - zax * ex.dot(zax)).normalized()
    xax = (xax + Vector((0, 0, -0.45))).normalized()
    yax = zax.cross(xax).normalized()
    zax = xax.cross(yax).normalized()
    M = Matrix.Translation(bl + ez * 0.02) @ Matrix((xax, yax, zax)).transposed().to_4x4() @ \
        Matrix.Diagonal((0.21, 0.065, 0.10, 1))
    add_dome_plate(bm, M, rng, sides=5, height=0.95, mat=MI['Obsidiana'], rim_w=0.14, peak=0.05)
bm_commit(cejas, bm)
symmetrize(cejas)
flat(cejas)

# --- corona: cuernos principales estriados de obsidiana, cuernos de ceja, cuerno nasal y puas de mejilla
cuernos = new_object('Cuernos', bpy.data.meshes.new('Cuernos'), COLS['Cabeza'], MATS)
bm = bm_from(cuernos)
horn = [Vector(p) for p in ((0.27, -1.86, 2.56), (0.39, -1.71, 2.77), (0.53, -1.56, 2.94), (0.67, -1.39, 3.06),
                            (0.79, -1.19, 3.14), (0.87, -0.99, 3.19), (0.91, -0.81, 3.29), (0.92, -0.70, 3.43))]
hr = [0.165, 0.155, 0.14, 0.12, 0.10, 0.075, 0.045, 0.012]
dense, drad = [], []
NS = 25
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
    drad.append(r * (1.13 if (s % 3 == 1 and t < 0.85) else 1.0))
add_tube(bm, dense, drad, sides=6, twist=0.04,
         mat_fn=lambda t: (MI['ObsidianaBrillo'] if (int(t * 24) % 3 == 1 or t > 0.86) else MI['Obsidiana']))
hl, hn = surface_hit(HEAD_BVH, horn[0] + Vector((0.12, 0.05, 0.45)), horn[0] - Vector((0.05, 0, 0.12)))
if hl is not None:
    add_hex_column(bm, align_matrix(hl - hn * 0.02, hn, 0, (0.2, 0.2, 0.12)), random.Random(72), height=0.6,
                   slant=0.1, mat=MI['Obsidiana'], top=MI['Obsidiana'], rim_w=0.14)
rng = random.Random(73)
for pts, rads in ((((0.30, -2.20, 2.55), (0.40, -2.28, 2.70), (0.47, -2.31, 2.83), (0.50, -2.30, 2.93)),
                   (0.07, 0.055, 0.03, 0.006)),
                  (((0.46, -1.80, 2.10), (0.62, -1.62, 2.12), (0.76, -1.44, 2.18), (0.86, -1.30, 2.27)),
                   (0.09, 0.07, 0.04, 0.008)),
                  (((0.47, -1.98, 2.04), (0.62, -1.85, 2.02), (0.73, -1.74, 2.03)), (0.065, 0.04, 0.006))):
    add_tube(bm, [Vector(p) for p in pts], list(rads), sides=5,
             mat_fn=lambda t: MI['Obsidiana'] if t < 0.6 else MI['ObsidianaBrillo'])
nl, nn = surface_hit(HEAD_BVH, (0, -2.75, 3.2), (0, -2.42, 2.3))
if nl is not None:
    axis = (nn * 0.6 + Vector((0, -0.55, 0.45))).normalized()
    add_shard(bm, frame(nl - axis * 0.03, axis, (0, 1, 0.4)), rng, length=0.42, radius=0.085, sides=5,
              bend=0.28, tip=0.4, rim_w=0.2)
bm_commit(cuernos, bm)
symmetrize(cuernos)
flat(cuernos)

# --- cresta de hojas y grieta sobre la cabeza + fosas nasales
bm = bm_from(craneo)
rift(bm, HEAD_BVH, -2.35, -1.62, w=0.05, h=0.03)
blade_row(bm, HEAD_BVH, -2.15, -1.70, 0.11, lambda y: 0.14 + 0.34 * (y + 2.15) / 0.45, 0.075,
          random.Random(74), lean_out=14, lean_back=40, radius=0.055, bend=0.22)
nl, nn = surface_hit(HEAD_BVH, (0.15, -3.4, 2.7), (0.13, -2.62, 2.22))
if nl is not None:
    add_hex_column(bm, align_matrix(nl, nn, math.radians(70), (0.04, 0.019, 0.02)), random.Random(5), height=0.25,
                   slant=0.0, mat=MI['Hocico'], top=MI['Magma'], rim=MI['Hocico'], rim_w=0.35, chamfer=0.3)
bm_commit(craneo, bm)
symmetrize(craneo)
flat(craneo)

# --- dientes superiores
dientes_sup = new_object('Dientes_Superiores', bpy.data.meshes.new('DS_O'), COLS['Cabeza'], MATS)
bm = bm_from(dientes_sup)
rng = random.Random(81)
for i in range(13):
    y = -1.84 - 0.066 * i
    zr = mouth_z(y) + 0.02
    loc, nor = surface_hit(HEAD_BVH, (2, y, zr), (0, y, zr))
    if loc is None:
        continue
    p = Vector((loc.x - 0.04, y, zr))
    big = i in (8, 9)
    Lt = 0.22 if big else rng.uniform(0.08, 0.12)
    M = Matrix.Translation(p) @ Matrix.Rotation(math.radians(-90), 4, 'X') @ \
        Matrix.Rotation(math.radians(rng.uniform(-8, 8)), 4, 'Z')
    add_claw(bm, M @ Matrix.Diagonal((Lt, Lt, Lt, 1)), rng, length=1.0, curve=0.18, base=0.26 if big else 0.3)
for x in (0.04, 0.12):
    zr = mouth_z(-2.75) + 0.02
    loc, nor = surface_hit(HEAD_BVH, (x, -3.4, zr), (x, -2.3, zr))
    if loc is not None:
        M = Matrix.Translation(loc + Vector((0, 0.04, 0))) @ Matrix.Rotation(math.radians(-90), 4, 'X')
        add_claw(bm, M @ Matrix.Diagonal((0.09, 0.09, 0.09, 1)), rng, length=1.0, curve=-0.2, base=0.3)
bm_commit(dientes_sup, bm)
symmetrize(dientes_sup)
assert len(dientes_sup.data.vertices) > 0
flat(dientes_sup)
for o in (craneo, ojos, cejas, cuernos, dientes_sup):
    set_pivot(o, HP)

# =========================================================================== MANDIBULA
mand = part('Mandibula', COLS['Cabeza'], [
    E((0, -2.15, 1.96), 0.42, 0.48, 0.17),
    E((0, -2.55, 1.97), 0.32, 0.14, 0.13),
    B((0.38, -1.78, 2.04), 0.17), B((-0.38, -1.78, 2.04), 0.17),
    E((0, -2.05, 1.84), 0.28, 0.34, 0.12),
], tris=800, voxel=0.02, res=0.02, disp=dict(scale=3, strength=0.01, cells=0.01, cell_scale=5, seed=62))
JT = 2.065
bisect(mand, (0, 0, JT), (0, 0, -1), keep_positive=True, fill=True, mat=MI['Hocico'])
symmetrize(mand)


def jaw_paint(c, n):
    if n.z > 0.9 and c.z > JT - 0.012:
        return MI['Hocico']
    if c.z > JT - 0.045 and abs(n.z) < 0.85:
        return MI['HuesoOscuro']
    if n.z < -0.45:
        return MI['Vientre']
    return MI['Cuerpo']


paint_faces(mand, jaw_paint)
JAW_BVH = bvh_of(mand)
bm = bm_from(mand)
rng = random.Random(91)
tongue = bmesh.ops.create_icosphere(bm, subdivisions=2, radius=1.0,
                                    matrix=Matrix.Translation((0, -2.12, JT - 0.006)) @ Matrix.Diagonal((0.15, 0.34, 0.04, 1)))
for v in tongue['verts']:
    v.co.z += 0.01 * math.sin(v.co.y * 34)
for f in {f for v in tongue['verts'] for f in v.link_faces}:
    f.material_index = MI['MagmaCaliente']
for i in range(12):
    y = -1.84 - 0.064 * i
    loc, nor = surface_hit(JAW_BVH, (2, y, JT - 0.018), (0, y, JT - 0.018))
    if loc is None:
        continue
    p = Vector((loc.x - 0.045, y, JT - 0.004))
    Lt = rng.uniform(0.065, 0.1)
    M = Matrix.Translation(p) @ Matrix.Rotation(math.radians(90), 4, 'X') @ \
        Matrix.Rotation(math.radians(rng.uniform(-8, 8)), 4, 'Z')
    add_claw(bm, M @ Matrix.Diagonal((Lt, Lt, Lt, 1)), rng, length=1.0, curve=-0.15, base=0.3)
tl, tn = surface_hit(JAW_BVH, (0.8, -2.8, JT - 0.006), (0.16, -2.5, JT - 0.03))
if tl is not None:
    base = tl + Vector((-0.02, 0.03, 0.0))
    add_tube(bm, [base, base + Vector((0.04, -0.04, 0.12)), base + Vector((0.065, -0.03, 0.24)),
                  base + Vector((0.06, 0.04, 0.33))], [0.055, 0.045, 0.026, 0.005], sides=5, mat=MI['Hueso'])
bm_commit(mand, bm)
symmetrize(mand)
flat(mand)
set_pivot(mand, (0, -1.74, 2.05))
mand.rotation_euler.x = math.radians(15)


# =========================================================================== PATAS
def leg(prefix, coll, S, Kn, W, foot_c, r_up, r_lo, fs, front, seed):
    rng = random.Random(seed)
    S, Kn, W, foot_c = map(Vector, (S, Kn, W, foot_c))
    sc = r_up / 0.25
    up = part(prefix + ('_Brazo' if front else '_Muslo'), coll, [
        B(S, r_up * 1.25), C(S, Kn, r_up),
        E((S + Kn) / 2 + Vector((0.06, 0, 0.02)) * sc, r_up * 1.05, r_up * 1.0, r_up * 1.35),
    ], tris=480, voxel=0.03, sym=False, disp=dict(scale=2.4, strength=0.03, cells=0.025, cell_scale=3.4, seed=seed))
    bvh = bvh_of(up)
    add_cracks(up, n_walks=4, steps=6, width=0.022, mask=lambda c, n: -0.3 < n.z < 0.9 and n.x > -0.2, seed=seed, sym=False)
    bm = bm_from(up)
    plates = [((0.18, 0.0, 0.16), 0.32, 0.55, 'O'), ((0.21, 0.02, -0.24), 0.18, 0.5, 'P')]
    if not front:
        plates.append(((0.05, 0.25, -0.05), 0.18, 0.5, 'P'))
    for (off, rr, hh, kind) in plates:
        o3 = Vector(off) * sc
        loc, nor = surface_hit(bvh, S + o3 + Vector((0.8, 0, 0.35)) * sc, S + Vector((0, o3.y, o3.z * 0.5)))
        if loc is None:
            continue
        M = align_matrix(loc - nor * rr * sc * (0.42 if kind == 'O' else 0.28), nor, rng.uniform(0, 6.28), (rr * sc,) * 3)
        if kind == 'O':
            add_dome_plate(bm, M, rng, sides=5, height=0.62, mat=MI['Obsidiana'], peak=0.8,
                           rim=MI['MagmaProfundo'], rim_w=0.03)
        else:
            add_dome_plate(bm, M, rng, sides=rng.choice((7, 8)), height=hh, mat=MI['Placa'])
    bm_commit(up, bm)
    paint_faces(up, lambda c, n: MI['CuerpoOscuro'] if n.z < -0.4 else None)
    flat(up)
    set_pivot(up, S)

    lo = part(prefix + ('_Antebrazo' if front else '_Pierna'), coll, [
        B(Kn, r_lo * 1.12), C(Kn, W, r_lo * 0.95), B(W + Vector((0, 0, 0.03)), r_lo * 1.0),
    ], tris=400, voxel=0.028, sym=False, disp=dict(scale=2.4, strength=0.025, cells=0.02, cell_scale=3.4, seed=seed + 1))
    bvh = bvh_of(lo)
    add_cracks(lo, n_walks=3, steps=6, width=0.02, mask=lambda c, n: -0.3 < n.z < 0.9 and n.x > -0.2, seed=seed + 1, sym=False)
    bm = bm_from(lo)
    mid = (Kn + W) / 2
    for (off, rr, hh) in (((0.0, -0.22, 0.04), 0.17, 0.5), ((0.20, -0.05, 0.02), 0.16, 0.48)):
        o3 = Vector(off) * sc
        loc, nor = surface_hit(bvh, mid + o3 * 4, mid + Vector((0, 0, o3.z)))
        if loc is None:
            continue
        M = align_matrix(loc - nor * 0.015, nor, rng.uniform(0, 6.28), (rr * sc,) * 3)
        add_dome_plate(bm, M, rng, sides=7, height=hh, mat=MI['Placa'] if rng.random() < 0.6 else MI['PlacaClara'])
    # pua de obsidiana: codo (delanteras) o talon (traseras), apuntando hacia atras
    jp = Kn if front else W + Vector((0, 0, 0.12))
    loc, nor = surface_hit(bvh, jp + Vector((0.15, 1.5, 0.1)), jp)
    if loc is not None:
        axis = (Vector((0.25, 1.0, 0.35 if front else 0.15))).normalized()
        shard_on_surface(bm, loc, nor, frame(loc - axis * 0.04, axis, (0, 0, 1)), rng, length=0.42 * sc * 0.8,
                         radius=0.075 * sc * 0.8, sides=5, bend=0.25, tip=0.4)
    bm_commit(lo, bm)
    paint_faces(lo, lambda c, n: MI['CuerpoOscuro'] if n.z < -0.3 else None)
    flat(lo)
    set_pivot(lo, Kn)

    toe_dirs = [-0.55, -0.18, 0.18, 0.55]
    elems = [E(foot_c, fs[0], fs[1], fs[2]), B(W + Vector((0, 0, -0.02)), r_lo * 0.95)]
    toes = []
    for a in toe_dirs:
        d = Vector((math.sin(a), -math.cos(a), 0))
        tp = foot_c + d * fs[1] * 0.92 + Vector((0, 0, -fs[2] * 0.1))
        toes.append((tp, d))
        elems.append(B(tp, fs[2] * 0.85))
    ft = part(prefix + ('_Mano' if front else '_Pie'), coll, elems, tris=420, voxel=0.022, sym=False,
              disp=dict(scale=3, strength=0.012, cells=0.015, cell_scale=5, seed=seed + 2))
    bisect(ft, (0, 0, 0.0), (0, 0, 1), keep_positive=True, fill=True, mat=MI['CuerpoOscuro'])
    paint_faces(ft, lambda c, n: MI['CuerpoOscuro'] if n.z < -0.2 else None)
    fbvh = bvh_of(ft)
    bm = bm_from(ft)
    loc, nor = surface_hit(fbvh, foot_c + Vector((0.05, -0.1, 1.0)), foot_c)
    if loc is not None:
        add_dome_plate(bm, align_matrix(loc - nor * 0.01, nor, 0.3, (fs[0] * 0.75,) * 3), rng, sides=5,
                       height=0.42, mat=MI['Obsidiana'], peak=0.6)
    bm_commit(ft, bm)
    flat(ft)
    set_pivot(ft, W)

    gar = new_object(prefix + '_Garras', bpy.data.meshes.new(prefix + '_Garras_O'), coll, MATS)
    bm = bm_from(gar)
    for i, (tp, d) in enumerate(toes):
        Lc = fs[1] * (0.95 if i in (1, 2) else 0.8)
        base = tp + d * fs[2] * 0.45
        M = Matrix.Translation(base) @ Matrix.Rotation(math.atan2(-d.x, d.y), 4, 'Z') @ \
            Matrix.Rotation(math.radians(12), 4, 'X')
        add_claw(bm, M @ Matrix.Diagonal((Lc, Lc, Lc, 1)), rng, length=1.0, curve=0.38, base=0.28)
    back = foot_c + Vector((-0.12, fs[1] * 0.70, 0.14))
    M = Matrix.Translation(back) @ Matrix.Rotation(math.radians(25), 4, 'Z') @ Matrix.Rotation(math.radians(-5), 4, 'X')
    add_claw(bm, M @ Matrix.Diagonal((0.15, 0.15, 0.15, 1)), rng, length=1.0, curve=0.5, base=0.3)
    bm_commit(gar, bm)
    flat(gar)
    set_pivot(gar, W)
    return [up, lo, ft, gar]


front_L = leg('PataDel_Izq', COLS['Patas_Delanteras'], S=(0.72, -0.65, 1.66), Kn=(0.95, -0.76, 0.98),
              W=(0.95, -0.85, 0.30), foot_c=(0.96, -0.99, 0.15), r_up=0.34, r_lo=0.28, fs=(0.31, 0.37, 0.155),
              front=True, seed=100)
back_L = leg('PataTra_Izq', COLS['Patas_Traseras'], S=(0.62, 1.02, 1.48), Kn=(0.88, 0.78, 0.86),
             W=(0.88, 1.18, 0.30), foot_c=(0.89, 1.02, 0.15), r_up=0.38, r_lo=0.26, fs=(0.31, 0.41, 0.155),
             front=False, seed=200)
for src in front_L + back_L:
    o = mirror_object_x(src, src.name.replace('_Izq', '_Der'), src.users_collection[0])
    o.location = Vector((-src.location.x, src.location.y, src.location.z))

# =========================================================================== COLA
T = [Vector(p) for p in ((0, 1.36, 1.45), (0, 1.96, 1.26), (0, 2.56, 1.02), (0, 3.12, 0.83), (0, 3.62, 0.71))]
TR = [0.46, 0.38, 0.30, 0.23, 0.18]
ORB = Vector((0, 4.02, 0.74))
for i in range(4):
    a, b = T[i], T[i + 1]
    ext = (b - a).normalized() * 0.08
    seg = part('Cola_%d' % (i + 1), COLS['Cola'], [
        B(a, TR[i]), C(a, b + ext, (TR[i] + TR[i + 1]) / 2), B(b, TR[i + 1] * 1.02),
    ], tris=420, voxel=0.026, disp=dict(scale=2.6, strength=0.025, cells=0.02, cell_scale=3.5, seed=300 + i))
    paint_faces(seg, belly_paint)
    sb = bvh_of(seg)
    add_cracks(seg, n_walks=3, steps=6, width=0.02, mask=lambda c, n: -0.2 < n.z < 0.85, seed=10 + i)
    rng = random.Random(400 + i)
    bm = bm_from(seg)
    rift(bm, sb, a.y - 0.02, b.y + 0.02, w=0.08 - 0.012 * i, h=0.045 - 0.006 * i)
    hb = [0.36, 0.30, 0.24, 0.18][i]
    blade_row(bm, sb, a.y + 0.08, b.y - 0.02, 0.2 - 0.02 * i, lambda y, hb=hb: hb, 0.10 - 0.012 * i, rng,
              lean_out=22, lean_back=40, radius=0.07 - 0.008 * i, bend=0.25)
    t_ = 0.5
    loc, nor = radial_hit(sb, a.y + (b.y - a.y) * t_, rng.uniform(66, 84), a.z + (b.z - a.z) * t_)
    if loc is not None:
        r = TR[i] * 0.55
        M = align_matrix(loc - nor * 0.015, nor, rng.uniform(0, 6.28), (r, r, r))
        add_dome_plate(bm, M, rng, sides=5 if i % 2 else 7, height=0.45,
                       mat=MI['Obsidiana'] if i % 2 else MI['Placa'], peak=0.6 if i % 2 else 0.25)
    bm_commit(seg, bm)
    symmetrize(seg)
    flat(seg)
    set_pivot(seg, a)

# orbe de magma enjaulado por garras de obsidiana
mazo = part('Cola_Mazo', COLS['Cola'], [
    C(T[4], ORB - Vector((0, 0.26, 0)), 0.17), B(ORB - Vector((0, 0.24, 0)), 0.22),
], tris=420, voxel=0.022, disp=dict(scale=3, strength=0.02, cells=0.03, cell_scale=4, seed=500))
paint_faces(mazo, belly_paint)
bm = bm_from(mazo)
rng = random.Random(501)
orb = bmesh.ops.create_icosphere(bm, subdivisions=2, radius=0.3, matrix=Matrix.Translation(ORB))
for v in orb['verts']:
    v.co += (v.co - ORB).normalized() * rng.uniform(-0.025, 0.025)
for f in {f for v in orb['verts'] for f in v.link_faces}:
    f.material_index = MI['MagmaCaliente'] if rng.random() < 0.3 else MI['Magma']
collar_c = ORB - Vector((0, 0.24, 0))
for k in range(7):
    a = 2 * math.pi * k / 7 + 0.2
    radial = Vector((math.cos(a), 0, math.sin(a)))
    if radial.x < -0.05:
        continue
    base = collar_c + radial * 0.2
    axis = (Vector((0, 1, 0)) * 0.75 + radial * 0.65).normalized()
    M = frame(base, axis, -radial)
    add_shard(bm, M, rng, length=0.62, radius=0.075, sides=5, bend=0.55, tip=0.35, rim_w=0.2, depth=0.15)
bm_commit(mazo, bm)
symmetrize(mazo)
flat(mazo)
set_pivot(mazo, T[4])

# =========================================================================== raiz
root = bpy.data.objects.new(NAME, None)
ROOT_COL.objects.link(root)
root.empty_display_type = 'ARROWS'
for o in ROOT_COL.all_objects:
    if o is not root and o.parent is None:
        o.parent = root
# parametros para la caminata (los lee export_roblox.py)
sc = bpy.context.scene
sc['walk_frames'] = 42
sc['walk_stride'] = 0.72
sc['walk_lift'] = 0.28
sc['walk_crouch'] = 0.10

total = 0
for o in sorted(ROOT_COL.all_objects, key=lambda o: o.name):
    if o.type == 'MESH':
        t = tri_count(o)
        total += t
        print('%-28s %6d tris' % (o.name, t))
print('TOTAL', total)
bpy.ops.wm.save_as_mainfile(filepath=OUT)
print('guardado', OUT)
