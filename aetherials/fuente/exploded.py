"""Vista explotada: las piezas separadas de su articulacion, con etiquetas (2D)."""
import bpy, sys, os, math, json
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mathutils import Vector, Matrix
from bpy_extras.object_utils import world_to_camera_view
from studio import *

args = sys.argv[sys.argv.index('--') + 1:]
OUT, SAMPLES = args[0], int(args[1])
ROT = float(args[2]) if len(args) > 2 else -55.0
RES = (1800, 1200) if SAMPLES > 40 else (600, 400)
NAME = os.environ.get('CRIATURA', 'Basaltor')
bpy.ops.wm.open_mainfile(filepath=os.environ.get('MODELO', NAME.lower() + '.blend'))
setup_render(RES, SAMPLES)
cam = build_studio()
setup_glare(0.3)
root = bpy.data.objects[NAME]
P = {o.name: o for o in bpy.data.collections[NAME].all_objects if o.type == 'MESH'}
SC, CENTER = creature_frame(root)

OFF = {
    'Craneo': (0, -0.95, 0.45), 'Ojos': (0, -1.40, 0.62), 'Cejas': (0, -1.15, 1.05),
    'Cuernos': (0, -0.75, 1.25), 'Dientes_Superiores': (0, -1.25, 0.10), 'Mandibula': (0, -1.30, -0.40),
    'Cuello': (0, -0.45, 0.15), 'Lomo': (0, 0.0, 1.05), 'Torso': (0, 0, 0),
    'Torso__Melena': (0, -0.20, 0.80), 'Torso__Nucleo': (0, -1.05, -0.30),
}
for side, sx in (('Izq', 1), ('Der', -1)):
    for pre, names in (('PataDel', ('Brazo', 'Antebrazo', 'Mano', 'Garras')),
                       ('PataTra', ('Muslo', 'Pierna', 'Pie', 'Garras'))):
        for k, nm in enumerate(names):
            dy = -0.16 if pre == 'PataDel' else 0.16
            OFF['%s_%s_%s' % (pre, side, nm)] = (sx * (0.6 + 0.28 * k), dy * k + (-0.30 if nm == 'Garras' else 0),
                                                 -0.10 * k + (0.06 if nm == 'Garras' else 0))
for i, n in enumerate(['Cola_1', 'Cola_2', 'Cola_3', 'Cola_4', 'Cola_Mazo']):
    OFF[n] = (0, 0.36 * (i + 1), 0.12 * (i + 1))
LIFT = Vector((0, 0, 0.75))
if bpy.context.scene.get('tipo') == 'ave':
    OFF = {'Craneo': (0, -0.55, 0.55), 'Ojos': (0, -1.05, 0.75), 'Cresta': (0, -0.15, 0.92), 'Mandibula': (0, -1.0, 0.2),
           'Cuello': (0, -0.3, 0.25), 'Cuello__Collar': (0, -0.75, 0.1), 'Torso': (0, 0, 0), 'Torso__Pechera': (0, -1.0, -0.15),
           'Cola': (0.55, 0.35, -0.55), 'Torso__Capa': (0.0, 0.85, 0.75)}
    for side, sx in (('Izq', 1), ('Der', -1)):
        for k, nm in enumerate(('Brazo', 'Antebrazo', 'Mano')):
            OFF['Ala_%s_%s' % (side, nm)] = (sx * (0.25 + 0.28 * k), 0, 0.25)
        for k, nm in enumerate(('Muslo', 'Tarso', 'Pie', 'Garras')):
            OFF['Pata_%s_%s' % (side, nm)] = (sx * (0.25 + 0.06 * k), -0.12 * k, -0.08 * k + (0.0 if nm != 'Garras' else 0.0))
    LIFT = Vector((0, 0, 0.55))
for n, o in P.items():
    o.location = o.location + (Vector(OFF.get(n, (0, 0, 0))) + LIFT) * SC

