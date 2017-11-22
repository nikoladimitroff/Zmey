bl_info = {
    "name": "Zmey Properties",
    "category": "Import-Export",
    "location" : "File > Export",
    "description" : "Export as Zmey Props"
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
import json
import mathutils
from bpy_extras.io_utils import (ExportHelper)

log = logging.getLogger("zmey")

def prepare_object_transform(obj):
    translation, rotation, scale = obj.matrix_world.decompose()
    #TODO: check coordinate systems if correct
    # Put w at the end
    rotation = mathutils.Quaternion((rotation[1], rotation[2], rotation[3], rotation[0]))
    return_value = {
        "translation" : [translation[0], translation[1], translation[2]],
        "rotation" : [rotation[0], rotation[1], rotation[2], rotation[3]],
        "scale" : [scale[0], scale[1], scale[2]]
    }
    return return_value

class ExportZmey(bpy.types.Operator, ExportHelper):
    '''Export scene as Zmey props'''
    bl_idname = "export_scene.zmey"
    bl_label = "Export Zmey"

    filename_ext = ".world"

    directory = bpy.props.StringProperty(name="Directory")

    def execute(self, context):
        bpy.context.window_manager.progress_begin(0, 100)
        bpy.context.window_manager.progress_update(0)
        zmey_scene = bpy.context.scene.world.zmey_scene_types

        # Export types
        type_list = [(t.name, t.export()) for t in zmey_scene.types]
        for name, data in type_list:
            file = open(self.directory + name + ".type", "w", encoding="utf8", newline="\n")
            file.write(json.dumps(data, indent=4, separators=(', ', ' : ')))
            file.write("\n")
            file.close()

        # Prepare objects
        entities_list = []
        for i, obj in enumerate(bpy.data.objects):
            if obj.zmey_props.enabled:
                type_object = zmey_scene.types[int(obj.zmey_props.type)]
                entity = {
                    "type" : type_object.name,
                    "compiler-id" : 1, #WTF ?
                    "components" : obj.zmey_props.components.export()
                }

                # Export transformation
                transform_component = prepare_object_transform(obj)
                transform_component["name"] = "transform"
                entity["components"].append(transform_component)

                # Export mesh component
                if obj.zmey_props.components.mesh_enabled or \
                    type_object.components.mesh_enabled :
                    # we are exporting all object with gltf so node index is the same as bpy.data.object
                    mesh_component = {
                        "name" : "mesh",
                        "glTF_node_index" : i
                    }
                    entity["components"].append(mesh_component)

                entities_list.append(entity)

        world = {
            "version" : "1.0",
            "name" : zmey_scene.name,
            "extends" : "city-template", # WTF is this ?
            "entities" : entities_list
        }

        file = open(self.filepath, "w", encoding="utf8", newline="\n")
        file.write(json.dumps(world, indent=4, separators=(', ', ' : ')))
        file.write("\n")
        file.close()

        # Export glTF
        bpy.ops.export_scene.gltf(filepath=self.filepath.rsplit(".", 1)[0] + ".gltf")

        bpy.context.window_manager.progress_end()
        return {'FINISHED'}

    def draw(self, contex):
        layout = self.layout
        layout.row().label(text="Zmey")

def menu_func_export_zmey(self, context):
    self.layout.operator(ExportZmey.bl_idname, text="Zmey")

def register():
    components.register()
    properties.register()
    operators.register()
    ui.register()

    bpy.utils.register_class(ExportZmey)

    bpy.types.INFO_MT_file_export.append(menu_func_export_zmey)

def unregister():
    components.unregister()
    properties.unregister()
    operators.unregister()
    ui.unregister()
    bpy.utils.unregister_class(ExportZmey)

    bpy.types.INFO_MT_file_export.remove(menu_func_export_zmey)

if __name__ == "__main__":
    register()