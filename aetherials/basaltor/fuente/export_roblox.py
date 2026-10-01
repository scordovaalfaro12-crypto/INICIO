"""Exporta Basaltor para Roblox.
Salidas (en la carpeta indicada):
  Basaltor_Paleta.png              textura de paleta (todas las piezas de roca la comparten)
  Basaltor_Roblox_Rig.fbx          modelo con esqueleto (piezas rigidas por hueso) + animacion de reposo
  Basaltor_Anim_Reposo.fbx         animacion de reposo (respiracion, cola, mandibula)
  Basaltor_Anim_Rugido.fbx         animacion de rugido
  Basaltor_Anim_Caminar.fbx        ciclo de caminata (en el lugar)
  Basaltor_Roblox_Partes.fbx       todas las piezas sueltas, sin esqueleto, con su pivote en la articulacion
  Basaltor.glb                     modelo completo con materiales originales y animaciones (visor / otros motores)
  Basaltor_Rig.blend               escena de Blender con el rig y las animaciones
Uso: python3 export_roblox.py basaltor.blend carpeta_salida
"""
import bpy, bmesh, math, os, sys
WORK = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, WORK)
from mathutils import Vector, Matrix, Quaternion, Euler
from lib import PALETTE, MI, srgb
import addon_utils

args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else sys.argv[1:]
SRC, OUT = os.path.abspath(args[0]), os.path.abspath(args[1])
os.makedirs(OUT, exist_ok=True)
for m in ('io_scene_fbx', 'io_scene_gltf2'):
    addon_utils.enable(m, default_set=True)

GLOW = {MI['Magma'], MI['MagmaCaliente']}
MAGMA_NEON = '#ff7a33'

# ------------------------------------------------------------------ paleta
COLS, ROWS, CELL = 8, 4, 64


def cell_uv(i):
    c, r = i % COLS, i // COLS
    u = (c + 0.5) / COLS
    v = 1 - (r + 0.5) / ROWS
    return u, v


def make_palette_png(path):
    from PIL import Image, ImageDraw
    im = Image.new('RGB', (COLS * CELL, ROWS * CELL), (255, 0, 255))
    d = ImageDraw.Draw(im)
    for i, (name, hexc, *_r) in enumerate(PALETTE):
        c, r = i % COLS, i // COLS
        col = tuple(int(hexc[k:k + 2], 16) for k in (1, 3, 5))
        d.rectangle([c * CELL, r * CELL, (c + 1) * CELL - 1, (r + 1) * CELL - 1], fill=col)
    im.save(path)


# ------------------------------------------------------------------ utilidades
def world_bmesh(objs):
    """Une objetos en un bmesh en espacio mundo, conservando el indice de material global."""
    bm = bmesh.new()
    for o in objs:
        n0 = len(bm.verts)
        bm.from_mesh(o.data)
        bm.verts.ensure_lookup_table()
        new = bm.verts[n0:]
        bmesh.ops.transform(bm, matrix=o.matrix_world, verts=new)
    return bm


def split_glow(bm):
    """Devuelve (bm_roca, bm_magma)."""
    rock = bm.copy()
    mag = bm.copy()
    bmesh.ops.delete(rock, geom=[f for f in rock.faces if f.material_index in GLOW], context='FACES')
    bmesh.ops.delete(mag, geom=[f for f in mag.faces if f.material_index not in GLOW], context='FACES')
    for b in (rock, mag):
        bmesh.ops.delete(b, geom=[v for v in b.verts if not v.link_faces], context='VERTS')
    return rock, mag


def palette_uv(bm):
    uv = bm.loops.layers.uv.verify()
    du, dv = 0.2 / COLS, 0.2 / ROWS  # triangulo pequeno dentro de la celda (sin sangrado al hacer mipmaps)
    for f in bm.faces:
        u, v = cell_uv(f.material_index)
        n = len(f.loops)
        for k, l in enumerate(f.loops):
            a = 2 * math.pi * k / n
            l[uv].uv = (u + du * math.cos(a), v + dv * math.sin(a))
        f.material_index = 0
        f.smooth = False


def mesh_from_bm(name, bm, mats):
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    for m in mats:
        me.materials.append(m)
    return me


