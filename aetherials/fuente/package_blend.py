"""<Nombre>.blend de entrega: piezas + estudio + camara en la vista principal (F12 para renderizar).
Uso: CRIATURA=Obsidrax python3 package_blend.py -- salida.blend"""
import bpy, sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mathutils import Vector, Matrix
from studio import *
args = sys.argv[sys.argv.index('--') + 1:]
NAME = os.environ.get('CRIATURA', 'Basaltor')
bpy.ops.wm.open_mainfile(filepath=os.environ.get('MODELO', NAME.lower() + '.blend'))
setup_render((1200, 1000), 128)
cam = build_studio()
setup_glare(0.4)
root = bpy.data.objects[NAME]
SC, CENTER = creature_frame(root)
root.matrix_world = Matrix.Rotation(math.radians(-35), 4, 'Z') @ Matrix.Translation(-CENTER)
aim_camera(cam, Vector((-1.2, -7.6, 3.4)) * SC, Vector((-0.1, 0, 0.95)) * SC, 50)
bpy.ops.wm.save_as_mainfile(filepath=args[0])
