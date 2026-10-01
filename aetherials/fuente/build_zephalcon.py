"""Zephalcon — segunda forma de Zephyrian (Aetherials, tipo Neutro).
Ave rapaz erguida: pechera de chevrones, collar de plumas, cresta larga, mascara dorada, pico ganchudo,
alas de 3 segmentos con plumas (pose de reposo = alas abiertas, las animaciones las pliegan),
patas con 'pantalones' de plumas, tarsos dorados y garras, cola en abanico con dos estelas.
Ejecutar: python3 build_zephalcon.py [salida.blend]
"""
import bpy, bmesh, math, random, sys, os, json
WORK = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, WORK)
from mathutils import Vector, Matrix, Euler, Quaternion
from lib import *
from lib import _bridge

OUT = sys.argv[-1] if sys.argv[-1].endswith('.blend') else os.path.join(WORK, 'zephalcon.blend')
NAME = 'Zephalcon'

bpy.ops.wm.read_factory_settings(use_empty=True)
MATS = build_materials()
ROOT_COL = collection(NAME)
COLS = {k: collection(k + '_Z', ROOT_COL) for k in ('Cuerpo', 'Cabeza', 'Alas', 'Patas', 'Cola')}
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
    """Matriz para add_feather: +Y = direccion de la pluma, +Z = normal de la lamina (hacia 'up')."""
    y = Vector(d).normalized()
    up = Vector(up)
    z = up - y * up.dot(y)
    if z.length < 1e-4:
        z = Vector((0, 0, 1)) - y * y.z
    z.normalize()
    x = y.cross(z)
    return Matrix.Translation(Vector(loc)) @ Matrix((x, y, z)).transposed().to_4x4()


def surface_frame(loc, nor):
    """Para add_chevron: +Z = normal, +Y = 'arriba' sobre la superficie (la V apunta hacia abajo)."""
    z = Vector(nor).normalized()
    y = Vector((0, 0, 1)) - z * z.z
    y.normalize()
    x = y.cross(z)
    return Matrix.Translation(Vector(loc)) @ Matrix((x, y, z)).transposed().to_4x4()


# =========================================================================== TORSO
torso = part('Torso', COLS['Cuerpo'], [
    E((0, 0.02, 1.15), 0.36, 0.33, 0.52, rot=(math.radians(34), 0, 0)),
    E((0, -0.18, 1.30), 0.32, 0.25, 0.36, rot=(math.radians(30), 0, 0)),
    E((0, 0.08, 0.88), 0.30, 0.30, 0.26),
    B((0.20, 0.10, 1.40), 0.19), B((-0.20, 0.10, 1.40), 0.19),
    E((0, 0.36, 0.80), 0.18, 0.20, 0.13),
], tris=1400, voxel=0.02, disp=dict(scale=3, strength=0.012, cells=0.012, cell_scale=5, seed=1))
paint_faces(torso, lambda c, n: MI['AvePecho'] if (n.y < -0.35 and 1.12 < c.z < 1.62 and abs(c.x) < 0.24) else None)
TORSO_BVH = bvh_of(torso)

# --- pechera de chevrones (como Zephyrian, pero en capas de armadura)
pechera = new_object('Torso__Pechera', bpy.data.meshes.new('Pechera'), COLS['Cuerpo'], MATS)
bm = bm_from(pechera)
rows = [(1.58, (0.0, 0.12)), (1.46, (0.065, 0.18)), (1.34, (0.0, 0.13)), (1.22, (0.065,)), (1.11, (0.0,))]
for k, (z, xs) in enumerate(rows):
    for x in xs:
        loc, nor = surface_hit(TORSO_BVH, (x * 1.2, -2.0, z), (x * 0.4, 0.2, z))
        if loc is None:
            continue
        w = 0.17 - 0.012 * k
        M = surface_frame(loc + nor * (0.022 - 0.003 * k) + Vector((0, 0, 0.05)), nor)
        add_chevron(bm, M, width=w, height=0.17, thick=0.014,
                    mat=MI['AvePecho'] if (k + int(x * 20)) % 3 else MI['AvePechoSombra'], rim_mat=MI['AvePechoSombra'])
bm_commit(pechera, bm)
symmetrize(pechera)
flat(pechera)

TORSO_PIVOT = (0, 0.05, 1.10)
set_pivot(torso, TORSO_PIVOT)
set_pivot(pechera, TORSO_PIVOT)

