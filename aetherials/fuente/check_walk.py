import bpy, sys
args = sys.argv[sys.argv.index('--') + 1:] if '--' in sys.argv else []
BLEND = args[0] if args else 'out/Basaltor_Rig.blend'
bpy.ops.wm.open_mainfile(filepath=BLEND)
arm = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
N = int(bpy.data.actions['Caminar'].frame_range[1])
arm.animation_data.action = bpy.data.actions['Caminar']
sc = bpy.context.scene
feet = ['PataTra_Izq_Pie', 'PataDel_Izq_Mano', 'PataTra_Der_Pie', 'PataDel_Der_Mano']
print('frame  ' + '  '.join('%-22s' % f for f in feet))
prev = None
for f in range(0, N + 1, 3):
    sc.frame_set(f)
    row = []
    for n in feet:
        pb = arm.pose.bones[n]
        h = arm.matrix_world @ pb.head
        # punta del pie (cola del hueso) para ver la inclinacion
        tl = arm.matrix_world @ pb.tail
        row.append('y=%+.3f z=%.3f tz=%.3f' % (h.y, h.z, tl.z))
    print('%3d   ' % f + '  '.join(row))
