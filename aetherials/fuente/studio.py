"""Estudio de render: ciclorama, luces de area, camara y ajustes de Cycles."""
import bpy, bmesh, math
from mathutils import Vector
from lib import srgb, collection


def setup_render(res=(1500, 1000), samples=96, preview=False):
    s = bpy.context.scene
    s.render.engine = 'CYCLES'
    s.cycles.device = 'CPU'
    s.cycles.samples = samples
    s.cycles.use_adaptive_sampling = True
    s.cycles.adaptive_threshold = 0.02 if not preview else 0.08
    s.cycles.use_denoising = True
    s.cycles.denoiser = 'OPENIMAGEDENOISE'
    s.cycles.max_bounces = 6
    s.cycles.diffuse_bounces = 3
    s.cycles.glossy_bounces = 2
    s.cycles.transparent_max_bounces = 2
    s.cycles.caustics_reflective = False
    s.cycles.caustics_refractive = False
    s.render.resolution_x, s.render.resolution_y = res
    s.render.resolution_percentage = 100
    s.render.film_transparent = False
    s.render.image_settings.file_format = 'PNG'
    s.view_settings.view_transform = 'AgX'
    try:
        s.view_settings.look = 'AgX - Medium High Contrast'
    except Exception:
        pass
    s.render.threads_mode = 'AUTO'


