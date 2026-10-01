"""Render de la caminata: la criatura camina en el lugar y el suelo (piedras) se desplaza
a la misma velocidad que los pies apoyados, asi se ve avanzar. Bucle perfecto.
Uso: python3 walk_render.py -- carpeta muestras ciclos ancho alto [frames_a_renderizar]
Con MODO=vuelo (aves) usa la accion Aletear: el ave vuela en el lugar y el suelo pasa rapido debajo."""
import bpy, bmesh, sys, os, math, random
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mathutils import Vector, Matrix
from studio import *
from fx import add_embers
from lib import srgb

args = sys.argv[sys.argv.index('--') + 1:]
OUTDIR, SAMPLES, CYCLES, W, H = args[0], int(args[1]), int(args[2]), int(args[3]), int(args[4])
ONLY = [int(x) for x in args[5].split(',')] if len(args) > 5 else None
os.makedirs(OUTDIR, exist_ok=True)

NAME, RIGBLEND = creature()
bpy.ops.wm.open_mainfile(filepath=RIGBLEND)
_sc = bpy.context.scene
VUELO = os.environ.get('MODO') == 'vuelo'
WALK_FRAMES, BETA, STRIDE = int(_sc.get('walk_frames', 36)), float(_sc.get('walk_beta', 0.70)), \
    float(_sc.get('walk_stride', 0.50))
STEP = STRIDE / (BETA * WALK_FRAMES)       # m por frame que avanza el suelo
if VUELO:
    WALK_FRAMES = 24                        # un aleteo
    STEP = 0.11                             # ~3,3 m/s
FLY = 1.25 if VUELO else 0.0
TOTAL = WALK_FRAMES * CYCLES
TILE = STEP * TOTAL                         # periodo del patron de piedras (bucle perfecto)
cam = build_studio()
setup_glare(0.35)
setup_render((W, H), SAMPLES, preview=SAMPLES < 40)
sc = bpy.context.scene
sc.render.fps = 30
arm = bpy.data.objects[NAME + '_Rig']
for o in bpy.data.objects:
    if o.type == 'MESH' and o.parent == arm:
        o.hide_render = not o.name.endswith('_glb')
SC, CENTER = creature_frame(arm)
LIFT = cam_lift()
LANE = tuple(_sc['walk_lane']) if 'walk_lane' in _sc else (0.38 * SC, 1.05 * SC)
act = bpy.data.actions['Aletear' if VUELO else 'Caminar']
arm.animation_data.action = act


def action_fcurves(a):
    try:
        return list(a.fcurves)
    except Exception:
        out = []
        for layer in a.layers:
            for strip in layer.strips:
                for cb in strip.channelbags:
                    out += list(cb.fcurves)
        return out


for fc in action_fcurves(act):
    if not any(m.type == 'CYCLES' for m in fc.modifiers):
        fc.modifiers.new('CYCLES')

# ---------------------------------------------------------------- piedras del suelo
rmat = bpy.data.materials.new('M_Piedra')
try:
    rmat.use_nodes = True
except Exception:
    pass
b = next(n for n in rmat.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
b.inputs['Base Color'].default_value = srgb('#4a5a70') + (1,)
b.inputs['Roughness'].default_value = 0.8
rmat2 = bpy.data.materials.new('M_PiedraOscura')
try:
    rmat2.use_nodes = True
except Exception:
    pass
b2 = next(n for n in rmat2.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
b2.inputs['Base Color'].default_value = srgb('#2a3546') + (1,)
b2.inputs['Roughness'].default_value = 0.8

rng = random.Random(9)
tile_rocks = []
n_rocks = max(8, int(TILE * 9))
while len(tile_rocks) < n_rocks:
    x = rng.uniform(-3.2, 3.2) * SC
    if LANE[0] < abs(x) < LANE[1] and not VUELO:  # carriles de las patas: sin piedras para que no las pisen
        continue
    y = rng.uniform(0, TILE)
    s = rng.choice((rng.uniform(0.03, 0.07), rng.uniform(0.03, 0.07), rng.uniform(0.08, 0.16)))
    tile_rocks.append((x, y, s, rng.uniform(0, 6.28), rng.random() < 0.5, rng.randint(0, 9999)))

ground = bpy.data.objects.new('Suelo_Movil', None)
sc.collection.objects.link(ground)
ground.parent = arm
bm = bmesh.new()
k = 0
reps = int(math.ceil(14 * SC / TILE))
for r in range(-reps, reps + 1):
    for (x, y, s, rot, dark, seed) in tile_rocks:
        yy = y + r * TILE
        if abs(yy - CENTER.y) > 7 * SC:
            continue
        rr = random.Random(seed)
        geo = bmesh.ops.create_icosphere(bm, subdivisions=1, radius=1.0,
                                         matrix=Matrix.Translation((x, yy, 0.0)) @ Matrix.Rotation(rot, 4, 'Z')
                                         @ Matrix.Diagonal((s * rr.uniform(0.8, 1.4), s * rr.uniform(0.8, 1.2), s * rr.uniform(0.4, 0.7), 1)))
        for v in geo['verts']:
            v.co += Vector((rr.uniform(-0.2, 0.2), rr.uniform(-0.2, 0.2), rr.uniform(-0.1, 0.1))) * s
        for f in {f for v in geo['verts'] for f in v.link_faces}:
            f.material_index = 1 if dark else 0
        k += 1
me = bpy.data.meshes.new('Piedras')
bm.to_mesh(me)
bm.free()
me.materials.append(rmat)
me.materials.append(rmat2)
rocks = bpy.data.objects.new('Piedras', me)
sc.collection.objects.link(rocks)
rocks.parent = ground
print('piedras', k, 'tile', round(TILE, 3))

if _sc.get('tipo') != 'ave':
    add_embers(arm, frames=TOTAL, region=EMBER_REGION.get(NAME, EMBER_REGION['Basaltor']))

arm.matrix_world = Matrix.Translation((0, 0, FLY)) @ Matrix.Rotation(math.radians(-72 if not VUELO else -55), 4, 'Z') @ \
    Matrix.Translation(-CENTER)
ground.matrix_parent_inverse = Matrix.Translation((0, 0, -FLY))  # el suelo se queda en el piso aunque el ave vuele
AVE = _sc.get('tipo') == 'ave'
if VUELO:
    aim_camera(cam, Vector((-1.6, -9.6, 2.9)) * SC + LIFT, Vector((-0.1, 0, 2.5)) * SC + LIFT, 38)
else:
    aim_camera(cam, Vector((-1.3, -8.4, 2.3)) * SC + LIFT, Vector((-0.3 if not AVE else 0.1, 0, 0.82)) * SC + LIFT,
               43 if not AVE else 36)

frames = ONLY if ONLY is not None else range(TOTAL)
for f in frames:
    out = os.path.join(OUTDIR, 'w%04d.png' % f)
    if os.path.exists(out) and ONLY is None:
        continue
    sc.frame_set(f)
    ground.location = (0, (STEP * f) % TILE, 0)  # el suelo retrocede (+Y local) al ritmo del paso
    sc.render.filepath = out
    bpy.ops.render.render(write_still=True)
    print('FRAME', f, flush=True)