root.matrix_world = Matrix.Rotation(math.radians(ROT), 4, 'Z') @ Matrix.Translation(-CENTER)
if bpy.context.scene.get('tipo') == 'ave':
    root.matrix_world = Matrix.Rotation(math.radians(-25), 4, 'Z') @ Matrix.Translation(-CENTER)
    aim_camera(cam, Vector((-1.4, -9.6, 8.0)) * SC + cam_lift(), Vector((0, 0.1, 1.2)) * SC + cam_lift(), 40)
else:
    aim_camera(cam, Vector((-1.8, -12.6, 6.4)) * SC, Vector((0.0, 0.3, 1.45)) * SC, 47)
bpy.context.view_layer.update()

LABELS = {
    'Craneo': 'Cráneo', 'Mandibula': 'Mandíbula', 'Cuernos': 'Cuernos estriados', 'Ojos': 'Ojos',
    'Cejas': 'Cejas de roca', 'Dientes_Superiores': 'Dientes', 'Cuello': 'Cuello', 'Lomo': 'Lomo de basalto',
    'Torso': 'Torso', 'PataDel_Izq_Brazo': 'Brazo', 'PataDel_Izq_Antebrazo': 'Antebrazo', 'PataDel_Izq_Mano': 'Mano',
    'PataDel_Izq_Garras': 'Garras', 'PataTra_Izq_Muslo': 'Muslo', 'PataTra_Izq_Pierna': 'Pierna',
    'PataTra_Izq_Pie': 'Pie', 'PataTra_Izq_Garras': 'Garras', 'Cola_1': 'Cola 1', 'Cola_2': 'Cola 2',
    'Cola_3': 'Cola 3', 'Cola_4': 'Cola 4', 'Cola_Mazo': 'Mazo de basalto',
}
if NAME in ('Zephalcon', 'Zephyrex'):
    LABELS = {'Craneo': 'Cabeza y pico', 'Ojos': 'Ojos y máscara dorada', 'Cresta': 'Cresta', 'Mandibula': 'Pico inferior',
              'Cuello': 'Cuello', 'Cuello__Collar': 'Collar de plumas', 'Torso': 'Cuerpo', 'Torso__Pechera': 'Pechera de chevrones',
              'Cola': 'Cola con estelas', 'Ala_Izq_Brazo': 'Ala: brazo', 'Ala_Izq_Antebrazo': 'Ala: antebrazo',
              'Ala_Izq_Mano': 'Ala: mano (primarias)', 'Pata_Izq_Muslo': 'Muslo', 'Pata_Izq_Tarso': 'Tarso dorado',
              'Pata_Izq_Pie': 'Pie', 'Pata_Izq_Garras': 'Garras'}
if NAME == 'Zephyrex':
    LABELS.update({'Cresta': 'Corona de plumas', 'Ojos': 'Ojos y visor dorado', 'Torso__Pechera': 'Pechera con emblema',
                   'Torso__Capa': 'Capa de plumas', 'Cola': 'Cola con 4 estelas', 'Pata_Izq_Tarso': 'Tarso con anillo de oro'})
if NAME == 'Obsidrax':
    LABELS.update({'Lomo': 'Lomo de obsidiana', 'Cuernos': 'Corona de cuernos', 'Cejas': 'Cejas de obsidiana',
                   'Ojos': 'Ojos de magma', 'Cola_Mazo': 'Orbe de magma', 'Torso__Melena': 'Melena de obsidiana',
                   'Torso__Nucleo': 'Corazón de magma'})
sc = bpy.context.scene
pts = {}
for n, lab in LABELS.items():
    o = P[n]
    vs = [o.matrix_world @ v.co for v in o.data.vertices]
    c = sum(vs, Vector()) / len(vs)
    p = world_to_camera_view(sc, cam, c)
    pts[n] = (lab, p.x * RES[0], (1 - p.y) * RES[1])
json.dump(pts, open(OUT + '.json', 'w'), ensure_ascii=False)
sc.render.filepath = OUT + '_raw.png'
bpy.ops.render.render(write_still=True)
