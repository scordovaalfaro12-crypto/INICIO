"""Zephyrex — tercera y ultima forma de Zephyrian (Aetherials, tipo Neutro).
Zephyrian -> Zephalcon -> Zephyrex. El rey del viento: corona de plumas en abanico, capa de plumas sobre
los hombros, emblema dorado en la pechera, mascara-visor dorada con lagrima, alas enormes con doble banda
y puntas plateadas, cuatro estelas en la cola, botas de plumas y anillo de oro en el tarso.
Mismo esqueleto de ave que Zephalcon (las animaciones se reutilizan). Se modela a escala de Zephalcon y
al final todo se escala x1,35.
Ejecutar: python3 build_zephyrex.py [salida.blend]
"""
import bpy, bmesh, math, random, sys, os, json
WORK = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, WORK)
from mathutils import Vector, Matrix, Euler, Quaternion
from lib import *
from lib import _bridge

OUT = sys.argv[-1] if sys.argv[-1].endswith('.blend') else os.path.join(WORK, 'zephyrex.blend')
NAME = 'Zephyrex'
S = 1.35  # escala final respecto de Zephalcon

bpy.ops.wm.read_factory_settings(use_empty=True)
MATS = build_materials()
ROOT_COL = collection(NAME)
COLS = {k: collection(k + '_X', ROOT_COL) for k in ('Cuerpo', 'Cabeza', 'Alas', 'Patas', 'Cola')}
K = 1 / 0.575


def E(co, sx, sy, sz, rot=None, stiff=2.0):
    return ellipsoid(co, K, sx, sy, sz, rot, stiff)


def B(co, R, stiff=2.0):
    return ball(co, R * K, stiff)


def C(a, b, R, stiff=2.0):
    return capsule(a, b, R * K, stiff)


def part(name, coll, elems, tris, voxel=0.015, disp=None, res=0.015, sym=True, mat='AveCuerpo'):
    me = metaball_mesh(name, elems, res=res)
    o = new_object(name, me, coll, MATS)
    lowpoly(o, tris, voxel=voxel, disp=disp, sym=sym)
    set_material(o, MI[mat])
    return o


def set_pivot(o, pivot):
    pv = Vector(pivot)
    o.data.transform(Matrix.Translation(-pv))
    o.location = pv


def feather_frame(loc, d, up=(0, 0, 1)):
    y = Vector(d).normalized()
    up = Vector(up)
    z = up - y * up.dot(y)
    if z.length < 1e-4:
        z = Vector((0, 0, 1)) - y * y.z
    z.normalize()
    x = y.cross(z)
    return Matrix.Translation(Vector(loc)) @ Matrix((x, y, z)).transposed().to_4x4()


def surface_frame(loc, nor):
    z = Vector(nor).normalized()
    y = Vector((0, 0, 1)) - z * z.z
    y.normalize()
    x = y.cross(z)
    return Matrix.Translation(Vector(loc)) @ Matrix((x, y, z)).transposed().to_4x4()


PL = MI['AvePlata']

# =========================================================================== TORSO (hombros mas anchos)
torso = part('Torso', COLS['Cuerpo'], [
    E((0, 0.02, 1.15), 0.38, 0.34, 0.53, rot=(math.radians(34), 0, 0)),
    E((0, -0.19, 1.31), 0.34, 0.26, 0.37, rot=(math.radians(30), 0, 0)),
    E((0, 0.08, 0.88), 0.31, 0.30, 0.26),
    B((0.22, 0.10, 1.42), 0.21), B((-0.22, 0.10, 1.42), 0.21),
    E((0, 0.36, 0.80), 0.19, 0.21, 0.13),
], tris=1500, voxel=0.02, disp=dict(scale=3, strength=0.012, cells=0.012, cell_scale=5, seed=31))
paint_faces(torso, lambda c, n: MI['AvePecho'] if (n.y < -0.35 and 1.10 < c.z < 1.64 and abs(c.x) < 0.25) else None)
TORSO_BVH = bvh_of(torso)

