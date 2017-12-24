import bpy

# Transform property is taken from the blender/gltf directly
# Assume every object has transform component

class ZmeyComponent_Physics(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
         cls.dynamic = bpy.props.BoolProperty(name="Dynamic")

    def draw(self, layout):
         layout.prop(self, "dynamic")

    def export(self):
        return { "name" : "physics", "dynamic" : self.dynamic}

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
    (ZmeyComponent_Tag, "tag"),
    (ZmeyComponent_Projectile, "projectile")
    ]

def register():
    bpy.utils.register_class(ZmeyComponent_Physics)
    bpy.utils.register_class(ZmeyComponent_Tag)
    bpy.utils.register_class(ZmeyComponent_Projectile)

def unregister():
    bpy.utils.unregister_class(ZmeyComponent_Physics)
    bpy.utils.unregister_class(ZmeyComponent_Tag)
    bpy.utils.unregister_class(ZmeyComponent_Projectile)