# =========================================================================== CUELLO + COLLAR
cuello = part('Cuello', COLS['Cuerpo'], [
    C((0, -0.13, 1.48), (0, -0.28, 1.80), 0.16),
    E((0, -0.24, 1.62), 0.17, 0.15, 0.16),
], tris=420, voxel=0.015, disp=dict(scale=4, strength=0.008, cells=0.008, cell_scale=6, seed=2))
paint_faces(cuello, lambda c, n: MI['AvePecho'] if n.y < -0.45 else None)
NECK_BVH = bvh_of(cuello)
collar = new_object('Cuello__Collar', bpy.data.meshes.new('Collar'), COLS['Cuerpo'], MATS)
bm = bm_from(collar)
rng = random.Random(3)
center = Vector((0, -0.21, 1.66))
for th in range(0, 181, 22):
    t = math.radians(th)
    r = Vector((math.sin(t), -math.cos(t), 0))  # 0 = frente, 180 = nuca
    loc, nor = surface_hit(NECK_BVH, center + r * 1.5 + Vector((0, 0, 0.05)), center)
    if loc is None:
        continue
    d = (nor * 0.55 + Vector((0, 0, -1.0))).normalized()
    front = th < 70
    for lay, (ln, dz) in enumerate(((0.26, 0.0), (0.20, 0.07))):
        add_feather(bm, feather_frame(loc + Vector((0, 0, dz)) - nor * 0.01, d, nor), length=ln * rng.uniform(0.9, 1.1),
                    width=0.12, thick=0.01, curve=0.12,
                    mat=MI['AvePecho'] if front else MI['AveAlaClara'],
                    tip_mat=MI['AvePechoSombra'] if front else MI['AveAla'])
bm_commit(collar, bm)
symmetrize(collar)
flat(collar)
NECK_PIVOT = (0, -0.14, 1.50)
set_pivot(cuello, NECK_PIVOT)
set_pivot(collar, NECK_PIVOT)

# =========================================================================== CABEZA
HP = Vector((0, -0.29, 1.80))
craneo = part('Craneo', COLS['Cabeza'], [
    E((0, -0.36, 1.95), 0.19, 0.21, 0.18),
    B((0.09, -0.42, 1.90), 0.12), B((-0.09, -0.42, 1.90), 0.12),
    E((0.085, -0.47, 2.00), 0.075, 0.09, 0.045), E((-0.085, -0.47, 2.00), 0.075, 0.09, 0.045),
    E((0, -0.27, 1.90), 0.14, 0.14, 0.14),
], tris=900, voxel=0.012, res=0.012, disp=dict(scale=5, strength=0.006, cells=0.006, cell_scale=8, seed=4))
paint_faces(craneo, lambda c, n: MI['AvePecho'] if (n.z < -0.35 and c.y < -0.33) else None)
HEAD_BVH = bvh_of(craneo)
bm = bm_from(craneo)
# pico superior ganchudo (cera clara en la base, punta oscura)
add_tube(bm, [Vector(p) for p in ((0, -0.495, 1.965), (0, -0.60, 1.955), (0, -0.69, 1.93), (0, -0.76, 1.88),
                                  (0, -0.775, 1.82), (0, -0.765, 1.785))],
         [0.10, 0.085, 0.062, 0.040, 0.018, 0.004], sides=6, side_scale=0.75,
         mat_fn=lambda t: MI['OroClaro'] if t < 0.18 else (MI['OroOscuro'] if t > 0.8 else MI['Oro']))
bm_commit(craneo, bm)
symmetrize(craneo)
flat(craneo)

# --- ojos: anillo dorado, iris ambar, pupila y brillo + mascara dorada hacia atras
ojos = new_object('Ojos', bpy.data.meshes.new('OjosZ'), COLS['Cabeza'], MATS)
bm = bm_from(ojos)
eye_loc, eye_nor = surface_hit(HEAD_BVH, (0.62, -0.78, 2.03), (0, -0.37, 1.96))
print('ojo', eye_loc, eye_nor)
ez = eye_nor.normalized()
ey = (Vector((0, 0, 1)) - ez * ez.z).normalized()
ex = ey.cross(ez)
R = Matrix((ex, ey, ez)).transposed()


def disc(radius, lift, mat, sides=8, sx=1.0):
    vs = []
    for i in range(sides):
        a = 2 * math.pi * i / sides + math.pi / sides
        p = eye_loc + R @ Vector((math.cos(a) * radius * sx, math.sin(a) * radius, 0)) + ez * lift
        vs.append(bm.verts.new(p))
    f = bm.faces.new(vs)
    f.material_index = mat
    return vs


