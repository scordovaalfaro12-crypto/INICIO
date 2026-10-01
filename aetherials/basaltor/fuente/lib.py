"""Helpers de modelado procedural para Basaltor (Blender 5.0, bpy)."""
import bpy, bmesh, math, random
from mathutils import Vector, Matrix, Quaternion, Euler, noise
from mathutils.bvhtree import BVHTree


# ---------------------------------------------------------------- materiales
def srgb(h):
    h = h.lstrip('#')
    c = [int(h[i:i + 2], 16) / 255 for i in (0, 2, 4)]
    return tuple(x / 12.92 if x <= 0.04045 else ((x + 0.055) / 1.055) ** 2.4 for x in c)


# nombre, color sRGB, rugosidad, emision (fuerza)
PALETTE = [
    ('Cuerpo',        '#1b3150', 0.62, 0),
    ('CuerpoOscuro',  '#122236', 0.66, 0),
    ('Placa',         '#3f6795', 0.55, 0),
    ('PlacaClara',    '#5b80a9', 0.55, 0),
    ('CabezaGris',    '#6e7a86', 0.60, 0),
    ('Hocico',        '#23282d', 0.55, 0),
    ('Vientre',       '#3b4c62', 0.70, 0),
    ('VientreBanda',  '#2b3a4e', 0.70, 0),
    ('Hueso',         '#e4d7be', 0.45, 0),
    ('HuesoOscuro',   '#b39d7a', 0.50, 0),
    ('Ojo',           '#efd9ae', 0.35, 0.15),
    ('Pupila',        '#1b2633', 0.30, 0),
    ('Brillo',        '#ffffff', 0.20, 1.5),
    ('Magma',         '#ff6a2c', 0.50, 7.0),
    ('MagmaCaliente', '#ff8f3a', 0.50, 9.0),
    ('MagmaProfundo', '#d05605', 0.55, 0.35),
    ('Basalto',       '#25364c', 0.60, 0),
    ('BasaltoCima',   '#6782a4', 0.50, 0),
]
MI = {n: i for i, (n, *_r) in enumerate(PALETTE)}
GLOW = {'Magma', 'MagmaCaliente', 'MagmaProfundo', 'Brillo'}


def build_materials():
    mats = []
    for name, hexc, rough, emis in PALETTE:
        m = bpy.data.materials.get('M_' + name) or bpy.data.materials.new('M_' + name)
        try:
            m.use_nodes = True
        except Exception:
            pass
        bsdf = next(n for n in m.node_tree.nodes if n.type == 'BSDF_PRINCIPLED')
        col = srgb(hexc) + (1.0,)
        bsdf.inputs['Base Color'].default_value = col
        bsdf.inputs['Roughness'].default_value = rough
        bsdf.inputs['Specular IOR Level'].default_value = 0.35
        if emis:
            bsdf.inputs['Emission Color'].default_value = col
            bsdf.inputs['Emission Strength'].default_value = emis
        m.diffuse_color = col
        mats.append(m)
    return mats


# ---------------------------------------------------------------- escena
def collection(name, parent=None):
    parent = parent or bpy.context.scene.collection
    c = bpy.data.collections.get(name)
    if c is None:
        c = bpy.data.collections.new(name)
        parent.children.link(c)
    return c


def new_object(name, me, coll, mats):
    o = bpy.data.objects.new(name, me)
    coll.objects.link(o)
    if not me.materials:
        for m in mats:
            me.materials.append(m)
    return o


def apply_modifiers(o):
    dg = bpy.context.evaluated_depsgraph_get()
    me = bpy.data.meshes.new_from_object(o.evaluated_get(dg))
    old = o.data
    o.modifiers.clear()
    o.data = me
    me.name = o.name
    if old.users == 0:
        bpy.data.meshes.remove(old)


