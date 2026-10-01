"""Brasas flotando sobre la grieta del lomo (solo para renders)."""
import bpy, bmesh, math, random
from mathutils import Vector, Matrix
from lib import srgb, collection


def ember_material():
    m = bpy.data.materials.get('M_Brasa') or bpy.data.materials.new('M_Brasa')
    try:
        m.use_nodes = True
    except Exception:
        pass
    b = next(n for n in m.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
    c = srgb('#ff8a33') + (1,)
    b.inputs['Base Color'].default_value = c
    b.inputs['Emission Color'].default_value = c
    b.inputs['Emission Strength'].default_value = 12.0
    return m


def add_embers(parent, n=46, seed=3, frames=None, region=((-0.25, 0.25), (-1.0, 1.2), (1.35, 2.4))):
    """Crea brasas (octaedros) parentadas a 'parent'. Si frames, anima subida en bucle."""
    rng = random.Random(seed)
    col = collection('FX_Brasas')
    mat = ember_material()
    me = bpy.data.meshes.new('Brasa')
    bm = bmesh.new()
    bmesh.ops.create_icosphere(bm, subdivisions=1, radius=1.0)
    bm.to_mesh(me)
    bm.free()
    me.materials.append(mat)
    objs = []
    for i in range(n):
        o = bpy.data.objects.new('Brasa_%02d' % i, me)
        col.objects.link(o)
        o.parent = parent
        x = rng.uniform(*region[0]) * rng.uniform(0.3, 1)
        y = rng.uniform(*region[1])
        z0 = rng.uniform(*region[2])
        s = rng.uniform(0.008, 0.02)
        o.location = (x, y, z0)
        o.scale = (s, s, s)
        o.visible_shadow = False
        if frames:
            ph = rng.random()
            rise = rng.uniform(0.8, 1.4)
            sw = rng.uniform(0.02, 0.07)
            for f in range(0, frames + 1, 3):
                t = (f / frames * rng.choice((1, 2)) + ph) % 1.0
                o.location = (x + sw * math.sin(t * 9 + i), y + sw * math.cos(t * 7 + i), region[2][0] + t * rise)
                k = s * (1 - t) ** 0.7
                o.scale = (k, k, k)
                o.keyframe_insert('location', frame=f)
                o.keyframe_insert('scale', frame=f)
        objs.append(o)
    return objs