ring = disc(0.062, 0.006, MI['OroClaro'], 8, 1.1)
back = [bm.verts.new(v.co - ez * 0.03) for v in ring]
_bridge(bm, back, ring, MI['Oro'])
disc(0.045, 0.010, MI['OjoAve'], 8, 1.05)
disc(0.022, 0.014, MI['Pupila'], 6)
gl = disc(0.008, 0.017, MI['Brillo'], 4)
for v in gl:
    v.co += R @ Vector((-0.014, 0.014, 0))
# mascara: franja dorada que sale del ojo hacia atras y arriba (marca de la segunda forma)
ml, mn = surface_hit(HEAD_BVH, eye_loc + R @ Vector((-0.07, 0.035, 0.0)) + ez * 0.3, eye_loc + R @ Vector((-0.07, 0.035, 0)) - ez * 0.1)
if ml is not None:
    d = (Vector((0.15, 1.0, 0.35)) - mn * Vector((0.15, 1.0, 0.35)).dot(mn)).normalized()
    add_feather(bm, feather_frame(ml + mn * 0.006, d, mn), length=0.17, width=0.055, thick=0.006,
                mat=MI['OroClaro'], tip_mat=MI['Oro'])
bm_commit(ojos, bm)
symmetrize(ojos)
flat(ojos)

# --- cresta: 5 plumas largas barridas hacia atras
cresta = new_object('Cresta', bpy.data.meshes.new('Cresta'), COLS['Cabeza'], MATS)
bm = bm_from(cresta)
for (x, y, L, w, tip) in ((0.0, -0.33, 0.46, 0.10, MI['AvePecho']), (0.055, -0.30, 0.40, 0.09, MI['AveAlaClara']),
                          (0.10, -0.25, 0.32, 0.085, MI['AveAlaClara'])):
    loc, nor = surface_hit(HEAD_BVH, (x, y, 3.0), (x, y, 1.5))
    if loc is None:
        continue
    d = Vector((x * 1.6, 0.85, 0.55)).normalized()
    add_feather(bm, feather_frame(loc - d * 0.03, d, (0, -0.4, 1)), length=L, width=w, thick=0.012, curve=-0.12,
                mat=MI['AveCuerpo'], tip_mat=tip, tip_frac=0.3)
bm_commit(cresta, bm)
symmetrize(cresta)
flat(cresta)
for o in (craneo, ojos, cresta):
    set_pivot(o, HP)

# =========================================================================== PICO INFERIOR (mandibula)
mand = new_object('Mandibula', bpy.data.meshes.new('MandZ'), COLS['Cabeza'], MATS)
bm = bm_from(mand)
add_tube(bm, [Vector(p) for p in ((0, -0.47, 1.895), (0, -0.60, 1.875), (0, -0.68, 1.86), (0, -0.725, 1.855))],
         [0.075, 0.06, 0.035, 0.008], sides=6, side_scale=0.75, flatten=0.6, mat=MI['PicoNaranja'])
bm_commit(mand, bm)
flat(mand)
JAW_PIVOT = (0, -0.47, 1.90)
set_pivot(mand, JAW_PIVOT)

# =========================================================================== ALAS (pose de reposo: abiertas)
WS = Vector((0.24, 0.06, 1.42))
WE = Vector((0.62, 0.10, 1.46))
WW = Vector((1.06, 0.06, 1.48))
WT = Vector((1.52, -0.02, 1.47))


def wing_part(name, a, b, r0, r1, feathers, seed):
    o = part(name, COLS['Alas'], [B(a, r0), C(a, b, (r0 + r1) / 2), B(b, r1)], tris=160, voxel=0.014,
             sym=False, mat='AveAla', disp=dict(scale=5, strength=0.006, cells=0.0, seed=seed))
    bm = bm_from(o)
    rng = random.Random(seed)
    for (t, d, L, w, mat, band, tip, dz) in feathers:
        base = a.lerp(b, t) + Vector((0, 0.025, dz))
        add_feather(bm, feather_frame(base, d, (0, 0, 1)), length=L * rng.uniform(0.96, 1.04), width=w, thick=0.011,
                    curve=-0.04, mat=mat, tip_mat=tip, tip_frac=0.14, band=band, band_mat=MI['AvePecho'])
    bm_commit(o, bm)
    flat(o)
    set_pivot(o, a)
    return o