# ---------------------------------------------------------------- formas base
def metaball_mesh(name, elems, res=0.025, threshold=0.6):
    """elems: dicts con co, r, type, size=(x,y,z), rot=Quaternion, stiff, neg"""
    mb = bpy.data.metaballs.new('MB' + name)
    mb.resolution = res
    mb.render_resolution = res
    mb.threshold = threshold
    o = bpy.data.objects.new('MB' + name, mb)
    bpy.context.scene.collection.objects.link(o)
    for el in elems:
        e = mb.elements.new(type=el.get('type', 'BALL'))
        e.co = el['co']
        e.radius = el['r']
        if 'size' in el:
            e.size_x, e.size_y, e.size_z = el['size']
        if 'rot' in el:
            e.rotation = el['rot']
        e.stiffness = el.get('stiff', 2.0)
        e.use_negative = el.get('neg', False)
    dg = bpy.context.evaluated_depsgraph_get()
    me = bpy.data.meshes.new_from_object(o.evaluated_get(dg))
    bpy.data.objects.remove(o)
    bpy.data.metaballs.remove(mb)
    me.name = name
    return me


def capsule(a, b, r, stiff=2.0, neg=False):
    """Elemento metaball tipo capsula entre los puntos a y b."""
    a, b = Vector(a), Vector(b)
    d = b - a
    q = d.to_track_quat('X', 'Z')
    return {'type': 'CAPSULE', 'co': (a + b) / 2, 'r': r, 'size': (d.length / 2, 0, 0),
            'rot': q, 'stiff': stiff, 'neg': neg}


def ellipsoid(co, r, sx, sy, sz, rot=None, stiff=2.0, neg=False):
    e = {'type': 'ELLIPSOID', 'co': co, 'r': r, 'size': (sx, sy, sz), 'stiff': stiff, 'neg': neg}
    if rot is not None:
        e['rot'] = Euler(rot).to_quaternion()
    return e


def ball(co, r, stiff=2.0, neg=False):
    return {'type': 'BALL', 'co': co, 'r': r, 'stiff': stiff, 'neg': neg}


def remesh(o, voxel):
    m = o.modifiers.new('Remesh', 'REMESH')
    m.mode = 'VOXEL'
    m.voxel_size = voxel
    apply_modifiers(o)


def rock_displace(o, scale=3.0, strength=0.03, cells=0.0, cell_scale=4.0, seed=0, sym=True,
                  falloff=None):
    """Ruido fractal + celulas (Voronoi) a lo largo de la normal: aspecto de roca tallada.
    Simetrico en X cuando sym=True."""
    bm = bmesh.new()
    bm.from_mesh(o.data)
    bm.normal_update()
    off = Vector((seed * 7.31, seed * 3.17, seed * 5.93))
    for v in bm.verts:
        p = Vector((abs(v.co.x) if sym else v.co.x, v.co.y, v.co.z))
        d = noise.fractal(p * scale + off, 0.6, 2.1, 4, noise_basis='PERLIN_ORIGINAL') * strength
        if cells:
            dist, _pts = noise.voronoi(p * cell_scale + off, distance_metric='DISTANCE', exponent=2.5)
            d += (dist[1] - dist[0]) * cells  # cresta en el centro de cada celda, surco en los bordes
        if falloff:
            d *= falloff(v.co)
        v.co += v.normal * d
    bm.to_mesh(o.data)
    bm.free()


def decimate(o, tris, sym=True):
    cur = sum(len(p.vertices) - 2 for p in o.data.polygons)
    if cur > tris:
        m = o.modifiers.new('Decimate', 'DECIMATE')
        m.decimate_type = 'COLLAPSE'
        m.ratio = tris / cur
        m.use_symmetry = sym
        m.symmetry_axis = 'X'
        m.use_collapse_triangulate = True
        apply_modifiers(o)


def symmetrize(o, direction='X'):
    bm = bmesh.new()
    bm.from_mesh(o.data)
    bmesh.ops.symmetrize(bm, input=bm.verts[:] + bm.edges[:] + bm.faces[:], direction=direction, dist=1e-4)
    bmesh.ops.triangulate(bm, faces=[f for f in bm.faces if len(f.verts) > 4])
    bm.to_mesh(o.data)
    bm.free()


