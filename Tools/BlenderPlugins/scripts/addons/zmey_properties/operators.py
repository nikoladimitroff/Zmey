import bpy

class ZmeyAddSceneType(bpy.types.Operator):
    """Add New Type"""
    bl_idname = "zmey.add_scene_type"
    bl_label = "Add New Type"

    def execute(self, context):
        context.world.zmey_scene_types.types.add()
        return {'FINISHED'}

class ZmeyRemoveSceneType(bpy.types.Operator):
    """Add New Type"""
    bl_idname = "zmey.remove_scene_type"
    bl_label = "Remove New Type"

    def execute(self, context):
        context.world.zmey_scene_types.types.remove(context.world.zmey_scene_types.active_type)
        return {'FINISHED'}

def register():
    bpy.utils.register_class(ZmeyAddSceneType)
    bpy.utils.register_class(ZmeyRemoveSceneType)

def unregister():
    bpy.utils.unregister_class(ZmeyAddSceneType)
    bpy.utils.unregister_class(ZmeyRemoveSceneType)