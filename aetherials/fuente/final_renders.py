"""Renders finales desde el blend del rig (materiales originales, permite poses)."""
import bpy, sys, os, math, time
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mathutils import Vector, Matrix
from studio import *
from fx import add_embers

args = sys.argv[sys.argv.index('--') + 1:]
OUTDIR, SAMPLES, ONLY = args[0], int(args[1]), (args[2].split(',') if len(args) > 2 else None)
os.makedirs(OUTDIR, exist_ok=True)
NAME, RIGBLEND = creature()
bpy.ops.wm.open_mainfile(filepath=RIGBLEND)
cam = build_studio()
setup_glare(0.4)
arm = bpy.data.objects[NAME + '_Rig']
for o in bpy.data.objects:
    if o.type == 'MESH' and o.parent == arm:
        o.hide_render = not o.name.endswith('_glb')
SC, CENTER = creature_frame(arm)
add_embers(arm, region=EMBER_REGION.get(NAME, EMBER_REGION['Basaltor']))
print('criatura', NAME, 'escala', round(SC, 2))

# nombre: (res, giro, cam, objetivo, lente, accion, frame)
SHOTS = {
    '01_tres_cuartos': ((1200, 1000), -35, (-1.2, -7.6, 3.4), (-0.1, 0, 0.95), 50, 'Reposo', 0),
    '02_cabeza':       ((1200, 1000), -40, (-0.6, -5.4, 1.9), (-1.0, -1.0, 1.25), 55, 'Reposo', 0),
    '03_arriba':       ((1200, 1000), -30, (-0.8, -6.4, 6.6), (0, 0, 0.7), 45, 'Reposo', 0),
    '04_atras':        ((1300, 1100), 145, (-1.2, -7.6, 3.6), (0, 0, 0.95), 50, 'Reposo', 0),
    '05_lado':         ((1500, 1000), -90, (0, -8.6, 1.7), (0, 0, 1.0), 50, 'Reposo', 0),
    '06_frente':       ((1200, 1000), 0, (0, -8.2, 1.9), (0, 0, 1.0), 50, 'Reposo', 0),
    '07_rugido':       ((1500, 1000), -55, (-2.0, -6.8, 2.3), (-0.65, 0, 1.2), 42, 'Rugido', 38),
}
for name, (res, ang, cpos, tgt, lens, act, frame) in SHOTS.items():
    if ONLY and name not in ONLY:
        continue
    setup_render(res if SAMPLES > 40 else (res[0] // 3, res[1] // 3), SAMPLES)
    arm.animation_data.action = bpy.data.actions[act]
    bpy.context.scene.frame_set(frame)
    arm.matrix_world = Matrix.Rotation(math.radians(ang), 4, 'Z') @ Matrix.Translation(-CENTER)
    aim_camera(cam, Vector(cpos) * SC, Vector(tgt) * SC, lens)
    bpy.context.scene.render.filepath = os.path.join(OUTDIR, '%s_%s.png' % (NAME, name))
    t = time.time()
    bpy.ops.render.render(write_still=True)
    print('RENDER', name, '%.0fs' % (time.time() - t), flush=True)
