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

#TODO : move in separate file with game specific components
class ZmeyComponent_ProjectileSpell(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
         cls.initial_speed = bpy.props.FloatProperty(name="Initial speed")
         cls.impact_damage = bpy.props.FloatProperty(name="Impact damage")
         cls.initial_mass = bpy.props.FloatProperty(name="initial mass")
         cls.cooldown_time = bpy.props.FloatProperty(name="Cooldown time")
         cls.life_time = bpy.props.FloatProperty(name="Lifetime")

    def draw(self, layout):
         layout.prop(self, "initial_speed")
         layout.prop(self, "impact_damage")
         layout.prop(self, "initial_mass")
         layout.prop(self, "cooldown_time")
         layout.prop(self, "life_time")

    def export(self):
        return { "name" : "projectilespell",
               "initial_speed" : self.initial_speed,
               "impact_damage" : self.impact_damage,
               "initial_mass" : self.initial_mass,
               "cooldown_time" : self.cooldown_time,
               "life_time" : self.life_time }

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
    (ZmeyComponent_Projectile, "projectile"),
    (ZmeyComponent_ProjectileSpell, "projectilespell")
    ]

def register():
    bpy.utils.register_class(ZmeyComponent_Physics)
    bpy.utils.register_class(ZmeyComponent_Tag)
    bpy.utils.register_class(ZmeyComponent_Projectile)
    bpy.utils.register_class(ZmeyComponent_ProjectileSpell)

def unregister():
    bpy.utils.unregister_class(ZmeyComponent_Physics)
    bpy.utils.unregister_class(ZmeyComponent_Tag)
    bpy.utils.unregister_class(ZmeyComponent_Projectile)
    bpy.utils.unregister_class(ZmeyComponent_ProjectileSpell)