# --- pechera de chevrones con emblema dorado al centro
pechera = new_object('Torso__Pechera', bpy.data.meshes.new('PecheraX'), COLS['Cuerpo'], MATS)
bm = bm_from(pechera)
rows = [(1.60, (0.0, 0.125)), (1.48, (0.065, 0.19)), (1.36, (0.0, 0.13, 0.24)), (1.24, (0.065, 0.18)),
        (1.12, (0.0, 0.12)), (1.01, (0.06,))]
for k, (z, xs) in enumerate(rows):
    for x in xs:
        loc, nor = surface_hit(TORSO_BVH, (x * 1.2, -2.0, z), (x * 0.4, 0.2, z))
        if loc is None:
            continue
        w = 0.17 - 0.010 * k
        M = surface_frame(loc + nor * (0.024 - 0.003 * k) + Vector((0, 0, 0.05)), nor)
        add_chevron(bm, M, width=w, height=0.17, thick=0.014,
                    mat=MI['AvePecho'] if (k + int(x * 20)) % 3 else MI['AvePechoSombra'], rim_mat=MI['AvePechoSombra'])
loc, nor = surface_hit(TORSO_BVH, (0, -2.0, 1.40), (0, 0.2, 1.40))
add_chevron(bm, surface_frame(loc + nor * 0.05 + Vector((0, 0, 0.06)), nor), width=0.2, height=0.22, thick=0.02,
            mat=MI['OroClaro'], rim_mat=MI['Oro'])
add_chevron(bm, surface_frame(loc + nor * 0.072 + Vector((0, 0, 0.035)), nor), width=0.09, height=0.11, thick=0.016,
            mat=MI['Oro'], rim_mat=MI['OroOscuro'])
bm_commit(pechera, bm)
symmetrize(pechera)
flat(pechera)

# --- capa de plumas escapulares sobre hombros y espalda
capa = new_object('Torso__Capa', bpy.data.meshes.new('Capa'), COLS['Cuerpo'], MATS)
bm = bm_from(capa)
rng = random.Random(32)
for row, (yb, zb, L, w, n_) in enumerate(((0.02, 1.62, 0.56, 0.17, 4), (0.12, 1.50, 0.50, 0.16, 4), (0.22, 1.34, 0.42, 0.15, 3))):
    for i in range(n_):
        x = 0.03 + i * 0.075 + (0.035 if row % 2 else 0)
        loc, nor = surface_hit(TORSO_BVH, (x, yb + 1.5, zb + 0.8), (x * 0.6, yb - 0.1, zb - 0.2))
        if loc is None:
            continue
        d = (Vector((x * 1.1, 0.42, -1.0)) + nor * 0.25).normalized()
        add_feather(bm, feather_frame(loc + nor * (0.03 + 0.012 * (2 - row)), d, nor), length=L * rng.uniform(0.94, 1.05),
                    width=w, thick=0.012, curve=0.08, mat=MI['AveAla'], tip_mat=PL, tip_frac=0.16,
                    band=(0.56, 0.7), band_mat=MI['AveAlaClara'])
bm_commit(capa, bm)
symmetrize(capa)
flat(capa)

TORSO_PIVOT = (0, 0.05, 1.10)
for o in (torso, pechera, capa):
    set_pivot(o, TORSO_PIVOT)

# =========================================================================== CUELLO + COLLAR (doble)
cuello = part('Cuello', COLS['Cuerpo'], [
    C((0, -0.13, 1.48), (0, -0.28, 1.80), 0.17),
    E((0, -0.24, 1.62), 0.18, 0.16, 0.17),
], tris=420, voxel=0.015, disp=dict(scale=4, strength=0.008, cells=0.008, cell_scale=6, seed=33))
paint_faces(cuello, lambda c, n: MI['AvePecho'] if n.y < -0.45 else None)
NECK_BVH = bvh_of(cuello)
collar = new_object('Cuello__Collar', bpy.data.meshes.new('CollarX'), COLS['Cuerpo'], MATS)
bm = bm_from(collar)
rng = random.Random(34)
center = Vector((0, -0.21, 1.66))
for th in range(0, 181, 18):
    t = math.radians(th)
    r = Vector((math.sin(t), -math.cos(t), 0))
    loc, nor = surface_hit(NECK_BVH, center + r * 1.5 + Vector((0, 0, 0.05)), center)
    if loc is None:
        continue
    d = (nor * 0.6 + Vector((0, 0, -1.0))).normalized()
    front = th < 70
    for lay, (ln, dz) in enumerate(((0.32, 0.0), (0.26, 0.07), (0.19, 0.13))):
        add_feather(bm, feather_frame(loc + Vector((0, 0, dz)) - nor * 0.01, d, nor), length=ln * rng.uniform(0.9, 1.1),
                    width=0.13, thick=0.01, curve=0.14,
                    mat=MI['AvePecho'] if front else MI['AveAlaClara'],
                    tip_mat=PL if front else MI['AveAla'])
