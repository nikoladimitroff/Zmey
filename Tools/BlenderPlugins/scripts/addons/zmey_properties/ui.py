import bpy

# Lists
class ZmeySceneTypeList(bpy.types.UIList):
    def draw_item(self, context, layout, data, item, icon, active_data, active_propname):
        if item:
            layout.prop(item, "name", text="", emboss=False)

# Panels
class ZmeyWorldPropertyPanel(bpy.types.Panel):
    """Zmey Scene Type Properties"""
    bl_label = "Zmey World Types"
    bl_idname = "ZMEY_SCENE_TYPES"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "world"

    @classmethod
    def poll(cls, context):
        return context.scene

    def draw(self, context):
        layout = self.layout
        world = context.world
        zmey = world.zmey_scene_types

        layout.row().prop(zmey, "name")
        layout.row().prop(zmey, "types")

        row = layout.row()
        row.template_list("ZmeySceneTypeList", "", zmey, "types", zmey, "active_type")
        column = row.column()
        column.operator("zmey.add_scene_type", icon="ZOOMIN", text="")
        column.operator("zmey.remove_scene_type", icon="ZOOMOUT", text="")

        if len(zmey.types):
            self.draw_type(zmey.types[zmey.active_type], layout.box())

    def draw_type(self, item, layout):
        layout.row().prop(item, "name")
        mesh_box = layout.box()
        mesh_box.row().prop(item, "mesh_reference")
        item.components.draw_type(layout)

class ZmeyObjectPropertiesPanel(bpy.types.Panel):
    """Zmey Object Properties"""
    bl_label = "Zmey Object"
    bl_idname = "ZMEY_OBJECT"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    def draw(self, context):
        layout = self.layout
        obj = context.object

        layout.row().prop(obj.zmey_props, "enabled", toggle=True)

        if obj.zmey_props.enabled:
            layout.row().prop(obj.zmey_props, "type")
            box = layout.box()
            zmey_type = bpy.context.scene.world.zmey_scene_types.types[int(obj.zmey_props.type)]
            box.box().prop(
                obj.zmey_props,
                "mesh_export",
                text="Export Mesh" if zmey_type.mesh_reference == None else "Override Type Mesh")
            obj.zmey_props.components.draw_type(box)


def register():
    bpy.utils.register_class(ZmeySceneTypeList)
    bpy.utils.register_class(ZmeyWorldPropertyPanel)
    bpy.utils.register_class(ZmeyObjectPropertiesPanel)

def unregister():
    bpy.utils.unregister_class(ZmeySceneTypeList)
    bpy.utils.unregister_class(ZmeyWorldPropertyPanel)
    bpy.utils.unregister_class(ZmeyObjectPropertiesPanel)