def palette_material(img_path):
    m = bpy.data.materials.new('Basaltor_Paleta')
    try:
        m.use_nodes = True
    except Exception:
        pass
    nt = m.node_tree
    bsdf = next(n for n in nt.nodes if n.type == 'BSDF_PRINCIPLED')
    tex = nt.nodes.new('ShaderNodeTexImage')
    tex.image = bpy.data.images.load(img_path)
    tex.interpolation = 'Closest'
    nt.links.new(tex.outputs['Color'], bsdf.inputs['Base Color'])
    bsdf.inputs['Roughness'].default_value = 0.6
    return m


def magma_material():
    m = bpy.data.materials.new('Basaltor_Magma_Neon')
    try:
        m.use_nodes = True
    except Exception:
        pass
    bsdf = next(n for n in m.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
    c = srgb(MAGMA_NEON) + (1,)
    bsdf.inputs['Base Color'].default_value = c
    bsdf.inputs['Emission Color'].default_value = c
    bsdf.inputs['Emission Strength'].default_value = 6.0
    m.diffuse_color = c
    return m


# ------------------------------------------------------------------ cargar
bpy.ops.wm.open_mainfile(filepath=SRC)
scene = bpy.context.scene
root = bpy.data.objects['Basaltor']
root.matrix_world = Matrix.Identity(4)
bpy.context.view_layer.update()
parts = {o.name: o for o in bpy.data.collections['Basaltor'].all_objects if o.type == 'MESH'}
ORIG_MATS = list(parts['Torso'].data.materials)
for n, o in parts.items():  # liberar los nombres limpios para las mallas exportadas
    o.name = 'SRC_' + n
    o.data.name = 'SRC_' + n

PNG = os.path.join(OUT, 'Basaltor_Paleta.png')
make_palette_png(PNG)
PAL = palette_material(PNG)
NEON = magma_material()


# ------------------------------------------------------------------ esqueleto
def loc(n):
    return parts[n].matrix_world.translation.copy()


BONES = []  # (nombre, cabeza, cola, padre)
tor = loc('Torso')
BONES.append(('Raiz', Vector((0, tor.y, 0)), Vector((0, tor.y - 0.4, 0)), None))
BONES.append(('Torso', tor, tor + Vector((0, -0.55, 0.0)), 'Raiz'))
neck = loc('Cuello')
head = loc('Craneo')
jaw = loc('Mandibula')
BONES.append(('Cuello', neck, head, 'Torso'))
BONES.append(('Cabeza', head, head + Vector((0, -0.55, 0.08)), 'Cuello'))
BONES.append(('Mandibula', jaw, jaw + Vector((0, -0.55, -0.08)), 'Cabeza'))
for side in ('Izq', 'Der'):
    for pre, a, b, c in (('PataDel', 'Brazo', 'Antebrazo', 'Mano'), ('PataTra', 'Muslo', 'Pierna', 'Pie')):
        n1, n2, n3 = ('%s_%s_%s' % (pre, side, x) for x in (a, b, c))
        p1, p2, p3 = loc(n1), loc(n2), loc(n3)
        BONES.append((n1, p1, p2, 'Torso'))
        BONES.append((n2, p2, p3, n1))
        BONES.append((n3, p3, p3 + Vector((0, -0.3, -0.12)), n2))
prev = 'Torso'
tail_names = ['Cola_1', 'Cola_2', 'Cola_3', 'Cola_4', 'Cola_Mazo']
for i, n in enumerate(tail_names):
    h = loc(n)
    t = loc(tail_names[i + 1]) if i + 1 < len(tail_names) else h + Vector((0, 0.5, 0))
    BONES.append((n, h, t, prev))
    prev = n

# pieza -> hueso
PART_BONE = {}
for n in parts:
    if n in ('Torso', 'Lomo'):
        PART_BONE[n] = 'Torso'
    elif n in ('Craneo', 'Ojos', 'Cejas', 'Cuernos', 'Dientes_Superiores'):
        PART_BONE[n] = 'Cabeza'
    elif n.endswith('_Garras'):
        PART_BONE[n] = n.replace('_Garras', '_Mano' if n.startswith('PataDel') else '_Pie')
    else:
        PART_BONE[n] = n
assert all(b in {x[0] for x in BONES} for b in PART_BONE.values()), PART_BONE

# ------------------------------------------------------------------ pieza sueltas (sin rig)
static_col = bpy.data.collections.new('Roblox_Partes')
scene.collection.children.link(static_col)
static_objs = []
for n, o in sorted(parts.items()):
    bm = world_bmesh([o])
    rock, mag = split_glow(bm)
    bm.free()
    piv = o.matrix_world.translation.copy()
    for suffix, b, mat in (('', rock, PAL), ('_Magma', mag, NEON)):
        if not b.faces:
            b.free()
            continue
        if mat is PAL:
            palette_uv(b)
        else:
            uv = b.loops.layers.uv.verify()
            u, v = cell_uv(MI['Magma'])
            for f in b.faces:
                f.material_index = 0
                for l in f.loops:
                    l[uv].uv = (u, v)
        bmesh.ops.translate(b, vec=-piv, verts=b.verts[:])
        bmesh.ops.triangulate(b, faces=b.faces[:])
        me = mesh_from_bm(n + suffix, b, [mat])
        b.free()
        so = bpy.data.objects.new(n + suffix, me)
        static_col.objects.link(so)
        so.location = piv
        static_objs.append(so)


def export_fbx(path, objs, armature=None, anim=False, action=None):
    bpy.ops.object.select_all(action='DESELECT')
    for o in objs:
        o.select_set(True)
    if armature:
        armature.select_set(True)
        bpy.context.view_layer.objects.active = armature
        if action is not None:
            armature.animation_data.action = action
            fr = action.frame_range
            scene.frame_start, scene.frame_end = int(fr[0]), int(fr[1])
    bpy.ops.export_scene.fbx(
        filepath=path, use_selection=True, object_types={'ARMATURE', 'MESH'},
        apply_unit_scale=True, apply_scale_options='FBX_SCALE_ALL', global_scale=1.0,
        axis_forward='Z', axis_up='Y', mesh_smooth_type='FACE', use_triangles=True,
        add_leaf_bones=False, primary_bone_axis='Y', secondary_bone_axis='X',
        use_armature_deform_only=False, bake_anim=anim, bake_anim_use_all_actions=False,
        bake_anim_use_nla_strips=False, bake_anim_force_startend_keying=True,
        bake_anim_simplify_factor=0.0, path_mode='COPY', embed_textures=True)


export_fbx(os.path.join(OUT, 'Basaltor_Roblox_Partes.fbx'), static_objs)
print('partes sueltas:', len(static_objs))
for so in static_objs:  # liberar nombres para el rig
    me = so.data
    bpy.data.objects.remove(so)
    bpy.data.meshes.remove(me)
static_objs = []

# ------------------------------------------------------------------ rig
arm_data = bpy.data.armatures.new('Basaltor_Rig')
arm = bpy.data.objects.new('Basaltor_Rig', arm_data)
rig_col = bpy.data.collections.new('Roblox_Rig')
scene.collection.children.link(rig_col)
rig_col.objects.link(arm)
bpy.context.view_layer.objects.active = arm
bpy.ops.object.mode_set(mode='EDIT')
for name, h, t, par in BONES:
    eb = arm_data.edit_bones.new(name)
    eb.head, eb.tail = h, t
    eb.roll = 0.0
    if par:
        eb.parent = arm_data.edit_bones[par]
bpy.ops.object.mode_set(mode='OBJECT')

# mallas unidas por hueso (roca con paleta + magma neon) y version con materiales originales (glb)
by_bone = {}
for n, b in PART_BONE.items():
    by_bone.setdefault(b, []).append(parts[n])

rig_meshes, glb_meshes = [], []
for bone, objs in sorted(by_bone.items()):
    bm = world_bmesh(sorted(objs, key=lambda o: o.name))
    # version glb (materiales originales)
    gb = bm.copy()
    bmesh.ops.triangulate(gb, faces=gb.faces[:])
    gme = mesh_from_bm('GLB_' + bone, gb, ORIG_MATS)
    gb.free()
    go = bpy.data.objects.new(bone + '_glb', gme)
    rig_col.objects.link(go)
    glb_meshes.append((go, bone))
    rock, mag = split_glow(bm)
    bm.free()
    for suffix, b, mat in (('', rock, PAL), ('_Magma', mag, NEON)):
        if not b.faces:
            b.free()
            continue
        if mat is PAL:
            palette_uv(b)
        else:
            uv = b.loops.layers.uv.verify()
            u, v = cell_uv(MI['Magma'])
            for f in b.faces:
                f.material_index = 0
                for l in f.loops:
                    l[uv].uv = (u, v)
        bmesh.ops.triangulate(b, faces=b.faces[:])
        me = mesh_from_bm(bone + suffix, b, [mat])
        b.free()
        mo = bpy.data.objects.new(bone + suffix, me)
        rig_col.objects.link(mo)
        rig_meshes.append((mo, bone))

for mo, bone in rig_meshes + glb_meshes:
    vg = mo.vertex_groups.new(name=bone)
    vg.add(list(range(len(mo.data.vertices))), 1.0, 'REPLACE')
    mo.parent = arm
    md = mo.modifiers.new('Armature', 'ARMATURE')
    md.object = arm

for o in list(parts.values()) + [root]:
    o.hide_render = True
    o.hide_viewport = True
for o in static_objs:
    o.hide_render = True
    o.hide_viewport = True

# ------------------------------------------------------------------ animaciones
FPS = 30
scene.render.fps = FPS


def set_rot(pb, q_world):
    """Rotacion expresada en ejes del mundo (armature space) -> espacio local del hueso."""
    R = pb.bone.matrix_local.to_3x3()
    pb.rotation_mode = 'QUATERNION'
    pb.rotation_quaternion = (R.inverted() @ q_world.to_matrix() @ R).to_quaternion()


def rx(deg):
    return Quaternion((1, 0, 0), math.radians(deg))


def rz(deg):
    return Quaternion((0, 0, 1), math.radians(deg))


def make_action(name, frames, pose_fn):
    act = bpy.data.actions.new(name)
    if arm.animation_data is None:
        arm.animation_data_create()
    arm.animation_data.action = act
    for f in range(frames + 1):
        t = f / frames
        poses = pose_fn(t)
        for pb in arm.pose.bones:
            q, dz = poses.get(pb.name, (Quaternion(), 0.0))
            set_rot(pb, q)
            pb.location = (0, 0, 0)
            if dz:
                R = pb.bone.matrix_local.to_3x3()
                pb.location = R.inverted() @ Vector((0, 0, dz))
            pb.keyframe_insert('rotation_quaternion', frame=f)
            pb.keyframe_insert('location', frame=f)
    act.use_fake_user = True
    return act


TAU = 2 * math.pi


def pose_idle(t):
    s = math.sin(TAU * t)
    c = math.cos(TAU * t)
    p = {
        'Torso': (rx(0.8 * s), 0.018 * s),
        'Cuello': (rx(-1.5 * s), 0.0),
        'Cabeza': (rx(2.0 * c) @ rz(1.2 * s), 0.0),
        'Mandibula': (rx(2.5 + 2.5 * s), 0.0),
    }
    for i, n in enumerate(tail_names):
        ph = TAU * t - 0.6 * (i + 1)
        p[n] = (rz((3 + 1.6 * i) * math.sin(ph)), 0.0)
    for side in ('Izq', 'Der'):
        p['PataDel_%s_Brazo' % side] = (rx(-0.8 * s), 0.0)
        p['PataDel_%s_Antebrazo' % side] = (rx(0.8 * s), 0.0)
        p['PataTra_%s_Muslo' % side] = (rx(-0.8 * s), 0.0)
        p['PataTra_%s_Pierna' % side] = (rx(0.8 * s), 0.0)
    return p


def ease(x):
    x = max(0.0, min(1.0, x))
    return x * x * (3 - 2 * x)


def pose_roar(t):
    # 0-0.25 carga, 0.25-0.7 rugido con temblor, 0.7-1 vuelve
    up = ease(t / 0.25) * (1 - ease((t - 0.7) / 0.3))
    shake = math.sin(TAU * t * 14) * up * 1.2
    p = {
        'Torso': (rx(-4 * up), 0.05 * up),
        'Cuello': (rx(-10 * up), 0.0),
        'Cabeza': (rx(-14 * up + shake) @ rz(shake * 0.6), 0.0),
        'Mandibula': (rx(26 * up), 0.0),
    }
    for i, n in enumerate(tail_names):
        p[n] = (rx(-6 * up * (1 + 0.3 * i)) @ rz(shake * (1 + i)), 0.0)
    for side, sg in (('Izq', 1), ('Der', -1)):
        p['PataDel_%s_Brazo' % side] = (rx(8 * up), 0.0)
        p['PataDel_%s_Antebrazo' % side] = (rx(-10 * up), 0.0)
        p['PataDel_%s_Mano' % side] = (rx(4 * up), 0.0)
    return p


# ---------------------------------------------------------------- caminata
# Paso lateral de cuadrupedo pesado (como un elefante): trasera izq -> delantera izq ->
# trasera der -> delantera der. En el lugar (in place): el juego mueve el modelo.
WALK_FRAMES = 36            # 1,2 s por ciclo a 30 fps
WALK_BETA = 0.70            # fraccion del ciclo con la pata apoyada
WALK_STRIDE = 0.50          # largo del paso (m) medido en la muneca/tobillo
WALK_LIFT = 0.20            # altura maxima del pie al avanzar (m)
WALK_CROUCH = 0.07          # el cuerpo baja un poco al caminar (da juego a las rodillas)
WALK_SPEED = WALK_STRIDE / (WALK_BETA * WALK_FRAMES / FPS)  # m/s sin que patinen los pies
# fase de cada pata (u = t + fase); el pie se apoya cuando u = 0
WALK_LEGS = {
    ('PataTra', 'Izq'): 0.00, ('PataDel', 'Izq'): 0.75,
    ('PataTra', 'Der'): 0.50, ('PataDel', 'Der'): 0.25,
}
LEG_BONES = {'PataDel': ('Brazo', 'Antebrazo', 'Mano'), 'PataTra': ('Muslo', 'Pierna', 'Pie')}


def ry(deg):
    return Quaternion((0, 1, 0), math.radians(deg))


def _wrap(a):
    return (a + math.pi) % (2 * math.pi) - math.pi


def leg_ik(S, Kn, W, T, knee_back):
    """IK de 2 huesos en el plano sagital (YZ). Devuelve los giros en X del hueso superior
    y del inferior. knee_back: True si la articulacion intermedia debe quedar hacia atras."""
    u0, v0 = Kn - S, W - Kn
    L1, L2 = math.hypot(u0.y, u0.z), math.hypot(v0.y, v0.z)
    dy, dz = T.y - S.y, T.z - S.z
    D = min(max(math.hypot(dy, dz), abs(L1 - L2) + 1e-3), L1 + L2 - 1e-4)
    phi = math.atan2(dz, dy)
    alpha = math.acos(max(-1.0, min(1.0, (L1 * L1 + D * D - L2 * L2) / (2 * L1 * D))))
    best = None
    for sgn in (1, -1):
        tu = phi + sgn * alpha
        ky, kz = S.y + L1 * math.cos(tu), S.z + L1 * math.sin(tu)
        score = ky if knee_back else -ky
        if best is None or score > best[0]:
            best = (score, tu, ky, kz)
    _s, tu, ky, kz = best
    ty, tz = S.y + D * math.cos(phi), S.z + D * math.sin(phi)
    tv = math.atan2(tz - kz, ty - ky)
    a1 = _wrap(tu - math.atan2(u0.z, u0.y))
    a2 = _wrap(tv - math.atan2(v0.z, v0.y) - a1)
    return a1, a2


def foot_track(u):
    """Trayectoria de la muneca/tobillo relativa al reposo: (dy, dz, inclinacion del pie)."""
    if u < WALK_BETA:  # apoyo: el pie retrocede pegado al suelo
        s = u / WALK_BETA
        return -WALK_STRIDE / 2 + WALK_STRIDE * s, 0.0, 0.0
    s = (u - WALK_BETA) / (1 - WALK_BETA)  # vuelo: se levanta y avanza
    e = s * s * (3 - 2 * s)
    return WALK_STRIDE / 2 - WALK_STRIDE * e, WALK_LIFT * math.sin(math.pi * s), 22.0 * math.sin(math.pi * s)


REST = {b.name: b.head_local.copy() for b in arm_data.bones}
TORSO_HEAD = REST['Torso']


def pose_walk(t):
    w = TAU * t
    bob = -WALK_CROUCH + 0.018 * math.cos(2 * w)
    pitch, roll, yaw = 0.8 * math.sin(2 * w), 1.8 * math.sin(w), 1.5 * math.cos(w)
    q_t = rz(yaw) @ ry(roll) @ rx(pitch)
    q_inv = q_t.inverted()
    p = {
        'Torso': (q_t, bob),
        'Cuello': (rz(-0.8 * yaw) @ rx(-0.6 * pitch + 1.2 * math.sin(2 * w + 0.6)), 0.0),
        'Cabeza': (rx(-1.5 * math.cos(2 * w + 0.9)) @ rz(-0.5 * yaw), 0.0),
        'Mandibula': (rx(3.0 + 1.5 * math.sin(2 * w)), 0.0),
    }
    for i, n in enumerate(tail_names):
        p[n] = (rz((5 + 2.2 * i) * math.sin(w - 0.7 * (i + 1))) @ rx(1.2 * math.sin(2 * w - 0.5 * i)), 0.0)
    for (pre, side), phase in WALK_LEGS.items():
        b1, b2, b3 = ('%s_%s_%s' % (pre, side, x) for x in LEG_BONES[pre])
        S, Kn, W = REST[b1], REST[b2], REST[b3]
        dy, dz, foot_pitch = foot_track((t + phase) % 1.0)
        target_world = W + Vector((0, dy, dz))
        # objetivo expresado en el marco del torso ya posado (sube/baja y se inclina)
        local = TORSO_HEAD + q_inv @ (target_world - Vector((0, 0, bob)) - TORSO_HEAD)
        a1, a2 = leg_ik(S, Kn, W, local, knee_back=(pre == 'PataDel'))
        a3 = math.radians(foot_pitch) - math.radians(pitch) - a1 - a2
        p[b1] = (Quaternion((1, 0, 0), a1), 0.0)
        p[b2] = (Quaternion((1, 0, 0), a2), 0.0)
        p[b3] = (Quaternion((1, 0, 0), a3), 0.0)
    return p


idle = make_action('Reposo', 60, pose_idle)
roar = make_action('Rugido', 75, pose_roar)
walk = make_action('Caminar', WALK_FRAMES, pose_walk)
print('velocidad de caminata sin patinar: %.3f m/s (%.3f m por ciclo)' % (WALK_SPEED, WALK_SPEED * WALK_FRAMES / FPS))
arm.animation_data.action = idle

rig_objs = [m for m, _b in rig_meshes]
export_fbx(os.path.join(OUT, 'Basaltor_Roblox_Rig.fbx'), rig_objs, armature=arm, anim=True, action=idle)
export_fbx(os.path.join(OUT, 'Basaltor_Anim_Reposo.fbx'), rig_objs, armature=arm, anim=True, action=idle)
export_fbx(os.path.join(OUT, 'Basaltor_Anim_Rugido.fbx'), rig_objs, armature=arm, anim=True, action=roar)
export_fbx(os.path.join(OUT, 'Basaltor_Anim_Caminar.fbx'), rig_objs, armature=arm, anim=True, action=walk)
arm.animation_data.action = idle

# glb: materiales originales + ambas animaciones
bpy.ops.object.select_all(action='DESELECT')
for go, _b in glb_meshes:
    go.select_set(True)
arm.select_set(True)
bpy.context.view_layer.objects.active = arm
bpy.ops.export_scene.gltf(filepath=os.path.join(OUT, 'Basaltor.glb'), export_format='GLB',
                          use_selection=True, export_animations=True, export_animation_mode='ACTIONS',
                          export_yup=True, export_apply=False, export_skins=True)

# reporte
print('\nMALLAS ROBLOX (rig):')
tot = 0
for mo, b in sorted(rig_meshes, key=lambda x: x[0].name):
    t = len(mo.data.polygons)
    tot += t
    print('  %-28s %6d tris  hueso=%s' % (mo.name, t, b))
print('  TOTAL', tot, 'tris en', len(rig_meshes), 'MeshParts')
print('huesos:', len(BONES))
# ordenar el .blend del rig: version bonita visible, version Roblox en su propia coleccion
bonito = bpy.data.collections.new('Rig_Materiales_Originales')
scene.collection.children.link(bonito)
for go, _b in glb_meshes:
    rig_col.objects.unlink(go)
    bonito.objects.link(go)
for o in list(parts.values()) + [root]:
    bpy.data.objects.remove(o)
for c in list(bpy.data.collections):
    if c.name in ('Basaltor', 'Cuerpo', 'Cabeza', 'Patas_Delanteras', 'Patas_Traseras', 'Cola', 'Roblox_Partes'):
        bpy.data.collections.remove(c)
for go, _b in glb_meshes:
    go.hide_viewport = False
    go.hide_render = False
for mo, _b in rig_meshes:
    mo.hide_render = True
    mo.hide_viewport = True
try:
    bpy.ops.file.pack_all()
except Exception as e:
    print('pack_all:', e)
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT, 'Basaltor_Rig.blend'))
