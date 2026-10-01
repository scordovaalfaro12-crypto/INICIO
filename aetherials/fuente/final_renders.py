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
LIFT = cam_lift()
AVE = bpy.context.scene.get('tipo') == 'ave'
if not AVE:  # las aves de tipo Neutro no llevan brasas
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
if AVE:
    # nombre: (res, giro, cam, objetivo, lente, accion, frame, altura de vuelo)
    SHOTS = {
        '01_tres_cuartos': ((1200, 1000), -35, (-1.2, -7.6, 3.0), (-0.1, 0, 0.95), 62, 'Reposo', 0, 0),
        '02_cabeza':       ((1200, 1000), -38, (-0.5, -4.3, 2.2), (-0.15, -0.3, 1.75), 55, 'Reposo', 0, 0),
        '03_vuelo':        ((1500, 1000), -28, (-1.4, -10.5, 2.4), (0, 0, 2.55), 50, 'Aletear', 4, 1.15),
        '04_atras':        ((1200, 1000), 150, (-1.2, -7.6, 3.2), (0, 0, 0.95), 60, 'Reposo', 0, 0),
        '05_lado':         ((1200, 1000), -90, (0, -8.6, 1.6), (0, 0, 1.0), 58, 'Reposo', 0, 0),
        '06_frente':       ((1200, 1000), 0, (0, -8.2, 1.8), (0, 0, 1.0), 58, 'Reposo', 0, 0),
        '07_grito':        ((1500, 1000), -12, (-1.0, -11.0, 2.5), (0, 0, 1.55), 45, 'Grito', 38, 0),
    }
else:
    SHOTS = {k: v + (0,) for k, v in SHOTS.items()}
for name, (res, ang, cpos, tgt, lens, act, frame, fly) in SHOTS.items():
    if ONLY and name not in ONLY:
        continue
    setup_render(res if SAMPLES > 40 else (res[0] // 3, res[1] // 3), SAMPLES)
    arm.animation_data.action = bpy.data.actions[act]
    bpy.context.scene.frame_set(frame)
    arm.matrix_world = Matrix.Translation((0, 0, fly)) @ Matrix.Rotation(math.radians(ang), 4, 'Z') @ \
        Matrix.Translation(-CENTER)
    aim_camera(cam, Vector(cpos) * SC + LIFT, Vector(tgt) * SC + LIFT, lens)
    bpy.context.scene.render.filepath = os.path.join(OUTDIR, '%s_%s.png' % (NAME, name))
    t = time.time()
    bpy.ops.render.render(write_still=True)
    print('RENDER', name, '%.0fs' % (time.time() - t), flush=True)