bm_commit(collar, bm)
symmetrize(collar)
flat(collar)
NECK_PIVOT = (0, -0.14, 1.50)
set_pivot(cuello, NECK_PIVOT)
set_pivot(collar, NECK_PIVOT)

# =========================================================================== CABEZA
HP = Vector((0, -0.29, 1.80))
craneo = part('Craneo', COLS['Cabeza'], [
    E((0, -0.36, 1.96), 0.20, 0.22, 0.19),
    B((0.095, -0.42, 1.90), 0.125), B((-0.095, -0.42, 1.90), 0.125),
    E((0.09, -0.48, 2.01), 0.08, 0.095, 0.05), E((-0.09, -0.48, 2.01), 0.08, 0.095, 0.05),
    E((0, -0.27, 1.91), 0.15, 0.15, 0.15),
], tris=950, voxel=0.012, res=0.012, disp=dict(scale=5, strength=0.006, cells=0.006, cell_scale=8, seed=35))
paint_faces(craneo, lambda c, n: MI['AvePecho'] if (n.z < -0.35 and c.y < -0.33) else None)
HEAD_BVH = bvh_of(craneo)
bm = bm_from(craneo)
# pico superior mas grande, con gancho oscuro
add_tube(bm, [Vector(p) for p in ((0, -0.50, 1.975), (0, -0.62, 1.965), (0, -0.72, 1.935), (0, -0.80, 1.88),
                                  (0, -0.815, 1.81), (0, -0.80, 1.77))],
         [0.115, 0.098, 0.072, 0.046, 0.021, 0.004], sides=6, side_scale=0.75,
         mat_fn=lambda t: MI['OroClaro'] if t < 0.18 else (MI['GarraOscura'] if t > 0.82 else MI['Oro']))
bm_commit(craneo, bm)
symmetrize(craneo)
flat(craneo)

# --- ojos + mascara-visor dorada (franja hacia atras y lagrima bajo el ojo)
ojos = new_object('Ojos', bpy.data.meshes.new('OjosX'), COLS['Cabeza'], MATS)
bm = bm_from(ojos)
eye_loc, eye_nor = surface_hit(HEAD_BVH, (0.64, -0.80, 2.04), (0, -0.37, 1.97))
print('ojo', eye_loc, eye_nor)
ez = eye_nor.normalized()
ey = (Vector((0, 0, 1)) - ez * ez.z).normalized()
ex = ey.cross(ez)
R = Matrix((ex, ey, ez)).transposed()


def disc(radius, lift, mat, sides=8, sx=1.0, off=(0, 0)):
    vs = []
    for i in range(sides):
        a = 2 * math.pi * i / sides + math.pi / sides
        p = eye_loc + R @ Vector((math.cos(a) * radius * sx + off[0], math.sin(a) * radius + off[1], 0)) + ez * lift
        vs.append(bm.verts.new(p))
    f = bm.faces.new(vs)
    f.material_index = mat
    return vs


ring = disc(0.066, 0.006, MI['OroClaro'], 8, 1.15)
back = [bm.verts.new(v.co - ez * 0.03) for v in ring]
_bridge(bm, back, ring, MI['Oro'])
disc(0.047, 0.010, MI['OjoAve'], 8, 1.05)
disc(0.023, 0.014, MI['Pupila'], 6)
disc(0.008, 0.017, MI['Brillo'], 4, off=(-0.014, 0.014))
for (dx, dy, dirv, L, w) in ((-0.075, 0.035, (0.15, 1.0, 0.40), 0.24, 0.06),   # franja hacia atras
                             (-0.02, -0.05, (0.05, 0.35, -1.0), 0.14, 0.05)):  # lagrima (bigote de halcon)
    p0 = eye_loc + R @ Vector((dx, dy, 0))
    ml, mn = surface_hit(HEAD_BVH, p0 + ez * 0.3, p0 - ez * 0.1)
    if ml is None:
        continue
    d = Vector(dirv)
    d = (d - mn * d.dot(mn)).normalized()
    add_feather(bm, feather_frame(ml + mn * 0.006, d, mn), length=L, width=w, thick=0.006,
                mat=MI['OroClaro'], tip_mat=MI['Oro'])
