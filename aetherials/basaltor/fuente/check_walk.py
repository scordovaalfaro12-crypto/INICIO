import bpy, sys
bpy.ops.wm.open_mainfile(filepath='out/Basaltor_Rig.blend')
arm = bpy.data.objects['Basaltor_Rig']
arm.animation_data.action = bpy.data.actions['Caminar']
sc = bpy.context.scene
feet = ['PataTra_Izq_Pie', 'PataDel_Izq_Mano', 'PataTra_Der_Pie', 'PataDel_Der_Mano']
print('frame  ' + '  '.join('%-22s' % f for f in feet))
prev = None
for f in range(0, 37, 3):
    sc.frame_set(f)
    row = []
    for n in feet:
        pb = arm.pose.bones[n]
        h = arm.matrix_world @ pb.head
        # punta del pie (cola del hueso) para ver la inclinacion
        tl = arm.matrix_world @ pb.tail
        row.append('y=%+.3f z=%.3f tz=%.3f' % (h.y, h.z, tl.z))
    print('%3d   ' % f + '  '.join(row))
