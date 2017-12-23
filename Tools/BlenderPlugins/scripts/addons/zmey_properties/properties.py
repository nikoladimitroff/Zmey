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

    def export(self):
        component_list = []
        for _, name in components.zmey_component_list:
            component_enabled_name = name + "_enabled"
            if getattr(self, component_enabled_name):
                exported = getattr(self, name).export()
                if exported is not None:
                    component_list.append(exported)

        return component_list

def change_mesh_from_type_for_obj(self, obj):
    type_index = int(self.type)
    new_type = bpy.context.scene.world.zmey_scene_types.types[type_index]
    if new_type.mesh_reference != None and not self.mesh_export:
        obj.data = new_type.mesh_reference.data

def get_index_for_type_name(name):
    types = [t.name for t in ( bpy.context.scene.world.zmey_scene_types.types)]
    return types.index(name)

def change_all_meshes_for_type(self, context):
    type_index = get_index_for_type_name(self.name)
    for blender_object in bpy.data.objects:
        # skip all not used object, not enabled for exporting
        # having different type, or have override on the mesh
        if blender_object.users == 0 or \
                not blender_object.zmey_props.enabled or \
                int(blender_object.zmey_props.type) != type_index or \
                blender_object.zmey_props.mesh_export:
            continue
        change_mesh_from_type_for_obj(blender_object.zmey_props, blender_object)

class ZmeySceneType(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        cls.name = bpy.props.StringProperty(name="Name", default="New Type")
        cls.components = bpy.props.PointerProperty(type=ZmeyComponentHolder)
        cls.mesh_reference = bpy.props.PointerProperty(
            type=bpy.types.Object,
            name="Reference Mesh",
            description="Will use this mesh for all types if set to anything.",
            update=change_all_meshes_for_type
        )

    def export(self):
        data = { "version" : "1.0",
         "name" : self.name,
         "extends" : "",
         "components" : self.components.export()
        }

        return data

class ZmeyWorldProperty(bpy.types.PropertyGroup):
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
    change_mesh_from_type_for_obj(self, context.object)

class ZmeyObjectProperty(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Object.zmey_props = bpy.props.PointerProperty(
            name="Zmey Object Properties",
            description="Zmey Object Properties",
            type=cls,
        )
        cls.enabled = bpy.props.BoolProperty(
            name="Object Enabled"
        )
        cls.type = bpy.props.EnumProperty(
            name="Object Type",
            items=get_available_zmey_types,
            update=change_mesh_from_type,
        )
        cls.components=bpy.props.PointerProperty(type=ZmeyComponentHolder)
        cls.mesh_export=bpy.props.BoolProperty(
            name="Export Mesh",
            description="Export mesh for this specific type. If Mesh is specified by the type, this will override this specific object.",
            update=change_mesh_from_type)

    @classmethod
    def unregister(cls):
        del bpy.types.Object.zmey_props

def register():
    bpy.utils.register_class(ZmeyComponentHolder)
    bpy.utils.register_class(ZmeySceneType)
    bpy.utils.register_class(ZmeyWorldProperty)
    bpy.utils.register_class(ZmeyObjectProperty)

def unregister():
    bpy.utils.unregister_class(ZmeyComponentHolder)
    bpy.utils.unregister_class(ZmeySceneType)
    bpy.utils.unregister_class(ZmeyWorldProperty)
    bpy.utils.unregister_class(ZmeyObjectProperty)