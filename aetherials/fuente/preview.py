"""Render de vistas: python3 preview.py modelo.blend salida_prefijo ancho alto muestras vista1,vista2..."""
import bpy, sys, os, math, time
WORK = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, WORK)
from mathutils import Vector, Matrix
from studio import setup_render, build_studio, aim_camera, setup_glare

args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]
blend, prefix, W, H, S, views = args[0], args[1], int(args[2]), int(args[3]), int(args[4]), args[5].split(',')
glare = len(args) > 6 and args[6] == 'glare'

bpy.ops.wm.open_mainfile(filepath=blend)
setup_render((W, H), S, preview=S < 40)
cam = build_studio()
if glare:
    setup_glare()
root = next(o for o in bpy.data.objects if o.type == 'EMPTY' and o.parent is None)
# escala de camara segun el largo de la criatura (Basaltor mide 5,34 m)
pts = [o.matrix_world @ v.co for o in bpy.data.objects if o.type == 'MESH' and o.parent == root
       for v in list(o.data.vertices)[::7]]
ys = [p.y for p in pts]
SCALE = (max(ys) - min(ys)) / 5.34
CENTER = Vector((0, (max(ys) + min(ys)) / 2, 0)) if SCALE > 1.05 else Vector((0, 0.45, 0))
print('escala', round(SCALE, 2), 'centro', CENTER)

# vista: (angulo de giro de la criatura, pos camara, objetivo, lente)
VIEWS = {
    # como las imagenes de referencia
    'frente34': (-35, (-1.2, -7.6, 3.4), (-0.1, 0, 0.9), 50),
    'lado': (-90, (0, -8.4, 1.6), (0, 0, 0.95), 50),
    'atras34': (145, (-1.2, -7.6, 3.6), (0, 0, 0.9), 50),
    'cabeza': (-40, (-0.6, -5.4, 1.9), (-1.0, -1.0, 1.2), 55),
    'frente': (0, (0, -8.0, 1.8), (0, 0, 0.95), 50),
    'arriba': (-30, (-0.8, -6.0, 6.5), (0, 0, 0.7), 45),
    'lado_der': (90, (0, -8.4, 1.6), (0, 0, 0.95), 50),
    'cabeza_frente': (-12, (-0.4, -4.6, 1.75), (-0.2, -1.6, 1.2), 55),
    'cabeza_lado': (-80, (-0.3, -4.2, 1.5), (-1.3, -0.3, 1.2), 50),
    'pecho': (-8, (-0.3, -5.2, 0.7), (-0.15, -0.8, 0.95), 50),
}

for v in views:
    ang, cpos, tgt, lens = VIEWS[v]
    root.matrix_world = Matrix.Rotation(math.radians(ang), 4, 'Z') @ Matrix.Translation(-CENTER)
    aim_camera(cam, Vector(cpos) * SCALE, Vector(tgt) * SCALE, lens)
    bpy.context.scene.render.filepath = '%s_%s.png' % (prefix, v)
    t = time.time()
    bpy.ops.render.render(write_still=True)
    print('render', v, '%.1fs' % (time.time() - t))