bm_commit(ojos, bm)
symmetrize(ojos)
flat(ojos)

# --- corona: cresta en abanico de 7 plumas detras de la cabeza
cresta = new_object('Cresta', bpy.data.meshes.new('CrestaX'), COLS['Cabeza'], MATS)
bm = bm_from(cresta)
for k, a_deg in enumerate((0, 20, 40, 60)):
    a = math.radians(a_deg)
    bx = math.sin(a) * 0.07
    loc, nor = surface_hit(HEAD_BVH, (bx, -0.28, 3.0), (bx, -0.30, 1.6))
    if loc is None:
        continue
    d = Vector((math.sin(a) * 0.95, 0.5, math.cos(a) * 0.95)).normalized()
    L = 0.62 - 0.07 * k
    add_feather(bm, feather_frame(loc - d * 0.04, d, (0, -1, 0.3)), length=L, width=0.11, thick=0.013, curve=-0.10,
                mat=MI['AveCuerpo'], tip_mat=PL if k else MI['OroClaro'], tip_frac=0.22,
                band=(0.58, 0.70), band_mat=MI['AveAlaClara'] if k else MI['Oro'])
bm_commit(cresta, bm)
symmetrize(cresta)
flat(cresta)
for o in (craneo, ojos, cresta):
    set_pivot(o, HP)

# =========================================================================== PICO INFERIOR
mand = new_object('Mandibula', bpy.data.meshes.new('MandX'), COLS['Cabeza'], MATS)
bm = bm_from(mand)
add_tube(bm, [Vector(p) for p in ((0, -0.47, 1.90), (0, -0.62, 1.88), (0, -0.71, 1.865), (0, -0.76, 1.86))],
         [0.085, 0.068, 0.04, 0.009], sides=6, side_scale=0.75, flatten=0.6, mat=MI['PicoNaranja'])
bm_commit(mand, bm)
flat(mand)
JAW_PIVOT = (0, -0.47, 1.905)
set_pivot(mand, JAW_PIVOT)

# =========================================================================== ALAS enormes
WS = Vector((0.26, 0.06, 1.44))
WE = Vector((0.70, 0.10, 1.48))
WW = Vector((1.22, 0.06, 1.50))
WT = Vector((1.78, -0.03, 1.49))


def wing_part(name, a, b, r0, r1, feathers, seed):
    o = part(name, COLS['Alas'], [B(a, r0), C(a, b, (r0 + r1) / 2), B(b, r1)], tris=170, voxel=0.014,
             sym=False, mat='AveAla', disp=dict(scale=5, strength=0.006, cells=0.0, seed=seed))
    bm = bm_from(o)
    rng = random.Random(seed)
    for (t, d, L, w, mat, band, tip, dz) in feathers:
        base = a.lerp(b, t) + Vector((0, 0.025, dz))
        add_feather(bm, feather_frame(base, d, (0, 0, 1)), length=L * rng.uniform(0.96, 1.04), width=w, thick=0.012,
                    curve=-0.04, mat=mat, tip_mat=tip, tip_frac=0.12, band=band, band_mat=MI['AvePecho'])
    bm_commit(o, bm)
    flat(o)
    set_pivot(o, a)
    return o


feat_brazo = [(t, Vector((0.12, 1, -0.04)), 0.46, 0.19, MI['AveAla'], None, MI['AveAlaClara'], 0.012 * i)
              for i, t in enumerate((0.08, 0.28, 0.48, 0.68, 0.88, 1.0))]
