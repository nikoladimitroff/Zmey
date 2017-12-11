import bpy

# Transform property is taken from the blender/gltf directly
# Assume every object has transform component

class ZmeyComponent_Physics(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        pass

    def draw(self, layout):
        pass

    def export(self):
        return { "name" : "physics" }

class ZmeyComponent_Mesh(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        cls.mesh = bpy.props.PointerProperty(type=bpy.types.Mesh)

    def draw(self, layout):
        layout.prop(self, "mesh")

    def export(self):
        # This is special component, as it is exported explicitly
        return None

class ZmeyComponent_Tag(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        cls.tag = bpy.props.StringProperty(name="Tag")

    def draw(self, layout):
        layout.prop(self, "tag")

    def export(self):
        return {"name": "tag", "tags" : [self.tag]}

class ZmeyComponent_Projectile(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        pass

    def draw(self, layout):
        pass

    def export(self):
        return { "name" : "projectile" }

zmey_component_list = [
    (ZmeyComponent_Physics, "physics"),
    (ZmeyComponent_Mesh, "mesh"),
    (ZmeyComponent_Tag, "tag"),
    (ZmeyComponent_Projectile, "projectile")
    ]

def register():
    bpy.utils.register_class(ZmeyComponent_Mesh)
    bpy.utils.register_class(ZmeyComponent_Physics)
    bpy.utils.register_class(ZmeyComponent_Tag)
    bpy.utils.register_class(ZmeyComponent_Projectile)

def unregister():
    bpy.utils.unregister_class(ZmeyComponent_Mesh)
    bpy.utils.unregister_class(ZmeyComponent_Physics)
    bpy.utils.unregister_class(ZmeyComponent_Tag)
    bpy.utils.unregister_class(ZmeyComponent_Projectile)