def build_studio():
    col = collection('Estudio')
    # ciclorama: piso + curva + pared al fondo (+Y), extruido en X
    bm = bmesh.new()
    prof = []
    for i in range(0, 13):
        prof.append((-14 + i * 1.5, 0.0))
    R = 5.0
    for i in range(1, 17):
        a = (math.pi / 2) * i / 16
        prof.append((4 + R * math.sin(a), R - R * math.cos(a)))
    prof.append((9.0, 14.0))
    rows = []
    for x in (-22, 22):
        rows.append([bm.verts.new((x, y, z)) for (y, z) in prof])
    for k in range(len(prof) - 1):
        bm.faces.new((rows[0][k], rows[0][k + 1], rows[1][k + 1], rows[1][k]))
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
    for f in bm.faces:
        f.smooth = True
        if f.normal.z < 0 and f.normal.y > -0.5:
            f.normal_flip()
    me = bpy.data.meshes.new('Ciclorama')
    bm.to_mesh(me)
    bm.free()
    cyc = bpy.data.objects.new('Ciclorama', me)
    col.objects.link(cyc)
    m = bpy.data.materials.new('M_Fondo')
    try:
        m.use_nodes = True
    except Exception:
        pass
    b = next(n for n in m.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
    b.inputs['Base Color'].default_value = srgb('#8a9db4') + (1,)
    b.inputs['Roughness'].default_value = 0.92
    b.inputs['Specular IOR Level'].default_value = 0.1
    me.materials.append(m)
    cyc.visible_shadow = True

    # mundo: azul oscuro tenue
    w = bpy.data.worlds.new('Mundo')
    bpy.context.scene.world = w
    try:
        w.use_nodes = True
    except Exception:
        pass
    bg = next(n for n in w.node_tree.nodes if n.type == 'BACKGROUND')
    bg.inputs['Color'].default_value = srgb('#2c3d52') + (1,)
    bg.inputs['Strength'].default_value = 0.4

    def area(name, loc, target, size, power, color='#ffffff', shape='DISK', size_y=None):
        ld = bpy.data.lights.new(name, 'AREA')
        ld.shape = shape
        ld.size = size
        if size_y:
            ld.size_y = size_y
        ld.energy = power
        ld.color = srgb(color)
        lo = bpy.data.objects.new(name, ld)
        col.objects.link(lo)
        lo.location = loc
        d = Vector(target) - Vector(loc)
        lo.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()
        return lo

    area('Luz_Clave', (-5.5, -6.5, 7.5), (0, 0.3, 0.6), 5.0, 1100, '#fff4e8')
    area('Luz_Relleno', (7.5, -4.0, 3.0), (0, 0.3, 0.8), 7.0, 320, '#cfe0ff')
    area('Luz_Contra', (2.5, 7.0, 6.0), (0, 0.5, 1.0), 4.0, 900, '#d8e6ff')
    area('Luz_Cenital', (0, 0.5, 9.0), (0, 0.5, 0), 6.0, 300, '#ffffff')

    cam_d = bpy.data.cameras.new('Camara')
    cam_d.lens = 55
    cam_d.sensor_width = 36
    cam = bpy.data.objects.new('Camara', cam_d)
    col.objects.link(cam)
    bpy.context.scene.camera = cam
    return cam


def aim_camera(cam, loc, target, lens=55):
    cam.location = loc
    d = Vector(target) - Vector(loc)
    cam.rotation_euler = d.to_track_quat('-Z', 'Y').to_euler()
    cam.data.lens = lens


def setup_glare(strength=0.35):
    """Brillo suave (bloom) para el magma en el compositor de Blender 5."""
    s = bpy.context.scene
    try:
        tree = bpy.data.node_groups.new('Comp', 'CompositorNodeTree')
        s.compositing_node_group = tree
        rl = tree.nodes.new('CompositorNodeRLayers')
        gl = tree.nodes.new('CompositorNodeGlare')
        out = tree.nodes.new('NodeGroupOutput')
        tree.interface.new_socket('Image', in_out='OUTPUT', socket_type='NodeSocketColor')
        for k, v in (('Type', 'Bloom'), ('Quality', 'High')):
            if k in gl.inputs:
                try:
                    gl.inputs[k].default_value = v
                except Exception:
                    pass
        for k, v in (('Threshold', 1.2), ('Strength', strength), ('Size', 0.6)):
            if k in gl.inputs:
                try:
                    gl.inputs[k].default_value = v
                except Exception:
                    pass
        tree.links.new(rl.outputs['Image'], gl.inputs['Image'])
        tree.links.new(gl.outputs['Image'], out.inputs[0])
        return True
    except Exception as e:
        print('glare no disponible:', e)
        return False


# ---------------------------------------------------------------- criatura actual
import os
EMBER_REGION = {
    'Basaltor': ((-0.25, 0.25), (-1.0, 1.2), (1.35, 2.4)),
    'Obsidrax': ((-0.3, 0.3), (-0.9, 1.5), (2.2, 3.5)),
}


def creature():
    """Nombre de la criatura (variable de entorno CRIATURA) y ruta del .blend con su rig."""
    name = os.environ.get('CRIATURA', 'Basaltor')
    rigdir = os.environ.get('RIGDIR', 'out')
    return name, os.path.join(rigdir, name + '_Rig.blend')


def cam_lift():
    """Altura extra de camara y objetivo (criaturas altas como las aves); propiedad 'cam_zoff' de la escena."""
    from mathutils import Vector
    return Vector((0, 0, float(bpy.context.scene.get('cam_zoff', 0.0))))


def creature_frame(root):
    """Escala de camara (Basaltor = 1) y centro de giro, medidos con root en identidad.
    Si la escena define 'cam_scale' y 'cam_center' (aves), se usan esos valores."""
    from mathutils import Vector, Matrix
    sc = bpy.context.scene
    if 'cam_scale' in sc:
        return float(sc['cam_scale']), Vector(tuple(sc['cam_center']))
    saved = root.matrix_world.copy()
    root.matrix_world = Matrix.Identity(4)
    bpy.context.view_layer.update()
    pts = []
    for o in bpy.data.objects:
        if o.type == 'MESH' and o.parent == root and not o.hide_render:
            pts += [o.matrix_world @ v.co for v in list(o.data.vertices)[::9]]
    root.matrix_world = saved
    ys = [p.y for p in pts]
    s = (max(ys) - min(ys)) / 5.34
    if s < 1.05:
        return 1.0, Vector((0, 0.45, 0))
    return s, Vector((0, (max(ys) + min(ys)) / 2, 0))