feat_ante = []
for i in range(11):
    t = i / 10
    feat_ante.append((t, Vector((0.06 + 0.12 * t, 1, -0.05)), 0.90 - 0.07 * t, 0.22, MI['AveAla'], (0.58, 0.70),
                      PL, -0.004 * i))
for i in range(8):
    t = 0.04 + i / 8
    feat_ante.append((t, Vector((0.1 + 0.1 * t, 1, -0.02)), 0.40, 0.18, MI['AveAlaClara'], None, MI['AveAla'],
                      0.02 + 0.002 * i))
feat_mano = []
for i in range(8):
    t = i / 7
    ang = math.radians(12 + 64 * t)
    d = Vector((math.sin(ang), math.cos(ang), -0.03))
    L = 0.88 + 0.22 * math.sin(math.pi * min(1, t * 1.15))
    feat_mano.append((t, d, L, 0.2, MI['AveAla'], (0.56, 0.68), PL, 0.008 * i))
for i in range(5):
    t = 0.04 + i / 5
    feat_mano.append((t, Vector((0.35 + 0.4 * t, 1, 0)), 0.34, 0.16, MI['AveAlaClara'], None, MI['AveAla'], 0.05))
for i in range(3):  # alula: plumitas hacia adelante en la muneca
    feat_mano.append((0.02, Vector((0.55 + 0.15 * i, -0.15 + 0.12 * i, 0.02)), 0.22 - 0.03 * i, 0.08, MI['AveAlaClara'],
                      None, PL, 0.06 + 0.006 * i))

ala_L = [wing_part('Ala_Izq_Brazo', WS, WE, 0.085, 0.07, feat_brazo, 41),
         wing_part('Ala_Izq_Antebrazo', WE, WW, 0.075, 0.058, feat_ante, 42),
         wing_part('Ala_Izq_Mano', WW, WT, 0.058, 0.032, feat_mano, 43)]

# =========================================================================== PATAS (botas de plumas, anillo de oro)
LH = Vector((0.18, 0.10, 0.92))
LA = Vector((0.21, 0.23, 0.50))
LF = Vector((0.22, 0.06, 0.11))
muslo = part('Pata_Izq_Muslo', COLS['Patas'], [B(LH, 0.14), C(LH, LA + Vector((0, 0, 0.08)), 0.13),
                                                 E((LH + LA) / 2 + Vector((0.01, 0.02, -0.02)), 0.13, 0.13, 0.17)],
             tris=280, voxel=0.015, sym=False, disp=dict(scale=5, strength=0.006, seed=51))
bm = bm_from(muslo)
mb = bvh_of(muslo)
for k in range(8):
    a = 2 * math.pi * k / 8
    p = LA + Vector((math.cos(a) * 0.1, math.sin(a) * 0.1, 0.16))
    loc, nor = surface_hit(mb, p + Vector((math.cos(a), math.sin(a), 0)) * 1.0, LA + Vector((0, 0, 0.16)))
    if loc is None:
        continue
    d = (nor * 0.3 + Vector((0, 0, -1))).normalized()
    add_feather(bm, feather_frame(loc - nor * 0.01, d, nor), length=0.2, width=0.1, thick=0.008, curve=0.1,
                mat=MI['AveCuerpo'], tip_mat=MI['AveAla'])
bm_commit(muslo, bm)
flat(muslo)
set_pivot(muslo, LH)

tarso = new_object('Pata_Izq_Tarso', bpy.data.meshes.new('TarsoX'), COLS['Patas'], MATS)
bm = bm_from(tarso)
pts = [LA.lerp(LF, t) for t in (0, 0.25, 0.5, 0.75, 1.0)]
add_tube(bm, pts, [0.068, 0.06, 0.055, 0.055, 0.06], sides=6,
         mat_fn=lambda t: MI['OroOscuro'] if t < 0.1 else (MI['Oro'] if int(t * 8) % 2 else MI['OroClaro']))