def flat(o):
    o.data.polygons.foreach_set('use_smooth', [False] * len(o.data.polygons))


def lowpoly(o, tris, voxel=0.02, disp=None, sym=True):
    """Pipeline: remesh voxel -> desplazamiento roca -> decimate -> simetria -> flat."""
    remesh(o, voxel)
    if disp:
        rock_displace(o, sym=sym, **disp)
    decimate(o, tris, sym=sym)
    if sym:
        symmetrize(o)
    flat(o)


def bisect(o, co, no, keep_positive=True, fill=True, mat=None):
    bm = bmesh.new()
    bm.from_mesh(o.data)
    geom = bm.verts[:] + bm.edges[:] + bm.faces[:]
    r = bmesh.ops.bisect_plane(bm, geom=geom, plane_co=Vector(co), plane_no=Vector(no),
                               clear_outer=not keep_positive, clear_inner=keep_positive, dist=1e-5)
    if fill:
        edges = [e for e in r['geom_cut'] if isinstance(e, bmesh.types.BMEdge)]
        if edges:
            f = bmesh.ops.holes_fill(bm, edges=edges, sides=0)
            nf = f['faces']
            if nf:
                bmesh.ops.triangulate(bm, faces=nf, quad_method='BEAUTY', ngon_method='BEAUTY')
                if mat is not None:
                    for fc in bm.faces:
                        if fc.material_index == -1:
                            pass
                for fc in nf:
                    if fc.is_valid and mat is not None:
                        fc.material_index = mat
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
    bm.to_mesh(o.data)
    bm.free()


def set_material(o, mi):
    o.data.polygons.foreach_set('material_index', [mi] * len(o.data.polygons))


def paint_faces(o, fn):
    """fn(center, normal) -> indice de material o None."""
    for p in o.data.polygons:
        r = fn(p.center, p.normal)
        if r is not None:
            p.material_index = r


# ---------------------------------------------------------------- bmesh piezas
def bm_from(o):
    bm = bmesh.new()
    bm.from_mesh(o.data)
    return bm


def bm_commit(o, bm):
    bm.to_mesh(o.data)
    bm.free()
    o.data.update()


def align_matrix(loc, normal, spin=0.0, scale=(1, 1, 1), up_hint=None):
    n = Vector(normal).normalized()
    q = n.to_track_quat('Z', 'Y')
    m = Matrix.Translation(Vector(loc)) @ q.to_matrix().to_4x4() @ Matrix.Rotation(spin, 4, 'Z')
    return m @ Matrix.Diagonal(Vector(scale).to_4d())


def _ring(bm, n, radii, z, jitter, rng, phase=0.0, center=(0, 0)):
    vs = []
    for i in range(n):
        a = phase + 2 * math.pi * i / n + rng.uniform(-jitter, jitter) * 0.5
        rr = radii * (1 + rng.uniform(-jitter, jitter))
        vs.append(bm.verts.new((center[0] + rr * math.cos(a), center[1] + rr * math.sin(a), z)))
    return vs


def _bridge(bm, ra, rb, mat):
    n = len(ra)
    fs = []
    for i in range(n):
        j = (i + 1) % n
        f = bm.faces.new((ra[i], ra[j], rb[j], rb[i]))
        f.material_index = mat
        fs.append(f)
    return fs


def _cap(bm, ring, mat, flip=False):
    f = bm.faces.new(list(reversed(ring)) if flip else ring)
    f.material_index = mat
    return f