feat_brazo = [(t, Vector((0.12, 1, -0.04)), 0.40, 0.17, MI['AveAla'], None, MI['AveAlaClara'], 0.012 * i)
              for i, t in enumerate((0.1, 0.32, 0.54, 0.76, 0.98))]
feat_ante = []
for i in range(9):
    t = i / 8
    feat_ante.append((t, Vector((0.06 + 0.12 * t, 1, -0.05)), 0.78 - 0.06 * t, 0.2, MI['AveAla'], (0.66, 0.80),
                      MI['AveCuerpo'], -0.004 * i))
for i in range(6):
    t = 0.05 + i / 6
    feat_ante.append((t, Vector((0.1 + 0.1 * t, 1, -0.02)), 0.34, 0.16, MI['AveAlaClara'], None, MI['AveAla'],
                      0.02 + 0.002 * i))
feat_mano = []
for i in range(7):
    t = i / 6
    ang = math.radians(14 + 62 * t)  # de atras hacia afuera
    d = Vector((math.sin(ang), math.cos(ang), -0.03))
    L = 0.80 + 0.26 * math.sin(math.pi * min(1, t * 1.15))
    feat_mano.append((t, d, L, 0.18, MI['AveAla'], (0.62, 0.76), MI['AveCuerpo'], 0.008 * i))
for i in range(4):
    t = 0.05 + i / 4
    feat_mano.append((t, Vector((0.35 + 0.4 * t, 1, 0)), 0.30, 0.14, MI['AveAlaClara'], None, MI['AveAla'], 0.05))

ala_L = [wing_part('Ala_Izq_Brazo', WS, WE, 0.08, 0.065, feat_brazo, 11),
         wing_part('Ala_Izq_Antebrazo', WE, WW, 0.07, 0.055, feat_ante, 12),
         wing_part('Ala_Izq_Mano', WW, WT, 0.055, 0.03, feat_mano, 13)]

# =========================================================================== PATAS
LH = Vector((0.17, 0.10, 0.92))
LA = Vector((0.20, 0.23, 0.50))
LF = Vector((0.21, 0.06, 0.11))
muslo = part('Pata_Izq_Muslo', COLS['Patas'], [B(LH, 0.13), C(LH, LA + Vector((0, 0, 0.08)), 0.12),
                                                 E((LH + LA) / 2 + Vector((0.01, 0.02, -0.02)), 0.12, 0.12, 0.16)],
             tris=260, voxel=0.015, sym=False, disp=dict(scale=5, strength=0.006, seed=21))
bm = bm_from(muslo)
rng = random.Random(22)
mb = bvh_of(muslo)
for k in range(7):  # 'pantalones' de plumas
    a = 2 * math.pi * k / 7
    p = LA + Vector((math.cos(a) * 0.1, math.sin(a) * 0.1, 0.16))
    loc, nor = surface_hit(mb, p + Vector((math.cos(a), math.sin(a), 0)) * 1.0, LA + Vector((0, 0, 0.16)))
    if loc is None:
        continue
    d = (nor * 0.35 + Vector((0, 0, -1))).normalized()
    add_feather(bm, feather_frame(loc - nor * 0.01, d, nor), length=0.16, width=0.09, thick=0.008, curve=0.1,
                mat=MI['AveCuerpo'], tip_mat=MI['AveAla'])
bm_commit(muslo, bm)
flat(muslo)
set_pivot(muslo, LH)

tarso = new_object('Pata_Izq_Tarso', bpy.data.meshes.new('TarsoZ'), COLS['Patas'], MATS)
bm = bm_from(tarso)
pts = [LA.lerp(LF, t) for t in (0, 0.25, 0.5, 0.75, 1.0)]
add_tube(bm, pts, [0.06, 0.052, 0.048, 0.048, 0.052], sides=6,
         mat_fn=lambda t: MI['OroOscuro'] if t < 0.1 else (MI['Oro'] if int(t * 8) % 2 else MI['OroClaro']))
bm_commit(tarso, bm)
flat(tarso)
set_pivot(tarso, LA)

pie = new_object('Pata_Izq_Pie', bpy.data.meshes.new('PieZ'), COLS['Patas'], MATS)
garras = new_object('Pata_Izq_Garras', bpy.data.meshes.new('GarrasZ'), COLS['Patas'], MATS)
bm = bm_from(pie)
bg = bm_from(garras)
rng = random.Random(23)
F0 = Vector((LF.x, LF.y, 0.045))
bmesh.ops.create_icosphere(bm, subdivisions=1, radius=0.065, matrix=Matrix.Translation(F0 + Vector((0, 0, 0.02))))
for f in bm.faces:
    f.material_index = MI['Oro']
