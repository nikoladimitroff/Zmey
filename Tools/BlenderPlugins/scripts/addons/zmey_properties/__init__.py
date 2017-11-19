bl_info = {
    "name": "Zmey Properties",
    "category": "Game Engine",
}

if "bpy" in locals():
    import importlib
    importlib.reload(components)
    importlib.reload(properties)
    importlib.reload(operators)
    importlib.reload(ui)
else:
    from . import components, properties, operators, ui

import bpy
import logging

log = logging.getLogger("zmey")

def register():
    components.register()
    properties.register()
    operators.register()
    ui.register()

def unregister():
    components.unregister()
    properties.unregister()
    operators.unregister()
    ui.unregister()

if __name__ == "__main__":
    register()