def add_dome_plate(bm, M, rng, sides=7, height=0.55, mat=MI['Placa'], rim=MI['Magma'],
                   rim_w=0.045, rim_h=0.035, peak=0.25):
    """Placa de roca abovedada (como las de Vulcanid) con un borde de magma debajo.
    Unidad: radio 1. M = matriz de colocacion (z = normal de la superficie)."""
    new = []
    phase = rng.uniform(0, math.pi)
    # faldon de magma: sobresale apenas del borde de la placa
    r_in = _ring(bm, sides, 1.0 + rim_w, -0.35, 0.0, rng, phase)
    r_out = _ring(bm, sides, 1.0 + rim_w, rim_h, 0.0, rng, phase)
    new += r_in + r_out
    fs = _bridge(bm, r_in, r_out, rim)
    # placa
    b0 = _ring(bm, sides, 1.0, rim_h * 0.6, 0.06, rng, phase)
    b1 = _ring(bm, sides, 0.86, height * 0.45, 0.08, rng, phase + 0.2)
    b2 = _ring(bm, sides, 0.48, height * 0.85, 0.12, rng, phase + 0.5)
    top = bm.verts.new((rng.uniform(-0.1, 0.1), rng.uniform(-0.1, 0.1), height * (1 + peak * rng.uniform(0, 1))))
    bot = _ring(bm, sides, 0.9, -0.35, 0.0, rng, phase)
    new += b0 + b1 + b2 + [top] + bot
    fs += _bridge(bm, bot, b0, mat)
    fs += _bridge(bm, b0, b1, mat)
    fs += _bridge(bm, b1, b2, mat)
    for i in range(sides):
        f = bm.faces.new((b2[i], b2[(i + 1) % sides], top))
        f.material_index = mat
        fs.append(f)
    fs.append(_cap(bm, bot, mat, flip=True))
    fs.append(_cap(bm, r_in, rim, flip=True))
    bmesh.ops.transform(bm, matrix=M, verts=new)
    bmesh.ops.triangulate(bm, faces=fs)
    return new


def add_hex_column(bm, M, rng, height=1.0, slant=0.35, mat=MI['Basalto'], top=MI['BasaltoCima'],
                   rim=MI['Magma'], rim_w=0.18, chamfer=0.18, sides=6, phase=None, depth=1.2, rim_h=0.08):
    """Columna de basalto hexagonal (radio 1) con tapa inclinada biselada y anillo de magma en la base."""
    new = []
    phase = rng.uniform(0, math.pi) if phase is None else phase
    sx, sy = rng.uniform(-1, 1), rng.uniform(-1, 1)
    sl = Vector((sx, sy)).normalized() * slant if (sx or sy) else Vector((slant, 0))

    def zt(x, y):
        return height + sl.x * x + sl.y * y

    bot = _ring(bm, sides, 1.0, -depth, 0.0, rng, phase)
    ring_lo = _ring(bm, sides, 1.0 + rim_w, -depth, 0.0, rng, phase)
    ring_hi = _ring(bm, sides, 1.0 + rim_w, rim_h, 0.0, rng, phase)
    side_lo = _ring(bm, sides, 1.0, rim_h * 0.6, 0.0, rng, phase)
    new += bot + ring_lo + ring_hi + side_lo
    fs = _bridge(bm, ring_lo, ring_hi, rim)
    fs.append(_cap(bm, ring_lo, rim, flip=True))
    # cuerpo
    side_hi = []
    for v in side_lo:
        x, y = v.co.x, v.co.y
        side_hi.append(bm.verts.new((x, y, zt(x, y) - chamfer)))
    cap = []
    for v in side_lo:
        x, y = v.co.x * (1 - chamfer * 0.9), v.co.y * (1 - chamfer * 0.9)
        cap.append(bm.verts.new((x, y, zt(x, y))))
    new += side_hi + cap
    fs += _bridge(bm, bot, side_lo, mat)
    fs += _bridge(bm, side_lo, side_hi, mat)
    fs += _bridge(bm, side_hi, cap, top)
    fs.append(_cap(bm, cap, top))
    fs.append(_cap(bm, bot, mat, flip=True))
    bmesh.ops.transform(bm, matrix=M, verts=new)
    bmesh.ops.triangulate(bm, faces=fs)
    return new


