"""Basaltor.blend de entrega: piezas + estudio + camara en la vista principal (F12 para renderizar)."""
import bpy, sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mathutils import Vector, Matrix
from studio import *
args = sys.argv[sys.argv.index('--') + 1:]
bpy.ops.wm.open_mainfile(filepath='basaltor.blend')
setup_render((1200, 1000), 128)
cam = build_studio()
setup_glare(0.4)
root = bpy.data.objects['Basaltor']
root.matrix_world = Matrix.Rotation(math.radians(-35), 4, 'Z') @ Matrix.Translation((0, -0.45, 0))
aim_camera(cam, (-1.2, -7.6, 3.4), (-0.1, 0, 0.95), 50)
bpy.ops.wm.save_as_mainfile(filepath=args[0])