for k, (ang, L) in enumerate(((-0.42, 0.20), (0.0, 0.24), (0.42, 0.20), (math.pi, 0.12))):
    d = Vector((math.sin(ang), -math.cos(ang), 0))
    p1 = F0 + d * L * 0.5 + Vector((0, 0, 0.012))
    p2 = F0 + d * L + Vector((0, 0, -0.008))
    add_tube(bm, [F0, p1, p2], [0.034, 0.028, 0.022], sides=5, mat=MI['Oro'])
    M = Matrix.Translation(p2) @ Matrix.Rotation(math.atan2(-d.x, d.y), 4, 'Z') @ Matrix.Rotation(math.radians(10), 4, 'X')
    add_claw(bg, M @ Matrix.Diagonal((0.075, 0.075, 0.075, 1)), rng, length=1.0, curve=0.55, base=0.28,
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

# =========================================================================== COLA (abanico + dos estelas)
cola = new_object('Cola', bpy.data.meshes.new('ColaZ'), COLS['Cola'], MATS)
bm = bm_from(cola)
TP = Vector((0, 0.38, 0.80))
for i in range(4):
    x = 0.035 * i
    d = Vector((x * 2.6, 1.0, -0.62)).normalized()
    add_feather(bm, feather_frame(TP + Vector((x, 0.02, -0.006 * i)), d, (0, 0.5, 1)), length=0.56 - 0.03 * i,
                width=0.15, thick=0.011, curve=-0.05, mat=MI['AveAla'], tip_mat=MI['AveCuerpo'], tip_frac=0.12,
                band=(0.66, 0.80), band_mat=MI['AvePecho'])
for x in (0.03,):  # estelas de viento (las dos plumas largas del centro, con punta blanca)
    d = Vector((0.06, 1.0, -0.5)).normalized()
    add_feather(bm, feather_frame(TP + Vector((x, 0.0, 0.025)), d, (0, 0.45, 1)), length=1.05, width=0.085, thick=0.011,
                curve=0.06, twist=0.4, mat=MI['AveCuerpo'], tip_mat=MI['AvePecho'], tip_frac=0.22)
for i in range(3):  # coberteras superiores
    x = 0.05 * i
    d = Vector((x * 2.0, 1.0, -0.45)).normalized()
    add_feather(bm, feather_frame(TP + Vector((x, -0.04, 0.05)), d, (0, 0.4, 1)), length=0.26, width=0.12, thick=0.01,
                mat=MI['AveAlaClara'], tip_mat=MI['AveAla'])
bm_commit(cola, bm)
symmetrize(cola)
flat(cola)
set_pivot(cola, TP)

# =========================================================================== raiz, rig y datos para exportar
root = bpy.data.objects.new(NAME, None)
ROOT_COL.objects.link(root)
root.empty_display_type = 'ARROWS'
for o in ROOT_COL.all_objects:
    if o is not root and o.parent is None:
        o.parent = root


def v3(v):
    return [round(c, 4) for c in v]


BONES = [('Raiz', (0, 0.1, 0), (0, -0.3, 0), None),
         ('Torso', TORSO_PIVOT, (0, -0.2, 1.5), 'Raiz'),
         ('Cuello', NECK_PIVOT, HP, 'Torso'),
         ('Cabeza', HP, (0, -0.62, 1.92), 'Cuello'),
         ('Mandibula', JAW_PIVOT, (0, -0.72, 1.855), 'Cabeza'),
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
sc['walk_frames'] = 30
sc['walk_stride'] = 0.40
sc['walk_lift'] = 0.14
sc['walk_crouch'] = 0.05
sc['cam_scale'] = 0.62      # encuadre: el ave es alta y estrecha (alas plegadas en las animaciones)
sc['cam_center'] = (0.0, 0.15, 0.0)
sc['cam_zoff'] = 0.62
sc['walk_lane'] = (0.10, 0.34)  # carriles de las patas (sin piedras en el GIF de caminata)

total = 0
for o in sorted(ROOT_COL.all_objects, key=lambda o: o.name):
    if o.type == 'MESH':
        t = tri_count(o)
        total += t
        print('%-28s %6d tris' % (o.name, t))
print('TOTAL', total)
bpy.ops.wm.save_as_mainfile(filepath=OUT)
print('guardado', OUT)