def add_claw(bm, M, rng, length=1.0, curve=0.35, segs=4, sides=4, mat=MI['Hueso'], base=0.22):
    """Garra conica curvada (eje +Y, curva hacia -Z)."""
    new = []
    rings = []
    for s in range(segs + 1):
        t = s / segs
        r = base * (1 - t) ** 0.85 + 0.004
        y = length * t
        z = -curve * length * t * t
        ring = []
        for i in range(sides):
            a = math.pi / 4 + 2 * math.pi * i / sides
            ring.append(bm.verts.new((r * math.cos(a) * 1.15, y, z + r * math.sin(a))))
        rings.append(ring)
        new += ring
    fs = []
    for a, b in zip(rings, rings[1:]):
        fs += _bridge(bm, a, b, mat)
    tip = bm.verts.new((0, length * 1.06, -curve * length * 1.1))
    new.append(tip)
    for i in range(sides):
        f = bm.faces.new((rings[-1][i], rings[-1][(i + 1) % sides], tip))
        f.material_index = mat
        fs.append(f)
    fs.append(_cap(bm, rings[0], mat))
    bmesh.ops.recalc_face_normals(bm, faces=fs)
    bmesh.ops.transform(bm, matrix=M, verts=new)
    bmesh.ops.triangulate(bm, faces=fs)
    return new


def add_tube(bm, pts, radii, sides=6, mat=0, mat_fn=None, cap_start=True, cap_end=True, twist=0.0,
             flatten=1.0, up=Vector((0, 0, 1))):
    """Tubo a lo largo de una polilinea (cuernos, puas). mat_fn(t)->material por segmento."""
    pts = [Vector(p) for p in pts]
    rings = []
    new = []
    n = len(pts)
    for k, p in enumerate(pts):
        if k == 0:
            tng = pts[1] - pts[0]
        elif k == n - 1:
            tng = pts[-1] - pts[-2]
        else:
            tng = pts[k + 1] - pts[k - 1]
        tng.normalize()
        side = tng.cross(up)
        if side.length < 1e-4:
            side = tng.cross(Vector((0, 1, 0)))
        side.normalize()
        nrm = side.cross(tng).normalized()
        r = radii[k]
        ring = []
        for i in range(sides):
            a = 2 * math.pi * i / sides + twist * k
            off = side * math.cos(a) * r + nrm * math.sin(a) * r * flatten
            ring.append(bm.verts.new(p + off))
        rings.append(ring)
        new += ring
    fs = []
    for k in range(n - 1):
        m = mat_fn(k / (n - 1)) if mat_fn else mat
        fs += _bridge(bm, rings[k], rings[k + 1], m)
    if cap_start:
        fs.append(_cap(bm, rings[0], mat_fn(0) if mat_fn else mat, flip=True))
    if cap_end:
        fs.append(_cap(bm, rings[-1], mat_fn(1) if mat_fn else mat))
    bmesh.ops.recalc_face_normals(bm, faces=fs)
    bmesh.ops.triangulate(bm, faces=fs)
    return new


def bvh_of(o):
    bm = bmesh.new()
    bm.from_mesh(o.data)
    bm.transform(o.matrix_world)
    t = BVHTree.FromBMesh(bm)
    bm.free()
    return t


def surface_hit(bvh, origin, target):
    origin, target = Vector(origin), Vector(target)
    d = (target - origin).normalized()
    loc, nor, _i, _dist = bvh.ray_cast(origin, d)
    return loc, nor


def mirror_object_x(src, name, coll):
    """Duplica un objeto espejado en X (para el lado derecho)."""
    me = src.data.copy()
    bm = bmesh.new()
    bm.from_mesh(me)
    bmesh.ops.scale(bm, vec=Vector((-1, 1, 1)), verts=bm.verts[:])
    bmesh.ops.reverse_faces(bm, faces=bm.faces[:])
    bm.to_mesh(me)
    bm.free()
    me.name = name
    o = bpy.data.objects.new(name, me)
    coll.objects.link(o)
    return o


def tri_count(o):
    return sum(len(p.vertices) - 2 for p in o.data.polygons)
