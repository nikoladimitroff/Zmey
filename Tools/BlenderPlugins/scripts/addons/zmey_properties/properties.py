import bpy
from . import components

class ZmeyComponentHolder(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        for comp_type, name in components.zmey_component_list:
            setattr(cls, name, bpy.props.PointerProperty(type=comp_type))
            setattr(cls, name + "_enabled", bpy.props.BoolProperty(name=name.capitalize()))

    def draw_type(self, layout):
        for _, name in components.zmey_component_list:
            component_enabled_name = name + "_enabled"
            innerbox = layout.box()
            innerbox.row().prop(self, component_enabled_name)
            if getattr(self, component_enabled_name):
                getattr(self, name).draw(innerbox)

class ZmeySceneType(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        cls.name = bpy.props.StringProperty(name="Name", default="New Type")
        cls.components = bpy.props.PointerProperty(type=ZmeyComponentHolder)

class ZmeySceneTypesProperty(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.World.zmey_scene_types = bpy.props.PointerProperty(
            name="Zmey Scene Types",
            description="Zmey Scene Types",
            type=cls,
            )
        cls.name = bpy.props.StringProperty(name="World Name")
        cls.types = bpy.props.CollectionProperty(type=ZmeySceneType)
        cls.active_type = bpy.props.IntProperty(name="index")

    @classmethod
    def unregister(cls):
        del bpy.types.World.zmey_scene_types

def get_available_zmey_types(self, context):
    types = bpy.context.scene.world.zmey_scene_types.types
    return_value = [(str(i), t.name, t.name) for i, t in enumerate(types)]
    return return_value

def change_mesh_from_type(self, context):
    type_index = int(self.type)
    new_type = context.scene.world.zmey_scene_types.types[type_index]
    obj = context.object
    if new_type.components.mesh_enabled and not self.components.mesh_enabled:
        obj.data = new_type.components.mesh.mesh

class ZmeyObjectProperty(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Object.zmey_props = bpy.props.PointerProperty(
            name="Zmey Object Properties",
            description="Zmey Object Properties",
            type=cls,
        )
        cls.type = bpy.props.EnumProperty(
            name="Object Type",
            items=get_available_zmey_types,
            update=change_mesh_from_type,
        )
        cls.name=bpy.props.StringProperty(
            name="Object Name"
        )
        cls.components=bpy.props.PointerProperty(type=ZmeyComponentHolder)

def register():
    bpy.utils.register_class(ZmeyComponentHolder)
    bpy.utils.register_class(ZmeySceneType)
    bpy.utils.register_class(ZmeySceneTypesProperty)
    bpy.utils.register_class(ZmeyObjectProperty)

def unregister():
    bpy.utils.unregister_class(ZmeyComponentHolder)
    bpy.utils.unregister_class(ZmeySceneType)
    bpy.utils.unregister_class(ZmeySceneTypesProperty)
    bpy.utils.unregister_class(ZmeyObjectProperty)