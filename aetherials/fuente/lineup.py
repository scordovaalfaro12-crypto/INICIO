"""Comparacion de tamano: Basaltor y Obsidrax lado a lado en el mismo estudio.
Uso: python3 lineup.py -- salida.png muestras"""
import bpy, sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mathutils import Vector, Matrix
from studio import *

args = sys.argv[sys.argv.index('--') + 1:]
OUT, SAMPLES = args[0], int(args[1])
bpy.ops.wm.open_mainfile(filepath='out_obs/Obsidrax_Rig.blend')
# traer el rig de Basaltor (armature + mallas con materiales originales)
with bpy.data.libraries.load('out/Basaltor_Rig.blend', link=False) as (src, dst):
    dst.objects = [n for n in src.objects if n == 'Basaltor_Rig' or n.endswith('_glb')]
for o in dst.objects:
    bpy.context.scene.collection.objects.link(o)
cam = build_studio()
setup_glare(0.35)
setup_render((1800, 900) if SAMPLES > 40 else (900, 450), SAMPLES)
sc = bpy.context.scene
for o in bpy.data.objects:
    if o.type == 'MESH' and o.parent and o.parent.type == 'ARMATURE':
        o.hide_render = '_glb' not in o.name  # las de Basaltor llegan como 'Cabeza_glb.001'
obs = bpy.data.objects['Obsidrax_Rig']
bas = bpy.data.objects['Basaltor_Rig']
for a in (obs, bas):
    if a.animation_data:
        a.animation_data.action = None
    for pb in a.pose.bones:
        pb.rotation_mode = 'QUATERNION'
        pb.rotation_quaternion = (1, 0, 0, 0)
        pb.location = (0, 0, 0)
# misma pose de reposo, mirando hacia la camara y un poco a la izquierda
bas.matrix_world = Matrix.Translation((-3.4, 0.6, 0)) @ Matrix.Rotation(math.radians(-32), 4, 'Z') @ \
    Matrix.Translation((0, -0.45, 0))
obs.matrix_world = Matrix.Translation((2.4, 0.9, 0)) @ Matrix.Rotation(math.radians(-32), 4, 'Z') @ \
    Matrix.Translation((0, -0.82, 0))
aim_camera(cam, (-0.8, -15.5, 5.2), (-0.5, 0.5, 1.4), 45)
sc.render.filepath = OUT
bpy.ops.render.render(write_still=True)
print('LINEUP listo')
