"""Video giratorio con la animacion de reposo (frames en PNG; luego ffmpeg)."""
import bpy, sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mathutils import Vector, Matrix
from studio import *
from fx import add_embers

args = sys.argv[sys.argv.index('--') + 1:]
OUTDIR, SAMPLES, FRAMES = args[0], int(args[1]), int(args[2])
os.makedirs(OUTDIR, exist_ok=True)
bpy.ops.wm.open_mainfile(filepath='out/Basaltor_Rig.blend')
cam = build_studio()
setup_glare(0.35)
setup_render((960, 760), SAMPLES)
sc = bpy.context.scene
sc.render.fps = 24
arm = bpy.data.objects['Basaltor_Rig']
for o in bpy.data.objects:
    if o.type == 'MESH' and o.parent == arm:
        o.hide_render = not o.name.endswith('_glb')
add_embers(arm, frames=FRAMES)
act = bpy.data.actions['Reposo']
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


for fc in action_fcurves(act):  # repetir la animacion de reposo (60 frames) en bucle
    if not any(m.type == 'CYCLES' for m in fc.modifiers):
        fc.modifiers.new('CYCLES')
CENTER = Vector((0, 0.45, 0))
aim_camera(cam, (0, -9.4, 3.4), (0, 0, 0.95), 50)
start = int(args[3]) if len(args) > 3 else 0
for f in range(start, FRAMES):
    out = os.path.join(OUTDIR, 'f%04d.png' % f)
    if os.path.exists(out):
        continue
    sc.frame_set(f)
    ang = -360.0 * f / FRAMES - 30
    arm.matrix_world = Matrix.Rotation(math.radians(ang), 4, 'Z') @ Matrix.Translation(-CENTER)
    sc.render.filepath = out
    bpy.ops.render.render(write_still=True)
    print('FRAME', f, flush=True)