ring_c = LA.lerp(LF, 0.68)
tang = (LF - LA).normalized()
add_tube(bm, [ring_c - tang * 0.025, ring_c + tang * 0.025], [0.075, 0.075], sides=8, mat=MI['OroClaro'])
for k in range(7):  # botas: plumas que cubren la parte alta del tarso
    a = 2 * math.pi * k / 7 + 0.3
    radial = Vector((math.cos(a), math.sin(a), 0))
    radial = (radial - tang * radial.dot(tang)).normalized()
    base = LA + tang * 0.03 + radial * 0.06
    d = (tang * 1.0 + radial * 0.25).normalized()
    add_feather(bm, feather_frame(base, d, radial), length=0.17, width=0.09, thick=0.008, curve=0.06,
                mat=MI['AveCuerpo'], tip_mat=MI['AveAla'])
bm_commit(tarso, bm)
flat(tarso)
set_pivot(tarso, LA)

pie = new_object('Pata_Izq_Pie', bpy.data.meshes.new('PieX'), COLS['Patas'], MATS)
garras = new_object('Pata_Izq_Garras', bpy.data.meshes.new('GarrasX'), COLS['Patas'], MATS)
bm = bm_from(pie)
bg = bm_from(garras)
rng = random.Random(53)
F0 = Vector((LF.x, LF.y, 0.05))
bmesh.ops.create_icosphere(bm, subdivisions=1, radius=0.075, matrix=Matrix.Translation(F0 + Vector((0, 0, 0.02))))
for f in bm.faces:
    f.material_index = MI['Oro']
for k, (ang, L) in enumerate(((-0.42, 0.23), (0.0, 0.27), (0.42, 0.23), (math.pi, 0.14))):
    d = Vector((math.sin(ang), -math.cos(ang), 0))
    p1 = F0 + d * L * 0.5 + Vector((0, 0, 0.012))
    p2 = F0 + d * L + Vector((0, 0, -0.01))
    add_tube(bm, [F0, p1, p2], [0.04, 0.033, 0.026], sides=5, mat=MI['Oro'])
    M = Matrix.Translation(p2) @ Matrix.Rotation(math.atan2(-d.x, d.y), 4, 'Z') @ Matrix.Rotation(math.radians(10), 4, 'X')
    add_claw(bg, M @ Matrix.Diagonal((0.095, 0.095, 0.095, 1)), rng, length=1.0, curve=0.6, base=0.28,
             mat=MI['GarraOscura'])
bm_commit(pie, bm)
bm_commit(garras, bg)
for o in (pie, garras):
    flat(o)
    set_pivot(o, LF)
pata_L = [muslo, tarso, pie, garras]

for src in ala_L + pata_L:
    o = mirror_object_x(src, src.name.replace('_Izq', '_Der'), src.users_collection[0])
    o.location = Vector((-src.location.x, src.location.y, src.location.z))

# =========================================================================== COLA: abanico + cuatro estelas
cola = new_object('Cola', bpy.data.meshes.new('ColaX'), COLS['Cola'], MATS)
bm = bm_from(cola)
TP = Vector((0, 0.38, 0.80))
for i in range(5):
    x = 0.035 * i
    d = Vector((x * 2.6, 1.0, -0.62)).normalized()
    add_feather(bm, feather_frame(TP + Vector((x, 0.02, -0.006 * i)), d, (0, 0.5, 1)), length=0.70 - 0.03 * i,
                width=0.17, thick=0.011, curve=-0.05, mat=MI['AveAla'], tip_mat=PL, tip_frac=0.12,
                band=(0.6, 0.74), band_mat=MI['AvePecho'])
# estelas: dos centrales largas con banda dorada y dos exteriores que se curvan como cintas
for (x, L, w, tw, cv, dirx, band) in ((0.025, 1.55, 0.09, 0.5, 0.05, 0.05, (0.66, 0.74)),
                                      (0.10, 1.25, 0.08, 0.9, -0.10, 0.35, None)):
    d = Vector((dirx, 1.0, -0.48)).normalized()
    add_feather(bm, feather_frame(TP + Vector((x, 0.0, 0.03)), d, (0, 0.45, 1)), length=L, width=w, thick=0.011,
                curve=cv, twist=tw, mat=MI['AveCuerpo'], tip_mat=PL, tip_frac=0.2,
                band=band, band_mat=MI['Oro'])
for i in range(4):
    x = 0.045 * i
    d = Vector((x * 2.0, 1.0, -0.45)).normalized()
    add_feather(bm, feather_frame(TP + Vector((x, -0.04, 0.05)), d, (0, 0.4, 1)), length=0.32, width=0.13, thick=0.01,
                mat=MI['AveAlaClara'], tip_mat=MI['AveAla'])
bm_commit(cola, bm)
symmetrize(cola)
flat(cola)
set_pivot(cola, TP)

# =========================================================================== escala final x1,35
SCALE_M = Matrix.Diagonal((S, S, S, 1))
for o in ROOT_COL.all_objects:
    if o.type == 'MESH':
        o.data.transform(SCALE_M)
        o.location = o.location * S

root = bpy.data.objects.new(NAME, None)
ROOT_COL.objects.link(root)
root.empty_display_type = 'ARROWS'
for o in ROOT_COL.all_objects:
    if o is not root and o.parent is None:
        o.parent = root


def v3(v):
    return [round(c * S, 4) for c in Vector(v)]


BONES = [('Raiz', (0, 0.1, 0), (0, -0.3, 0), None),
         ('Torso', TORSO_PIVOT, (0, -0.2, 1.5), 'Raiz'),
         ('Cuello', NECK_PIVOT, HP, 'Torso'),
         ('Cabeza', HP, (0, -0.64, 1.93), 'Cuello'),
         ('Mandibula', JAW_PIVOT, (0, -0.76, 1.86), 'Cabeza'),
         ('Cola', TP, (0, 0.9, 0.5), 'Torso')]
for side, sx in (('Izq', 1), ('Der', -1)):
    m = Vector((sx, 1, 1))

    def mm(v):
        return Vector(v) * m
    BONES += [('Ala_%s_Brazo' % side, mm(WS), mm(WE), 'Torso'),
              ('Ala_%s_Antebrazo' % side, mm(WE), mm(WW), 'Ala_%s_Brazo' % side),
              ('Ala_%s_Mano' % side, mm(WW), mm(WT), 'Ala_%s_Antebrazo' % side),
              ('Pata_%s_Muslo' % side, mm(LH), mm(LA), 'Torso'),
              ('Pata_%s_Tarso' % side, mm(LA), mm(LF), 'Pata_%s_Muslo' % side),
              ('Pata_%s_Pie' % side, mm(LF), mm(LF + Vector((0, -0.22, -0.06))), 'Pata_%s_Tarso' % side)]
part_bone = {}
for o in ROOT_COL.all_objects:
    if o.type != 'MESH':
        continue
    n = o.name
    if '__' in n:
        part_bone[n] = n.split('__')[0]
    elif n in ('Craneo', 'Ojos', 'Cresta'):
        part_bone[n] = 'Cabeza'
    elif n.endswith('_Garras'):
        part_bone[n] = n.replace('_Garras', '_Pie')
    else:
        part_bone[n] = n
sc = bpy.context.scene
sc['tipo'] = 'ave'
sc['rig_bones'] = json.dumps([[n, v3(h), v3(t), p] for n, h, t, p in BONES])
sc['part_bone'] = json.dumps(part_bone)
# plegado propio: las primarias son mas largas, la mano se pliega mas hacia atras para no arrastrar
sc['wing_fold'] = json.dumps({'Brazo': [[0.24, 0.62, -0.75], [0.25, 0.45, -1.0]],
                              'Antebrazo': [[0.10, -0.85, 0.50], [0.22, 0.50, -1.0]],
                              'Mano': [[0.04, 0.95, -0.30], [0.16, 0.95, -0.75]]})
sc['walk_frames'] = 34
sc['walk_stride'] = 0.40 * S
sc['walk_lift'] = 0.15 * S
sc['walk_crouch'] = 0.05 * S
sc['cam_scale'] = 0.62 * S * 1.2
sc['cam_center'] = (0.0, 0.15 * S, 0.0)
sc['cam_zoff'] = 0.62 * S * 1.12
sc['walk_lane'] = (0.10 * S, 0.36 * S)

total = 0
for o in sorted(ROOT_COL.all_objects, key=lambda o: o.name):
    if o.type == 'MESH':
        t = tri_count(o)
        total += t
        print('%-28s %6d tris' % (o.name, t))
print('TOTAL', total)
bpy.ops.wm.save_as_mainfile(filepath=OUT)
print('guardado', OUT)
