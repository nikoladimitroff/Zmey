# Copyright (c) 2017 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# Imports
#

import base64
import bpy
import copy

from .gltf2_animate import *
from .gltf2_create import *
from .gltf2_debug import *
from .gltf2_extract import *
from .gltf2_filter import *
from .gltf2_get import *

#
# Globals
#

#
# Functions
#

def generate_asset(operator,
                  context,
                  export_settings,
                  glTF):
    """
    Generates the top level asset entry.
    """

    asset = {}
    
    #
    #
    
    asset['version'] = '2.0'
    
    #
    
    asset['generator'] = 'Khronos Blender glTF 2.0 exporter'
    
    #
    
    if export_settings['gltf_copyright'] != "":
        asset['copyright'] = export_settings['gltf_copyright']

    #
    #
        
    glTF['asset'] = asset


def generate_animations_parameter(operator,
                  context,
                  export_settings,
                  glTF,
                  action,
                  channels,
                  samplers,
                  blender_node_name,
                  blender_bone_name,
                  rotation_mode,
                  matrix_correction,
                  matrix_basis,
                  is_morph_data):
    """
    Helper function for storing animation parameters.
    """
    
    name = blender_node_name
    
    prefix = ""
    postfix = ""
    
    node_type = 'NODE'
    used_node_name = blender_node_name 
    if blender_bone_name is not None:
        node_type = 'JOINT'
        used_node_name = blender_bone_name

    #
        
    location = [None, None, None]
    rotation_axis_angle = [None, None, None, None]
    rotation_euler = [None, None, None]
    rotation_quaternion = [None, None, None, None]
    scale = [None, None, None]
    value = []
    
    data = {
        'location' : location,
        'rotation_axis_angle' : rotation_axis_angle,
        'rotation_euler' : rotation_euler,
        'rotation_quaternion' : rotation_quaternion,
        'scale' : scale,
        'value' : value
    }
    
    # Gather fcurves by transform
    for blender_fcurve in action.fcurves:
        node_name = get_node(blender_fcurve.data_path)

        if node_name is not None and not is_morph_data:
            if blender_bone_name is None:
                continue
            elif blender_bone_name != node_name:
                continue
            else:
                prefix = node_name + "_"
                postfix = "_"  + node_name
        
        data_path = get_data_path(blender_fcurve.data_path)
        
        if data_path not in ['location', 'rotation_axis_angle', 'rotation_euler', 'rotation_quaternion', 'scale', 'value']:
            continue

        if data_path != 'value':
            data[data_path][blender_fcurve.array_index] = blender_fcurve
        else:
            data[data_path].append(blender_fcurve)
    
    #

    if location.count(None) < 3:
        
        sampler_name = prefix + action.name + "_translation"
        
        if get_index(samplers, sampler_name) == -1:
            
            sampler = {}
            
            #
            
            interpolation = animate_get_interpolation(export_settings, location)
            if interpolation == 'CUBICSPLINE' and node_type == 'JOINT':
                interpolation = 'CONVERSION_NEEDED'
            
            sampler['interpolation'] = interpolation
            if interpolation == 'CONVERSION_NEEDED':
                sampler['interpolation'] = 'LINEAR'
            
            translation_data, in_tangent_data, out_tangent_data = animate_location(export_settings, location, interpolation, node_type, used_node_name, matrix_correction, matrix_basis)
            
            #
            
            keys = sorted(translation_data.keys())
            values = []
            final_keys = []

            key_offset = 0.0
            if len(keys) > 0 and export_settings['gltf_move_keyframes']:
                key_offset = bpy.context.scene.frame_start / bpy.context.scene.render.fps

            for key in keys:
                if key - key_offset < 0.0:
                    continue 
                
                final_keys.append(key - key_offset)
                
                if interpolation == 'CUBICSPLINE':
                    for i in range(0, 3):
                        values.append(in_tangent_data[key][i])
                for i in range(0, 3):
                    values.append(translation_data[key][i])
                if interpolation == 'CUBICSPLINE':
                    for i in range(0, 3):
                        values.append(out_tangent_data[key][i])
            
            #
            
            componentType = "FLOAT"
            count = len(final_keys)
            type = "SCALAR"
            
            input = create_accessor(operator, context, export_settings, glTF, final_keys, componentType, count, type, "")
            
            sampler['input'] = input
            
            #
    
            componentType = "FLOAT"
            count = len(values) // 3
            type = "VEC3"
            
            output = create_accessor(operator, context, export_settings, glTF, values, componentType, count, type, "")
            
            sampler['output'] = output
            
            #
    
            sampler['name'] = sampler_name
            
            samplers.append(sampler)  

    #
    #
    
    rotation_data = None
    rotation_in_tangent_data = [0.0, 0.0, 0.0, 0.0]
    rotation_out_tangent_data = [0.0, 0.0, 0.0, 0.0]
    interpolation = None
    
    sampler_name = prefix + action.name + "_rotation"

    if get_index(samplers, sampler_name) == -1:
        if rotation_axis_angle.count(None) < 4:
            interpolation = animate_get_interpolation(export_settings, rotation_axis_angle)
            # Conversion required in any case.
            if interpolation == 'CUBICSPLINE':
                interpolation = 'CONVERSION_NEEDED'
            rotation_data = animate_rotation_axis_angle(export_settings, rotation_axis_angle, interpolation, node_type, used_node_name, matrix_correction, matrix_basis)
        
        if rotation_euler.count(None) < 3:
            interpolation = animate_get_interpolation(export_settings, rotation_euler)
            # Conversion required in any case.
            if interpolation == 'CUBICSPLINE':
                interpolation = 'CONVERSION_NEEDED'
            rotation_data = animate_rotation_euler(export_settings, rotation_euler, rotation_mode, interpolation, node_type, used_node_name, matrix_correction, matrix_basis)

        if rotation_quaternion.count(None) < 4:
            interpolation = animate_get_interpolation(export_settings, rotation_quaternion)
            if interpolation == 'CUBICSPLINE' and node_type == 'JOINT':
                interpolation = 'CONVERSION_NEEDED'
            rotation_data, rotation_in_tangent_data, rotation_out_tangent_data = animate_rotation_quaternion(export_settings, rotation_quaternion, interpolation, node_type, used_node_name, matrix_correction, matrix_basis)
        
    if rotation_data is not None:
        
        #

        keys = sorted(rotation_data.keys())
        values = []
        final_keys = []

        key_offset = 0.0
        if len(keys) > 0 and export_settings['gltf_move_keyframes']:
            key_offset = bpy.context.scene.frame_start / bpy.context.scene.render.fps

        for key in keys:
            if key - key_offset < 0.0:
                continue 
            
            final_keys.append(key - key_offset)
            
            if interpolation == 'CUBICSPLINE':
                for i in range(0, 4):
                    values.append(rotation_in_tangent_data[key][i])
            for i in range(0, 4):
                values.append(rotation_data[key][i])
            if interpolation == 'CUBICSPLINE':
                for i in range(0, 4):
                    values.append(rotation_out_tangent_data[key][i])

        #

        sampler = {}

        #
        
        componentType = "FLOAT"
        count = len(final_keys)
        type = "SCALAR"
        
        input = create_accessor(operator, context, export_settings, glTF, final_keys, componentType, count, type, "")
        
        sampler['input'] = input
        
        #

        componentType = "FLOAT"
        count = len(values) // 4
        type = "VEC4"
        
        output = create_accessor(operator, context, export_settings, glTF, values, componentType, count, type, "")
        
        sampler['output'] = output
        
        #
        
        sampler['interpolation'] = interpolation
        if interpolation == 'CONVERSION_NEEDED':
            sampler['interpolation'] = 'LINEAR'
        
        #

        sampler['name'] = sampler_name
        
        samplers.append(sampler) 
    
    #
    #
    
    if scale.count(None) < 3:
        sampler_name = prefix + action.name + "_scale"
    
        if get_index(samplers, sampler_name) == -1:

            sampler = {}
            
            #
            
            interpolation = animate_get_interpolation(export_settings, scale)
            if interpolation == 'CUBICSPLINE' and node_type == 'JOINT':
                interpolation = 'CONVERSION_NEEDED'

            sampler['interpolation'] = interpolation
            if interpolation == 'CONVERSION_NEEDED':
                sampler['interpolation'] = 'LINEAR'
            
            scale_data, in_tangent_data, out_tangent_data = animate_scale(export_settings, scale, interpolation, node_type, used_node_name, matrix_correction, matrix_basis)

            #

            keys = sorted(scale_data.keys())
            values = []
            final_keys = []
    
            key_offset = 0.0
            if len(keys) > 0 and export_settings['gltf_move_keyframes']:
                key_offset = bpy.context.scene.frame_start / bpy.context.scene.render.fps
    
            for key in keys:
                if key - key_offset < 0.0:
                    continue 
                
                final_keys.append(key - key_offset)
                
                if interpolation == 'CUBICSPLINE':
                    for i in range(0, 3):
                        values.append(in_tangent_data[key][i])
                for i in range(0, 3):
                    values.append(scale_data[key][i])
                if interpolation == 'CUBICSPLINE':
                    for i in range(0, 3):
                        values.append(out_tangent_data[key][i])
    
            #
            
            componentType = "FLOAT"
            count = len(final_keys)
            type = "SCALAR"
            
            input = create_accessor(operator, context, export_settings, glTF, final_keys, componentType, count, type, "")
            
            sampler['input'] = input
            
            #

            componentType = "FLOAT"
            count = len(values) // 3
            type = "VEC3"
            
            output = create_accessor(operator, context, export_settings, glTF, values, componentType, count, type, "")
            
            sampler['output'] = output
            
            #

            sampler['name'] = sampler_name
            
            samplers.append(sampler)
            
    #
    #  

    if len(value) > 0 and is_morph_data:
        sampler_name = prefix + action.name + "_weights"
    
        if get_index(samplers, sampler_name) == -1:
            
            sampler = {}
            
            #
            
            interpolation = animate_get_interpolation(export_settings, value)
            if interpolation == 'CUBICSPLINE' and node_type == 'JOINT':
                interpolation = 'CONVERSION_NEEDED'

            sampler['interpolation'] = interpolation
            if interpolation == 'CONVERSION_NEEDED':
                sampler['interpolation'] = 'LINEAR'
            
            value_data, in_tangent_data, out_tangent_data = animate_value(export_settings, value, interpolation, node_type, used_node_name, matrix_correction, matrix_basis)

            #

            keys = sorted(value_data.keys())
            values = []
            final_keys = []
    
            key_offset = 0.0
            if len(keys) > 0 and export_settings['gltf_move_keyframes']:
                key_offset = bpy.context.scene.frame_start / bpy.context.scene.render.fps
    
            for key in keys:
                if key - key_offset < 0.0:
                    continue 
                
                final_keys.append(key - key_offset)
                
                if interpolation == 'CUBICSPLINE':
                    for i in range(0, len(in_tangent_data[key])):
                        values.append(in_tangent_data[key][i])
                for i in range(0, len(value_data[key])):
                    values.append(value_data[key][i])
                if interpolation == 'CUBICSPLINE':
                    for i in range(0, len(out_tangent_data[key])):
                        values.append(out_tangent_data[key][i])
    
            #
            
            componentType = "FLOAT"
            count = len(final_keys)
            type = "SCALAR"
            
            input = create_accessor(operator, context, export_settings, glTF, final_keys, componentType, count, type, "")
            
            sampler['input'] = input
            
            #

            componentType = "FLOAT"
            count = len(values)
            type = "SCALAR"
            
            output = create_accessor(operator, context, export_settings, glTF, values, componentType, count, type, "")
            
            sampler['output'] = output
            
            #

            sampler['name'] = sampler_name
            
            samplers.append(sampler)

    #
    #
    #
    #
    
    write_transform = [False, False, False, False]
    
    # Gather fcurves by transform
    for blender_fcurve in action.fcurves:
        node_name = get_node(blender_fcurve.data_path)
        
        if node_name is not None and not is_morph_data:
            if blender_bone_name is None:
                continue
            elif blender_bone_name != node_name:
                continue
            else:
                prefix = node_name + "_"
                postfix = "_"  + node_name

        data_path = get_data_path(blender_fcurve.data_path)
        if data_path not in ['location', 'rotation_axis_angle', 'rotation_euler', 'rotation_quaternion', 'scale', 'value']:
            continue
        
        if data_path == 'location':
            write_transform[0] = True
        if data_path == 'rotation_axis_angle' or data_path == 'rotation_euler' or data_path == 'rotation_quaternion':
            write_transform[1] = True
        if data_path == 'scale':
            write_transform[2] = True
        if data_path == 'value':
            write_transform[3] = True

    #

    write_transform_index = 0
    for path in ['translation', 'rotation', 'scale', 'weights']:

        if write_transform[write_transform_index]:        
            channel = {}
            
            #
            
            sampler_name = prefix + action.name + "_" + path
            
            channel['sampler'] = get_index(samplers, sampler_name)
            
            #
            #
            
            target = {}
            
            target['path'] = path
            
            target_name = name + postfix 
            
            target['node'] = get_node_index(glTF, target_name)
            
            channel['target'] = target
            
            # 
            
            channels.append(channel)
        
        write_transform_index += 1


#
# Property: animations
#
def generate_animations(operator,
                  context,
                  export_settings,
                  glTF):
    """
    Generates the top level animations, channels and samplers entry.
    """

    animations = []

    channels = []
    
    samplers = []
    
    #
    #
    
    filtered_objects = export_settings['filtered_objects']

    #
    #
    
    blender_backup_action = {}
    
    if export_settings['gltf_bake_skins']:
        
        start = None
        end = None
        
        for current_blender_action in bpy.data.actions:
            for current_blender_fcurve in current_blender_action.fcurves:
                if current_blender_fcurve is None:
                    continue
                
                if start == None:
                    start = current_blender_fcurve.range()[0]
                else:
                    start = min(start, current_blender_fcurve.range()[0])
                    
                if end == None:
                    end = current_blender_fcurve.range()[1]
                else:
                    end = max(end, current_blender_fcurve.range()[1])
        
        if start is None or end is None or export_settings['gltf_frame_range']:
            start = bpy.context.scene.frame_start
            end = bpy.context.scene.frame_end
        
        #
        
        for blender_object in filtered_objects:
            if blender_object.animation_data is not None: 
                blender_backup_action[blender_object.name] = blender_object.animation_data.action
                
            bpy.context.scene.objects.active = blender_object
            
            #
            
            bpy.ops.nla.bake(frame_start=start, frame_end=end, only_selected=False, visual_keying=True, clear_constraints=False, use_current_action=False, bake_types={'POSE'})
    
    #
    #
    
    for blender_object in filtered_objects:
        if blender_object.animation_data is None:
            continue
        
        blender_action = blender_object.animation_data.action

        if blender_action is None:
            continue
        
        #
        #
        
        correction_matrix_local = mathutils.Matrix.Identity(4)
        matrix_basis = mathutils.Matrix.Identity(4)
        
        generate_animations_parameter(operator, context, export_settings, glTF, blender_action, channels, samplers, blender_object.name, None, blender_object.rotation_mode, correction_matrix_local, matrix_basis, False)
        
        if export_settings['gltf_skins']:
            if blender_object.type == 'ARMATURE' and len(blender_object.pose.bones) > 0:
                
                #
                
                axis_basis_change = mathutils.Matrix(((1.0, 0.0, 0.0, 0.0), (0.0, 0.0, 1.0, 0.0), (0.0, -1.0, 0.0, 0.0) , (0.0, 0.0, 0.0, 1.0)))
                
                # Precalculate joint animation data.
                
                start = None
                end = None
                
                for current_blender_action in bpy.data.actions:
                    for current_blender_fcurve in current_blender_action.fcurves:
                        if current_blender_fcurve is None:
                            continue
                        
                        if start == None:
                            start = current_blender_fcurve.range()[0]
                        else:
                            start = min(start, current_blender_fcurve.range()[0])
                            
                        if end == None:
                            end = current_blender_fcurve.range()[1]
                        else:
                            end = max(end, current_blender_fcurve.range()[1])
                
                if start is None or end is None:
                    start = bpy.context.scene.frame_start
                    end = bpy.context.scene.frame_end
                
                #

                for frame in range(int(start), int(end) + 1):
                    bpy.context.scene.frame_set(frame)
                    
                    for blender_bone in blender_object.pose.bones:
                    
                        matrix_basis = blender_bone.matrix_basis
                        
                        #
    
                        correction_matrix_local = mathutils.Matrix.Identity(4)
                    
                        if blender_bone.parent is None:
                            correction_matrix_local = axis_basis_change * blender_bone.bone.matrix_local
                        else:
                            correction_matrix_local = blender_bone.parent.bone.matrix_local.inverted() * blender_bone.bone.matrix_local
                            
                        #
                        if not export_settings['gltf_joint_cache'].get(blender_bone.name):
                            export_settings['gltf_joint_cache'][blender_bone.name] = {}
                        
                        if export_settings['gltf_bake_skins']:
                            matrix_basis = blender_object.convert_space(blender_bone, blender_bone.matrix, from_space='POSE', to_space='LOCAL')
                        
                        matrix = correction_matrix_local * matrix_basis 
            
                        tmp_location, tmp_rotation, tmp_scale = matrix.decompose()
                        
                        export_settings['gltf_joint_cache'][blender_bone.name][float(frame)] = [tmp_location, tmp_rotation, tmp_scale]
                        
                #

                for blender_bone in blender_object.pose.bones:
                    
                    matrix_basis = blender_bone.matrix_basis
                    
                    #

                    correction_matrix_local = mathutils.Matrix.Identity(4)
                
                    if blender_bone.parent is None:
                        correction_matrix_local = axis_basis_change * blender_bone.bone.matrix_local
                    else:
                        correction_matrix_local = blender_bone.parent.bone.matrix_local.inverted() * blender_bone.bone.matrix_local
                    
                    #
                    
                    if export_settings['gltf_bake_skins']:
                        matrix_basis = blender_object.convert_space(blender_bone, blender_bone.matrix, from_space='POSE', to_space='LOCAL')
                    
                    generate_animations_parameter(operator, context, export_settings, glTF, blender_action, channels, samplers, blender_object.name, blender_bone.name, blender_bone.rotation_mode, correction_matrix_local, matrix_basis, False)

    #
    #
    
    processed_meshes = []
        
    for blender_object in filtered_objects:
        
        # Export morph targets animation data.

        if blender_object.type != 'MESH' or blender_object.data is None:
            continue
        
        blender_mesh = blender_object.data
        
        if blender_mesh in processed_meshes:
            continue

        if blender_mesh.shape_keys is None or blender_mesh.shape_keys.animation_data is None:
            continue
        
        blender_action = blender_mesh.shape_keys.animation_data.action

        if blender_action is None:
            continue
        
        #
        #
        
        correction_matrix_local = mathutils.Matrix.Identity(4)
        matrix_basis = mathutils.Matrix.Identity(4)
        
        generate_animations_parameter(operator, context, export_settings, glTF, blender_action, channels, samplers, blender_object.name, None, blender_object.rotation_mode, correction_matrix_local, matrix_basis, True)
        
        processed_meshes.append(blender_mesh)
    
    #
    #
    
    if export_settings['gltf_bake_skins']:
        for blender_object in filtered_objects:
            if blender_backup_action.get(blender_object.name) is not None:
                blender_object.animation_data.action = blender_backup_action[blender_object.name]
    
    #
    #

    if len(channels) > 0 or len(samplers) > 0:
        
        # Sampler 'name' is used to gather the index. However, 'name' is no property of sampler and has to be removed.
        for sampler in samplers:
           del sampler['name']
        
        #
        
        animation = {
            'channels' : channels,
            'samplers' : samplers 
        }
        
        animations.append(animation)
    
    #
    #

    if len (animations) > 0:
        glTF['animations'] = animations


def generate_cameras(operator,
                  context,
                  export_settings,
                  glTF):
    """
    Generates the top level cameras entry.
    """

    cameras = []

    #
    #
    
    filtered_cameras = export_settings['filtered_cameras']
    
    for blender_camera in filtered_cameras:

        #
        # Property: camera
        #

        camera = {}

        if blender_camera.type == 'PERSP':
            camera['type'] = 'perspective'
            
            perspective = {}
            
            #
            
            # None of them can get 0, as Blender checks this.
            width = bpy.context.scene.render.pixel_aspect_x * bpy.context.scene.render.resolution_x
            height = bpy.context.scene.render.pixel_aspect_y * bpy.context.scene.render.resolution_y
            
            aspectRatio = width / height
            
            perspective['aspectRatio'] = aspectRatio

            yfov = None
            
            if width >= height:
                if blender_camera.sensor_fit != 'VERTICAL':                
                    yfov = 2.0 * math.atan(math.tan(blender_camera.angle * 0.5) / aspectRatio);
                else:
                    yfov = blender_camera.angle                
            else:
                if blender_camera.sensor_fit != 'HORIZONTAL':                
                    yfov = blender_camera.angle                
                else:
                    yfov = 2.0 * math.atan(math.tan(blender_camera.angle * 0.5) / aspectRatio);

            perspective['yfov'] = yfov

            perspective['znear'] = blender_camera.clip_start
            
            if not export_settings['gltf_camera_infinite']:
                perspective['zfar'] = blender_camera.clip_end
            
            #
            
            camera['perspective'] = perspective
        elif blender_camera.type == 'ORTHO':
            camera['type'] = 'orthographic'

            orthographic = {}
            
            #    

            orthographic['xmag'] = blender_camera.ortho_scale
            orthographic['ymag'] = blender_camera.ortho_scale

            orthographic['znear'] = blender_camera.clip_start
            orthographic['zfar'] = blender_camera.clip_end
            
            # 
            
            camera['orthographic'] = orthographic
        else:
            continue
    
        #
        
        camera['name'] = blender_camera.name
        
        #
        #
        
        cameras.append(camera)

    #
    #

    if len (cameras) > 0:
        glTF['cameras'] = cameras


def generate_lights_cmn(operator,
                    context,
                    export_settings,
                    glTF):
    """
    Generates the top level lights entry.
    Note: This is currently an experimental feature.
    """

    lights = []

    #
    #
    
    filtered_lights_cmn = export_settings['filtered_lights_cmn']
    
    for blender_light in filtered_lights_cmn:

        #
        # Property: light
        #

        light = {'profile' : 'CMN'}
        
        if blender_light.type == 'SUN':
            light['type'] = 'directional' 
        elif blender_light.type == 'POINT':
            light['type'] = 'point' 
        elif blender_light.type == 'SPOT':
            light['type'] = 'spot' 
        else:
            continue

        if blender_light.type == 'POINT' or blender_light.type == 'SPOT':
            
            positional = {}
            
            if blender_light.falloff_type == 'CONSTANT':
                positional['constantAttenuation'] = 1.0
            elif blender_light.falloff_type == 'INVERSE_LINEAR':
                positional['linearAttenuation'] = 1.0 / blender_light.distance
            elif blender_light.falloff_type == 'INVERSE_SQUARE':
                positional['quadraticAttenuation'] = 1.0 / blender_light.distance
            elif blender_light.falloff_type == 'INVERSE_COEFFICIENTS':
                positional['constantAttenuation'] = blender_light.constant_coefficient * 1.0
                positional['linearAttenuation'] = blender_light.linear_coefficient * 1.0 / blender_light.distance
                positional['quadraticAttenuation'] = blender_light.quadratic_coefficient *  1.0 / blender_light.distance
            else:
                continue
            
            light['positional'] = positional 
            
            if blender_light.type == 'SPOT':
                spot = {}
                
                angle = blender_light.spot_size * 0.5
                
                spot['outerAngle'] = angle
                spot['innerAngle'] = angle - angle * blender_light.spot_blend
                
                positional['spot'] = spot

        light['color'] = [blender_light.color[0] * blender_light.energy, blender_light.color[1] * blender_light.energy, blender_light.color[2] * blender_light.energy]
        
        #
        
        light['name'] = blender_light.name
        
        #
        #
        
        lights.append(light)
        
    #
    #
    #
    
    for blender_scene in bpy.data.scenes:

        #
        # Property: light
        #

        light = {}
        
        light['type'] = 'ambient' 

        light['color'] = [blender_scene.world.ambient_color[0], blender_scene.world.ambient_color[1], blender_scene.world.ambient_color[2]]
        
        #
        
        light['name'] = 'Ambient_' + blender_scene.name
        
        #
        #
        
        lights.append(light)
    
    #
    #

    if len (lights) > 0:
        create_extensionsUsed(operator, context, export_settings, glTF, 'KHR_lights_cmn')
        create_extensionsRequired(operator, context, export_settings, glTF, 'KHR_lights_cmn')

        if glTF.get('extensions') is None:
            glTF['extensions'] = {}
            
        extensions = glTF['extensions']
        
        extensions_lights_cmn = {}

        extensions['KHR_lights_cmn'] = extensions_lights_cmn
        
        extensions_lights_cmn['lights'] = lights


def generate_lights_pbr(operator,
                    context,
                    export_settings,
                    glTF):
    """
    Generates the top level lights entry.
    Note: This is currently an experimental feature.
    """

    lights = []

    #
    #
    
    filtered_lights_pbr = export_settings['filtered_lights_pbr']
    
    for blender_light in filtered_lights_pbr:

        #
        # Property: light
        #

        light = {'profile' : 'PBR'}
        
        #
        
        blender_light_node = None
        
        for blender_node in blender_light.node_tree.nodes:
            if isinstance(blender_node, bpy.types.ShaderNodeGroup):
                if blender_node.node_tree.name.startswith('glTF Directional Light'):
                    light['type'] = 'directional'
                    blender_light_node = blender_node 
                    break
                if blender_node.node_tree.name.startswith('glTF Point Light'):
                    light['type'] = 'point'
                    blender_light_node = blender_node
                    break
                if blender_node.node_tree.name.startswith('glTF Spot Light'):
                    light['type'] = 'spot'
                    blender_light_node = blender_node
                    break
            
        if blender_light_node is None:
            continue
        
        #
        
        if light['type'] == 'point' or light['type'] == 'spot':
            
            positional = {}

            positional['radius'] = blender_light_node.inputs['Radius'].default_value
            
            light['positional'] = positional 
            
            if light['type'] == 'spot':
                spot = {}
                
                angle = blender_light.spot_size * 0.5
                
                spot['outerAngle'] = angle
                spot['innerAngle'] = angle - angle * blender_light.spot_blend

                positional['spot'] = spot

        light['color'] = [blender_light_node.inputs['Color'].default_value[0], blender_light_node.inputs['Color'].default_value[1], blender_light_node.inputs['Color'].default_value[2]]
        light['strength'] = blender_light_node.inputs['Strength'].default_value
        
        #
        
        light['name'] = blender_light.name
        
        #
        #
        
        lights.append(light)
        
    #
    #

    if len (lights) > 0:
        create_extensionsUsed(operator, context, export_settings, glTF, 'KHR_lights_pbr')
        create_extensionsRequired(operator, context, export_settings, glTF, 'KHR_lights_pbr')

        if glTF.get('extensions') is None:
            glTF['extensions'] = {}
            
        extensions = glTF['extensions']
        
        extensions_lights_pbr = {}

        extensions['KHR_lights_pbr'] = extensions_lights_pbr
        
        extensions_lights_pbr['lights'] = lights


def generate_meshes(operator,
                  context,
                  export_settings,
                  glTF):
    """
    Generates the top level meshes entry.
    """

    meshes = []

    #
    #
    
    filtered_meshes = export_settings['filtered_meshes']
    
    filtered_vertex_groups = export_settings['filtered_vertex_groups']

    for name, blender_mesh in filtered_meshes.items():
        
        internal_primitives = extract_primitives(glTF, blender_mesh, filtered_vertex_groups[name], export_settings)
        
        if len(internal_primitives) == 0:
            continue
        
        #
        # Property: mesh
        #
        
        mesh = {}
        
        #
        
        primitives = []
        
        for internal_primitive in internal_primitives:
            
            primitive = {}
            
            #
            #
            
            if export_settings['gltf_materials']:
                material = get_material_index(glTF, internal_primitive['material'])

                if get_material_requires_texcoords(glTF, material) and not export_settings['gltf_texcoords']:
                    material = -1

                if get_material_requires_normals(glTF, material) and not export_settings['gltf_normals']:
                    material = -1

                # Meshes/primitives without material are allowed.
                if material >= 0:
                    primitive['material'] = material
                else:
                    print_console('WARNING', 'Material ' + internal_primitive['material'] + ' not found. Please assign glTF 2.0 material or enable Blinn-Phong material in export.')
                
            #
            #
            
            indices = internal_primitive['indices']

            componentType = "UNSIGNED_BYTE"
            
            max_index = max(indices)
            
            if max_index < 256:
                componentType = "UNSIGNED_BYTE"
            elif max_index < 65536:
                componentType = "UNSIGNED_SHORT"
            elif max_index < 4294967296:
                componentType = "UNSIGNED_INT"
            else:
                print_console('ERROR', 'Invalid max_index: ' + str(max_index))
                continue
            
            if export_settings['gltf_force_indices']:
                componentType = export_settings['gltf_indices']

            count = len(indices)
            
            type = "SCALAR"
            
            indices_index = create_accessor(operator, context, export_settings, glTF, indices, componentType, count, type, "ELEMENT_ARRAY_BUFFER")
            
            if indices_index < 0:
                print_console('ERROR', 'Could not create accessor for indices')
                continue
            
            primitive['indices'] = indices_index
            
            #
            #
            
            attributes = {}
            
            #
            
            internal_attributes = internal_primitive['attributes']
            
            #
            #
            
            internal_position = internal_attributes['POSITION']

            componentType = "FLOAT"

            count = len(internal_position) // 3
            
            type = "VEC3"
            
            position = create_accessor(operator, context, export_settings, glTF, internal_position, componentType, count, type, "ARRAY_BUFFER")
            
            if position < 0:
                print_console('ERROR', 'Could not create accessor for position')
                continue
            
            attributes['POSITION'] = position
            
            #
            
            if export_settings['gltf_normals']:            
                internal_normal = internal_attributes['NORMAL']
    
                componentType = "FLOAT"
    
                count = len(internal_normal) // 3
                
                type = "VEC3"
                
                normal = create_accessor(operator, context, export_settings, glTF, internal_normal, componentType, count, type, "ARRAY_BUFFER")
                
                if normal < 0:
                    print_console('ERROR', 'Could not create accessor for normal')
                    continue
                
                attributes['NORMAL'] = normal
            
            #

            if export_settings['gltf_tangents']:            
                if internal_attributes.get('TANGENT') is not None:
                    internal_tangent = internal_attributes['TANGENT']
    
                    componentType = "FLOAT"
        
                    count = len(internal_tangent) // 4
                    
                    type = "VEC4"
                    
                    tangent = create_accessor(operator, context, export_settings, glTF, internal_tangent, componentType, count, type, "ARRAY_BUFFER")
                    
                    if tangent < 0:
                        print_console('ERROR', 'Could not create accessor for tangent')
                        continue
                    
                    attributes['TANGENT'] = tangent
            
            #
            
            if export_settings['gltf_texcoords']:
                texcoord_index = 0
                
                process_texcoord = True
                while process_texcoord:  
                    texcoord_id = 'TEXCOORD_' + str(texcoord_index)
                    
                    if internal_attributes.get(texcoord_id) is not None:
                        internal_texcoord = internal_attributes[texcoord_id]
            
                        componentType = "FLOAT"
            
                        count = len(internal_texcoord) // 2
                        
                        type = "VEC2"
                        
                        texcoord = create_accessor(operator, context, export_settings, glTF, internal_texcoord, componentType, count, type, "ARRAY_BUFFER")
                        
                        if texcoord < 0:
                            process_texcoord = False
                            print_console('ERROR', 'Could not create accessor for ' + texcoord_id)
                            continue
                        
                        attributes[texcoord_id] = texcoord
                        
                        texcoord_index += 1
                    else:
                        process_texcoord = False
                        
            #

            if export_settings['gltf_colors']:
                color_index = 0
                
                process_color = True
                while process_color:  
                    color_id = 'COLOR_' + str(color_index)
                    
                    if internal_attributes.get(color_id) is not None:
                        internal_color = internal_attributes[color_id]
            
                        componentType = "FLOAT"
            
                        count = len(internal_color) // 4
                        
                        type = "VEC4"
                        
                        color = create_accessor(operator, context, export_settings, glTF, internal_color, componentType, count, type, "ARRAY_BUFFER")
                        
                        if color < 0:
                            process_color = False
                            print_console('ERROR', 'Could not create accessor for ' + color_id)
                            continue
                        
                        attributes[color_id] = color
                        
                        color_index += 1
                    else:
                        process_color = False

            #
            
            if export_settings['gltf_skins']:
                bone_index = 0
                
                process_bone = True
                while process_bone:  
                    joint_id = 'JOINTS_' + str(bone_index)
                    weight_id = 'WEIGHTS_' + str(bone_index)
                    
                    if internal_attributes.get(joint_id) is not None and internal_attributes.get(weight_id) is not None:
                        internal_joint = internal_attributes[joint_id]
            
                        componentType = "UNSIGNED_SHORT"
            
                        count = len(internal_joint) // 4
                        
                        type = "VEC4"
                        
                        joint = create_accessor(operator, context, export_settings, glTF, internal_joint, componentType, count, type, "ARRAY_BUFFER")
                        
                        if joint < 0:
                            process_bone = False
                            print_console('ERROR', 'Could not create accessor for ' + joint_id)
                            continue
                        
                        attributes[joint_id] = joint
                        
                        #
                        #
    
                        internal_weight = internal_attributes[weight_id]
            
                        componentType = "FLOAT"
            
                        count = len(internal_weight) // 4
                        
                        type = "VEC4"
                        
                        weight = create_accessor(operator, context, export_settings, glTF, internal_weight, componentType, count, type, "ARRAY_BUFFER")
                        
                        if weight < 0:
                            process_bone = False
                            print_console('ERROR', 'Could not create accessor for ' + weight_id)
                            continue
                        
                        attributes[weight_id] = weight
                        
                        #
                        #
                        
                        bone_index += 1
                    else:
                        process_bone = False
            
            #
            
            if export_settings['gltf_morph']:
                if blender_mesh.shape_keys is not None:
                    targets = []

                    morph_index = 0
                    for blender_shape_key in blender_mesh.shape_keys.key_blocks:
                        if blender_shape_key != blender_shape_key.relative_key:
                    
                            target_position_id = 'MORPH_POSITION_' + str(morph_index)
                            target_normal_id = 'MORPH_NORMAL_' + str(morph_index)
                            target_tangent_id = 'MORPH_TANGENT_' + str(morph_index)
                            
                            if internal_attributes.get(target_position_id) is not None:
                                internal_target_position = internal_attributes[target_position_id]
                    
                                componentType = "FLOAT"
                    
                                count = len(internal_target_position) // 3
                                
                                type = "VEC3"
                                
                                target_position = create_accessor(operator, context, export_settings, glTF, internal_target_position, componentType, count, type, "")
                                
                                if target_position < 0:
                                    print_console('ERROR', 'Could not create accessor for ' + target_position_id)
                                    continue
                                
                                #
                                
                                target = {
                                    'POSITION' : target_position
                                }
                                
                                #
                                
                                if export_settings['gltf_normals'] and export_settings['gltf_morph_normal'] and internal_attributes.get(target_normal_id) is not None: 
    
                                    internal_target_normal = internal_attributes[target_normal_id]
                        
                                    componentType = "FLOAT"
                        
                                    count = len(internal_target_normal) // 3
                                    
                                    type = "VEC3"
                                    
                                    target_normal = create_accessor(operator, context, export_settings, glTF, internal_target_normal, componentType, count, type, "")
                                    
                                    if target_normal < 0:
                                        print_console('ERROR', 'Could not create accessor for ' + target_normal_id)
                                        continue
                                    
                                    target['NORMAL'] = target_normal
                                #
                                
                                if export_settings['gltf_tangents'] and export_settings['gltf_morph_tangent'] and internal_attributes.get(target_tangent_id) is not None: 

                                    internal_target_tangent = internal_attributes[target_tangent_id]
                        
                                    componentType = "FLOAT"
                        
                                    count = len(internal_target_tangent) // 3
                                    
                                    type = "VEC3"
                                    
                                    target_tangent = create_accessor(operator, context, export_settings, glTF, internal_target_tangent, componentType, count, type, "")
                                    
                                    if target_tangent < 0:
                                        print_console('ERROR', 'Could not create accessor for ' + target_tangent_id)
                                        continue
                                    
                                    target['TANGENT'] = target_tangent
                                
                                #
                                #
                                
                                targets.append(target)
                                
                                morph_index += 1
        
                    if len(targets) > 0:
                        primitive['targets'] = targets

            #
            #
            
            primitive['attributes'] = attributes
            
            #
            #
            
            primitives.append(primitive)
        
        #
            
        if export_settings['gltf_morph']:
            if blender_mesh.shape_keys is not None:
                morph_max = len(blender_mesh.shape_keys.key_blocks) - 1
                if morph_max > 0:
                    weights = []
                    
                    for blender_shape_key in blender_mesh.shape_keys.key_blocks:
                        if blender_shape_key != blender_shape_key.relative_key:
                            weights.append(blender_shape_key.value)
                    
                    mesh['weights'] = weights

                    
        #
        
        if export_settings['gltf_extras']:
            extras = create_custom_property(blender_mesh)
            
            if extras is not None:
                mesh['extras'] = extras 

        #

        mesh['primitives'] = primitives

        #

        mesh['name'] = name

        #
        #

        meshes.append(mesh)

    #
    #

    if len (meshes) > 0:
        glTF['meshes'] = meshes


def generate_dublicate_mesh(operator,
                  context,
                  export_settings,
                  glTF, blender_object):
    """
    Helper function for dublicating meshes with linked object materials.
    """
    
    if blender_object is None:
        return -1
    
    mesh_index = get_mesh_index(glTF, blender_object.data.name)
    
    if mesh_index == -1:
        return False
    
    new_mesh = copy.deepcopy(glTF['meshes'][mesh_index])
    
    #

    primitives = new_mesh['primitives']
    
    primitive_index = 0
    for blender_material_slot in blender_object.material_slots:
        if blender_material_slot.link == 'OBJECT':
            primitives[primitive_index]['material'] = get_material_index(glTF, blender_material_slot.material.name)
            
        primitive_index += 1

    #
    
    new_name = blender_object.data.name + '_' + blender_object.name
    
    new_mesh['name'] = new_name
    
    glTF['meshes'].append(new_mesh)
    
    return get_mesh_index(glTF, new_name)


def generate_node_parameter(operator,
                  context,
                  export_settings,
                  glTF,
                  matrix,
                  node,
                  node_type):
    """
    Helper function for storing node parameters.
    """
        
    translation, rotation, scale = decompose_transition(matrix, node_type, export_settings)
    
    #
    
    if translation[0] != 0.0 or translation[1] != 0.0 or translation[2] != 0.0:
        node['translation'] = [translation[0], translation[1], translation[2]]
        
    #

    if rotation[0] != 0.0 or rotation[1] != 0.0 or rotation[2] != 0.0 or rotation[3] != 1.0:
        node['rotation'] = [rotation[0], rotation[1], rotation[2], rotation[3]]
        
    #

    if scale[0] != 1.0 or scale[1] != 1.0 or scale[2] != 1.0:
        node['scale'] = [scale[0], scale[1], scale[2]]


def generate_node_instance(operator,
                  context,
                  export_settings,
                  glTF,
                  nodes, 
                  blender_object,
                  force_visible):
    """
    Helper function for storing node instances.
    """

    correction_quaternion = convert_swizzle_rotation(mathutils.Quaternion((1.0, 0.0, 0.0), math.radians(-90.0)))
    
    #
    # Property: node
    #

    node = {}
    
    #
    #
    
    generate_node_parameter(operator, context, export_settings, glTF, blender_object.matrix_local, node, 'NODE')
    
    #
    #
    
    if export_settings['gltf_layers'] or blender_object.layers[0] or force_visible:
        
        #
        #
        
        if blender_object.type == 'MESH' or blender_object.type == 'CURVE':
                mesh = get_mesh_index(glTF, blender_object.data.name)
                
                if mesh >= 0:
                    
                    need_dublicate = False
                    
                    if blender_object.material_slots:
                        for blender_material_slot in blender_object.material_slots:
                            if blender_material_slot.link == 'OBJECT':
                                need_dublicate = True
                                break

                    if need_dublicate:
                        mesh = generate_dublicate_mesh(operator, context, export_settings, glTF, blender_object)
                    
                    #
                    
                    if mesh >= 0:
                        node['mesh'] = mesh
        
        #
        #
        
        if export_settings['gltf_cameras']:
            if blender_object.type == 'CAMERA':
                camera = get_camera_index(glTF, blender_object.data.name)
                
                if camera >= 0:
                    # Add correction node for camera, as default direction is different to Blender.
                    correction_node = {}
                    
                    correction_node['name'] = 'Correction_' + blender_object.name
                    correction_node['rotation'] = [correction_quaternion[1], correction_quaternion[2], correction_quaternion[3], correction_quaternion[0]]
                    
                    correction_node['camera'] = camera
                    
                    nodes.append(correction_node)
    
    
        if export_settings['gltf_lights_cmn']:
            if blender_object.type == 'LAMP':
                light = get_light_index_cmn(glTF, blender_object.data.name)
                if light >= 0:
                    khr_lights_cmn = {'light' : light}
                    extensions = {'KHR_lights_cmn' : khr_lights_cmn}
                    
                    # Add correction node for light, as default direction is different to Blender.
                    correction_node = {}
                    
                    correction_node['name'] = 'Correction_' + blender_object.name
                    correction_node['rotation'] = [correction_quaternion[1], correction_quaternion[2], correction_quaternion[3], correction_quaternion[0]]
                    
                    correction_node['extensions'] = extensions
                    
                    nodes.append(correction_node)

        if export_settings['gltf_lights_pbr']:
            if blender_object.type == 'LAMP':
                light = get_light_index_pbr(glTF, blender_object.data.name)
                if light >= 0:
                    khr_lights_pbr = {'light' : light}
                    extensions = {'KHR_lights_pbr' : khr_lights_pbr}
                    
                    # Add correction node for light, as default direction is different to Blender.
                    correction_node = {}
                    
                    correction_node['name'] = 'Correction_' + blender_object.name
                    correction_node['rotation'] = [correction_quaternion[1], correction_quaternion[2], correction_quaternion[3], correction_quaternion[0]]
                    
                    correction_node['extensions'] = extensions
                    
                    nodes.append(correction_node)
                
    #
    
    if export_settings['gltf_extras']:
        extras = create_custom_property(blender_object)
        
        if extras is not None:
            node['extras'] = extras 

    #

    node['name'] = blender_object.name
    
    #
    
    return node


def generate_nodes(operator,
                  context,
                  export_settings,
                  glTF):
    """
    Generates the top level nodes entry.
    """
    
    nodes = []
    
    skins = []

    #
    #
    
    filtered_objects = export_settings['filtered_objects']

    for blender_object in filtered_objects:

        node = generate_node_instance(operator, context, export_settings, glTF, nodes, blender_object, False)

        #
        #

        nodes.append(node)

    #
    #
    
    for blender_object in filtered_objects:
        if blender_object.dupli_type == 'GROUP' and blender_object.dupli_group != None:
            
            if export_settings['gltf_layers'] or (blender_object.layers[0] and blender_object.dupli_group.layers[0]):
                
                for blender_dupli_object in blender_object.dupli_group.objects:
    
                    node = generate_node_instance(operator, context, export_settings, glTF, nodes, blender_dupli_object, True)
    
                    node['name'] = 'Duplication_' + blender_object.name + '_' + blender_dupli_object.name 
            
                    #
                    #
            
                    nodes.append(node)
                
                #
                
                node = {}
                
                node['name'] = 'Duplication_Offset_' + blender_object.name
                
                translation = convert_swizzle_location(blender_object.dupli_group.dupli_offset)
                
                node['translation'] = [-translation[0], -translation[1], -translation[2]]
                
                nodes.append(node)
            
            

    #
    #

    if len (nodes) > 0:
        glTF['nodes'] = nodes

    #
    #
    
    if export_settings['gltf_skins']:
        for blender_object in filtered_objects:
            if blender_object.type != 'ARMATURE' or len(blender_object.pose.bones) == 0:
                continue
    
            temp_action = None

            if export_settings['gltf_bake_skins'] and not export_settings['gltf_animations']:
                if blender_object.animation_data is not None: 
                    temp_action = blender_object.animation_data.action
                
                bpy.context.scene.objects.active = blender_object
                bpy.ops.object.mode_set(mode='POSE')
                bpy.ops.nla.bake(frame_start=bpy.context.scene.frame_current, frame_end=bpy.context.scene.frame_current, only_selected=False, visual_keying=True, clear_constraints=False, use_current_action=False, bake_types={'POSE'})

            joints = []
            
            joints_written = False
            
            #
            
            children_list = list(blender_object.children)
            
            for blender_check_object in filtered_objects:
                blender_check_armature = blender_check_object.find_armature()
                
                if blender_check_armature is not None and blender_check_object not in children_list:
                    children_list.append(blender_check_object)
            
            #
    
            for blender_object_child in children_list:
                #
                # Property: skin and node
                #
                
                inverse_matrices = []
                
                for blender_bone in blender_object.pose.bones:
        
                    axis_basis_change = mathutils.Matrix(((1.0, 0.0, 0.0, 0.0), (0.0, 0.0, 1.0, 0.0), (0.0, -1.0, 0.0, 0.0) , (0.0, 0.0, 0.0, 1.0))) 

                    if not joints_written:                    
                        node = {}
                    
                        correction_matrix_local = mathutils.Matrix.Identity(4)
                        
                        if blender_bone.parent is None:
                            correction_matrix_local = axis_basis_change * blender_bone.bone.matrix_local
                        else:
                            correction_matrix_local = blender_bone.parent.bone.matrix_local.inverted() * blender_bone.bone.matrix_local
                        
                        matrix_basis = blender_bone.matrix_basis
                        
                        if export_settings['gltf_bake_skins']:
                            matrix_basis = blender_object.convert_space(blender_bone, blender_bone.matrix, from_space='POSE', to_space='LOCAL')
                        
                        generate_node_parameter(operator, context, export_settings, glTF, correction_matrix_local * matrix_basis, node, 'JOINT')
                
                        #
                
                        node['name'] = blender_object.name + "_" + blender_bone.name
                
                        #
                        #
            
                        joints.append(len(nodes))
                        
                        nodes.append(node)
                    
                    #
                    #
                    
                    inverse_bind_matrix = axis_basis_change * blender_bone.bone.matrix_local

                    bind_shape_matrix = axis_basis_change * blender_object.matrix_world.inverted() * blender_object_child.matrix_world * axis_basis_change.inverted() 
                    
                    inverse_bind_matrix = inverse_bind_matrix.inverted() * bind_shape_matrix
                    
                    for column in range(0, 4):
                        for row in range(0, 4):
                            inverse_matrices.append(inverse_bind_matrix[row][column])
                    
                #
            
                joints_written = True                    

                #
                    
                skin = {}
                
                skin['skeleton'] = get_node_index(glTF, blender_object.name)

                skin['joints'] = joints
                
                #
                 
                componentType = "FLOAT"
                count = len(inverse_matrices) // 16
                type = "MAT4"
                
                inverseBindMatrices = create_accessor(operator, context, export_settings, glTF, inverse_matrices, componentType, count, type, "")
                 
                skin['inverseBindMatrices'] = inverseBindMatrices
                
                #
                
                skins.append(skin)
            
            #
            
            if temp_action is not None:
                blender_object.animation_data.action = temp_action



    #
    #

    if len (skins) > 0:
        glTF['skins'] = skins

    #
    # Resolve children etc.
    #

    for blender_object in filtered_objects:
        node_index = get_node_index(glTF, blender_object.name)
        
        node = nodes[node_index]
        
        #
        
        if export_settings['gltf_skins']:
            blender_armature = blender_object.find_armature()
            if blender_armature is not None:
                index_offset = 0
                
                if blender_object in blender_armature.children:
                    index_offset = blender_armature.children.index(blender_object)
                else:
                    index_local_offset = 0
                    
                    for blender_check_object in filtered_objects:
                        blender_check_armature = blender_check_object.find_armature()
                        if blender_check_armature == blender_armature:
                            index_local_offset += 1
                            
                        if blender_object == blender_check_object:
                            index_local_offset -= 1
                            break
                    
                    index_offset = len(blender_armature.children) + index_local_offset
                
                node['skin'] = get_skin_index(glTF, blender_armature.name, index_offset)

        #

        children = []
        
        # Camera
        if export_settings['gltf_cameras']:
            if blender_object.type == 'CAMERA':
                child_index = get_node_index(glTF, 'Correction_' + blender_object.name)
                if child_index >= 0:
                    children.append(child_index)

        # Light CMN
        if export_settings['gltf_lights_cmn']:
            if blender_object.type == 'LAMP':
                child_index = get_node_index(glTF, 'Correction_' + blender_object.name)
                if child_index >= 0:
                    children.append(child_index)

        # Light PBR
        if export_settings['gltf_lights_pbr']:
            if blender_object.type == 'LAMP':
                child_index = get_node_index(glTF, 'Correction_' + blender_object.name)
                if child_index >= 0:
                    children.append(child_index)

        # Nodes
        for blender_child_node in blender_object.children:
            child_index = get_node_index(glTF, blender_child_node.name)
            
            if blender_child_node.parent_type == 'BONE' and export_settings['gltf_skins']:
                continue
            
            if child_index < 0:
                continue
            
            children.append(child_index)
            
        # Duplications
        if blender_object.dupli_type == 'GROUP' and blender_object.dupli_group != None:

            child_index = get_node_index(glTF, 'Duplication_Offset_' + blender_object.name)
            if child_index >= 0:
                children.append(child_index)
                
                duplication_node = nodes[child_index]
                
                duplication_children = []
                
                for blender_dupli_object in blender_object.dupli_group.objects:
                    child_index = get_node_index(glTF, 'Duplication_' + blender_object.name + '_' + blender_dupli_object.name)
                    if child_index >= 0:
                        duplication_children.append(child_index)
                
                duplication_node['children'] = duplication_children 
        
        #
        
        if export_settings['gltf_skins']:
            # Joint
            if blender_object.type == 'ARMATURE' and len(blender_object.pose.bones) > 0:
                
                #

                blender_object_to_bone = {}

                if export_settings['gltf_skins']:
                    for blender_child_node in blender_object.children:
                        if blender_child_node.parent_type == 'BONE':
                            blender_object_to_bone[blender_child_node.name] = blender_child_node.parent_bone
                
                # 
                
                for blender_bone in blender_object.pose.bones:
                    
                    if blender_bone.parent:
                        continue
                    
                    child_index = get_node_index(glTF, blender_object.name + "_" + blender_bone.name)
            
                    if child_index < 0:
                        continue
                
                    children.append(child_index)
                
                for blender_bone in blender_object.pose.bones:
                    joint_children = []
                    for blender_bone_child in blender_bone.children:
                        child_index = get_node_index(glTF, blender_object.name + "_" + blender_bone_child.name) 
                    
                        if child_index < 0:
                            continue
                    
                        joint_children.append(child_index)
                        
                    for blender_object_name in blender_object_to_bone:
                        blender_bone_name = blender_object_to_bone[blender_object_name]
                        if blender_bone_name == blender_bone.name:
                            child_index = get_node_index(glTF, blender_object_name) 
                        
                            if child_index < 0:
                                continue
                        
                            joint_children.append(child_index)
                
                    if len(joint_children) > 0:
                        node_index = get_node_index(glTF, blender_object.name + "_" + blender_bone.name)
                        
                        child_node = nodes[node_index]
                        
                        child_node['children'] = joint_children

        if len(children) > 0:
            node['children'] = children


def generate_images(operator,
                  context,
                  export_settings,
                  glTF):
    """
    Generates the top level images entry.
    """

    filtered_images = export_settings['filtered_images']
                  
    images = []

    #
    #

    for blender_image in filtered_images:
        #
        # Property: image
        #

        image = {}

        #

        if export_settings['gltf_format'] == 'ASCII':

            if export_settings['gltf_embed_images']:
                # Embed image as Base64.
                
                png_data = create_png_data(blender_image)

                # Required

                image['uri'] = 'data:image/png;base64,' + base64.b64encode(png_data).decode('ascii')

            else:
                # Store image external.

                uri = get_uri(blender_image.filepath)

                context.scene.render.image_settings.file_format = 'PNG'
                context.scene.render.image_settings.color_depth = '8'        
                
                blender_image.save_render(export_settings['gltf_filedirectory'] + uri, context.scene)

                # Required

                image['uri'] = uri

        else:            
            # Store image as glb.
            
            png_data = create_png_data(blender_image)
            
            bufferView = create_bufferView(operator, context, export_settings, glTF, png_data, 0, 0)

            # Required

            image['mimeType'] = 'image/png'
            
            image['bufferView'] = bufferView

        #
        #
        
        export_settings['gltf_uri'].append(get_uri(blender_image.filepath)) 

        images.append(image)

    #
    #

    if len (images) > 0:
        glTF['images'] = images


def generate_textures(operator,
                       context,
                       export_settings,
                       glTF):
    """
    Generates the top level textures entry.
    """

    filtered_textures = export_settings['filtered_textures']
                  
    textures = []

    #
    #

    for blender_texture in filtered_textures:
        #
        # Property: texture
        #

        texture = {}
        
        #

        if isinstance(blender_texture, bpy.types.ShaderNodeTexImage):
            magFilter = 9729
            if blender_texture.interpolation == 'Closest':
                magFilter = 9728
            wrap = 10497
            if blender_texture.extension == 'CLIP':
                wrap = 33071

            texture['sampler'] = create_sampler(operator, context, export_settings, glTF, magFilter, wrap)

            texture['source'] = get_image_index(export_settings, get_uri(blender_texture.image.filepath))
            
            #
            #
    
            textures.append(texture)
            
        else:
            magFilter = 9729
            wrap = 10497
            if blender_texture.texture.extension == 'CLIP':
                wrap = 33071

            texture['sampler'] = create_sampler(operator, context, export_settings, glTF, magFilter, wrap)

            texture['source'] = get_image_index(export_settings, get_uri(blender_texture.texture.image.filepath))

            #
            #
    
            textures.append(texture)

    #
    #

    if len (textures) > 0:
        glTF['textures'] = textures


def generate_materials(operator,
                       context,
                       export_settings,
                       glTF):
    """
    Generates the top level materials entry.
    """

    filtered_materials = export_settings['filtered_materials']
                  
    materials = []
    
    KHR_materials_pbrSpecularGlossiness_Used = False
    KHR_materials_common_Used = False
    KHR_materials_displacement_Used = False

    #
    #
    
    for blender_material in filtered_materials:
        # 
        # Property: material
        #

        material = {}

        #
        
        if blender_material.node_tree is not None and blender_material.use_nodes:
            
            #
            # Cycles Render.
            #
        
            for blender_node in blender_material.node_tree.nodes:
                if isinstance(blender_node, bpy.types.ShaderNodeGroup):
                    
                    alpha = 1.0
    
                    if blender_node.node_tree.name.startswith('glTF Metallic Roughness'):
                        # 
                        # Property: pbrMetallicRoughness
                        #
                        
                        material['pbrMetallicRoughness'] = {}
    
                        pbrMetallicRoughness = material['pbrMetallicRoughness']
    
                        #
                        # Base color texture
                        #
                        index = get_texture_index(export_settings, glTF, 'BaseColor', blender_node)
                        if index >= 0:
                            baseColorTexture = {
                                'index' : index
                            }
    
                            texCoord = get_texcoord_index(glTF, 'BaseColor', blender_node)
                            if texCoord > 0:
                                baseColorTexture['texCoord'] = texCoord
                            
                            pbrMetallicRoughness['baseColorTexture'] = baseColorTexture
    
                        #
                        # Base color factor
                        #
                        baseColorFactor = get_vec4(blender_node.inputs['BaseColorFactor'].default_value, [1.0, 1.0, 1.0, 1.0])
                        if baseColorFactor[0] != 1.0 or baseColorFactor[1] != 1.0 or baseColorFactor[2] != 1.0 or baseColorFactor[3] != 1.0:
                            pbrMetallicRoughness['baseColorFactor'] = baseColorFactor
                            alpha = baseColorFactor[3]
    
                        #
                        # Metallic factor
                        #
                        metallicFactor = get_scalar(blender_node.inputs['MetallicFactor'].default_value, 1.0)
                        if metallicFactor != 1.0:
                            pbrMetallicRoughness['metallicFactor'] = metallicFactor
    
                        #
                        # Roughness factor
                        #
                        roughnessFactor = get_scalar(blender_node.inputs['RoughnessFactor'].default_value, 1.0)
                        if roughnessFactor != 1.0:
                            pbrMetallicRoughness['roughnessFactor'] = roughnessFactor
    
                        #
                        # Metallic roughness texture
                        #
                        index = get_texture_index(export_settings, glTF, 'MetallicRoughness', blender_node)
                        if index >= 0:
                            metallicRoughnessTexture = {
                                'index' : index
                            }
                            
                            texCoord = get_texcoord_index(glTF, 'MetallicRoughness', blender_node)
                            if texCoord > 0:
                                metallicRoughnessTexture['texCoord'] = texCoord
    
                            pbrMetallicRoughness['metallicRoughnessTexture'] = metallicRoughnessTexture
                            
                    if blender_node.node_tree.name.startswith('glTF Specular Glossiness'):
                        KHR_materials_pbrSpecularGlossiness_Used = True
                        
                        # 
                        # Property: Specular Glossiness Material
                        #
            
                        pbrSpecularGlossiness = {}
                        
                        material['extensions'] = { 'KHR_materials_pbrSpecularGlossiness' : pbrSpecularGlossiness }
                        
                        #
                        # Diffuse texture
                        #
                        index = get_texture_index(export_settings, glTF, 'Diffuse', blender_node)
                        if index >= 0:
                            diffuseTexture = {
                                'index' : index
                            }
    
                            texCoord = get_texcoord_index(glTF, 'Diffuse', blender_node)
                            if texCoord > 0:
                                diffuseTexture['texCoord'] = texCoord
                            
                            pbrSpecularGlossiness['diffuseTexture'] = diffuseTexture
    
                        #
                        # Diffuse factor
                        #
                        diffuseFactor = get_vec4(blender_node.inputs['DiffuseFactor'].default_value, [1.0, 1.0, 1.0, 1.0])
                        if diffuseFactor[0] != 1.0 or diffuseFactor[1] != 1.0 or diffuseFactor[2] != 1.0 or diffuseFactor[3] != 1.0:
                            pbrSpecularGlossiness['diffuseFactor'] = diffuseFactor
                            alpha = diffuseFactor[3]

                        #
                        # Specular texture
                        #
                        index_a = get_texture_index(export_settings, glTF, 'Specular', blender_node)
                        index_b = get_texture_index(export_settings, glTF, 'Glossiness', blender_node)
                        if index_a >= 0 and index_b >= 0 and index_a == index_b:
                            specularGlossinessTexture = {
                                'index' : index_a
                            }
    
                            texCoord = get_texcoord_index(glTF, 'Specular', blender_node)
                            if texCoord > 0:
                                specularGlossinessTexture['texCoord'] = texCoord
                            
                            pbrSpecularGlossiness['specularGlossinessTexture'] = specularGlossinessTexture

                        #
                        # Specular factor
                        #
                        specularFactor = get_vec3(blender_node.inputs['SpecularFactor'].default_value, [1.0, 1.0, 1.0])
                        if specularFactor[0] != 1.0 or specularFactor[1] != 1.0 or specularFactor[2] != 1.0:
                            pbrSpecularGlossiness['specularFactor'] = specularFactor

                        #
                        # Glossiness factor
                        #
                        glossinessFactor = get_scalar(blender_node.inputs['GlossinessFactor'].default_value, 1.0)
                        if glossinessFactor != 1.0:
                            pbrSpecularGlossiness['glossinessFactor'] = glossinessFactor
                        
                    # TODO: Export displacement data for PBR.
    
                    #
                    # Emissive texture
                    #
                    index = get_texture_index(export_settings, glTF, 'Emissive', blender_node)
                    if index >= 0:
                        emissiveTexture = {
                            'index' : index
                        }
    
                        texCoord = get_texcoord_index(glTF, 'Emissive', blender_node)
                        if texCoord > 0:
                            emissiveTexture['texCoord'] = texCoord
    
                        material['emissiveTexture'] = emissiveTexture
    
                    #
                    # Emissive factor
                    #
                    emissiveFactor = get_vec3(blender_node.inputs['EmissiveFactor'].default_value, [0.0, 0.0, 0.0])
                    if emissiveFactor[0] != 0.0 or emissiveFactor[1] != 0.0 or emissiveFactor[2] != 0.0:
                        material['emissiveFactor'] = emissiveFactor
    
                    #
                    # Normal texture
                    #
                    index = get_texture_index(export_settings, glTF, 'Normal', blender_node)
                    if index >= 0:
                        normalTexture = {
                            'index' : index
                        }
    
                        texCoord = get_texcoord_index(glTF, 'Normal', blender_node)
                        if texCoord > 0:
                            normalTexture['texCoord'] = texCoord
    
                        scale = get_scalar(blender_node.inputs['NormalScale'].default_value, 1.0)
    
                        if scale != 1.0:
                            normalTexture['scale'] = scale
    
                        material['normalTexture'] = normalTexture
    
                    #
                    # Occlusion texture
                    #
                    if len(blender_node.inputs['Occlusion'].links) > 0:
                        index = get_texture_index(export_settings, glTF, 'Occlusion', blender_node)
                        if index >= 0:
                            occlusionTexture = {
                                'index' : index
                            }
    
                            texCoord = get_texcoord_index(glTF, 'Occlusion', blender_node)
                            if texCoord > 0:
                                occlusionTexture['texCoord'] = texCoord
    
                            strength = get_scalar(blender_node.inputs['OcclusionStrength'].default_value, 1.0)
    
                            if strength != 1.0:
                                occlusionTexture['strength'] = strength
    
                            material['occlusionTexture'] = occlusionTexture
    
                    #
                    # Alpha
                    #
                    index = get_texture_index(export_settings, glTF, 'Alpha', blender_node)
                    if index >= 0 or alpha < 1.0:
                        alphaMode = 'BLEND'
                        if get_scalar(blender_node.inputs['AlphaMode'].default_value, 0.0) >= 0.5:
                            alphaMode = 'MASK'
    
                            material['alphaCutoff'] = get_scalar(blender_node.inputs['AlphaCutoff'].default_value, 0.5)
    
                        material['alphaMode'] = alphaMode
                        
                    #
                    # Double sided
                    #
                    if get_scalar(blender_node.inputs['DoubleSided'].default_value, 0.0) >= 0.5:
                        material['doubleSided'] = True
                    
                    #
                    # Use Color_0
                    #
                    
                    if get_scalar(blender_node.inputs['Use COLOR_0'].default_value, 0.0) < 0.5:
                        export_settings['gltf_use_no_color'].append(blender_material.name)

                    #
                    
                    if export_settings['gltf_extras']:
                        extras = create_custom_property(blender_material)
                        
                        if extras is not None:
                            material['extras'] = extras 
            
                    #
            
                    material['name'] = blender_material.name
            
                    #
                    #
            
                    materials.append(material)

        else:
            
            #
            # Blender Render.
            #
            
            if export_settings['gltf_common']:
                KHR_materials_common_Used = True
                
                # 
                # Property: Common Material
                #
    
                common = { }
                
                material['extensions'] = { 'KHR_materials_cmnBlinnPhong' : common }
                
                alpha = 1.0
                alphaMode = 'OPAQUE'
                if blender_material.use_transparency:
                    alpha = blender_material.alpha
                    if blender_material.transparency_method == 'MASK':
                        alphaMode = 'MASK'
                    else:
                        alphaMode = 'BLEND'

                common['diffuseFactor'] = [blender_material.diffuse_color[0] * blender_material.diffuse_intensity, blender_material.diffuse_color[1] * blender_material.diffuse_intensity, blender_material.diffuse_color[2] * blender_material.diffuse_intensity, alpha]
    
                if alphaMode != 'OPAQUE': 
                    material['alphaMode'] = alphaMode

                common['specularFactor'] = [blender_material.specular_color[0] * blender_material.specular_intensity, blender_material.specular_color[1] * blender_material.specular_intensity, blender_material.specular_color[2] * blender_material.specular_intensity]
    
                shininessFactor = 128.0 * (float(blender_material.specular_hardness) - 1.0) / 510.0
    
                common['shininessFactor'] = shininessFactor
    
                #
                
                material['emissiveFactor'] = [blender_material.emit * blender_material.diffuse_color[0], blender_material.emit * blender_material.diffuse_color[1], blender_material.emit * blender_material.diffuse_color[2]]
                
                #
                
                for blender_texture_slot in blender_material.texture_slots:
                    if blender_texture_slot and blender_texture_slot.texture and blender_texture_slot.texture.type == 'IMAGE' and blender_texture_slot.texture.image is not None:
                        #
                        # Diffuse texture
                        #
                        if blender_texture_slot.use_map_color_diffuse:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                diffuseTexture = {
                                    'index' : index
                                }
                                common['diffuseTexture'] = diffuseTexture
                        #
                        # Specular shininess texture
                        #
                        if blender_texture_slot.use_map_color_spec:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                specularShininessTexture = {
                                    'index' : index
                                }
                                common['specularShininessTexture'] = specularShininessTexture
                        #
                        # Ambient texture
                        #
                        if blender_texture_slot.use_map_ambient:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                ambientTexture = {
                                    'index' : index
                                }
                                common['ambientTexture'] = ambientTexture
                        #
                        # Emissive texture
                        #
                        if blender_texture_slot.use_map_emit:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                emissiveTexture = {
                                    'index' : index
                                }
                                material['emissiveTexture'] = emissiveTexture
                        #
                        # Normal texture
                        #
                        if blender_texture_slot.use_map_normal:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                normalTexture = {
                                    'index' : index
                                }
                                material['normalTexture'] = normalTexture
                                
                        #
                        # Displacement textue
                        #
                        if export_settings['gltf_displacement']:
                            if blender_texture_slot.use_map_displacement:
                                index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                                if index >= 0:
                                    extensions = material['extensions']

                                    # 
                                    
                                    displacementTexture = {
                                        'index' : index,
                                        'strength' : blender_texture_slot.displacement_factor 
                                    }
                                     
                                    extensions['KHR_materials_displacement'] = {'displacementTexture' : displacementTexture}
                                    
                                    #
                                    
                                    KHR_materials_displacement_Used = True

                #
                
                if export_settings['gltf_extras']:
                    extras = create_custom_property(blender_material)
                    
                    if extras is not None:
                        material['extras'] = extras 
        
                #
        
                material['name'] = blender_material.name
        
                #
                #
        
                materials.append(material)

            else:

                #
                # A minimal export of basic material properties that didn't get picked up any other way to a pbrMetallicRoughness glTF material
                #
                material['pbrMetallicRoughness'] = {}

                pbrMetallicRoughness = material['pbrMetallicRoughness']

                alpha = 1.0
                alphaMode = 'OPAQUE'
                if blender_material.use_transparency:
                    alpha = blender_material.alpha
                    if blender_material.transparency_method == 'MASK':
                        alphaMode = 'MASK'
                    else:
                        alphaMode = 'BLEND'

                if alphaMode != 'OPAQUE': 
                    material['alphaMode'] = alphaMode

                for blender_texture_slot in blender_material.texture_slots:
                    if blender_texture_slot and blender_texture_slot.texture and blender_texture_slot.texture.type == 'IMAGE' and blender_texture_slot.texture.image is not None:
                        #
                        # Diffuse texture becmomes baseColorTexture
                        #
                        if blender_texture_slot.use_map_color_diffuse:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                baseColorTexture = {
                                    'index' : index
                                }
                                pbrMetallicRoughness['baseColorTexture'] = baseColorTexture
                        
                        #
                        # Ambient texture becomes occlusionTexture
                        #
                        if blender_texture_slot.use_map_ambient:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                ambientTexture = {
                                    'index' : index
                                }
                                material['occlusionTexture'] = ambientTexture
                        
                        #
                        # Emissive texture
                        #
                        if blender_texture_slot.use_map_emit:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                emissiveTexture = {
                                    'index' : index
                                }
                                material['emissiveTexture'] = emissiveTexture

                        #
                        # Normal texture
                        #
                        if blender_texture_slot.use_map_normal:
                            index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                            if index >= 0:
                                normalTexture = {
                                    'index' : index
                                }
                                material['normalTexture'] = normalTexture
                                
                        #
                        # Displacement textue
                        #
                        if export_settings['gltf_displacement']:
                            if blender_texture_slot.use_map_displacement:
                                index = get_texture_index_by_filepath(export_settings, glTF, blender_texture_slot.texture.image.filepath)
                                if index >= 0:
                                    extensions = material['extensions']

                                    # 
                                    
                                    displacementTexture = {
                                        'index' : index,
                                        'strength' : blender_texture_slot.displacement_factor 
                                    }
                                     
                                    extensions['KHR_materials_displacement'] = {'displacementTexture' : displacementTexture}
                                    
                                    #
                                    
                                    KHR_materials_displacement_Used = True

                #
                # Base color factor
                #
                baseColorFactor = [blender_material.diffuse_color[0] * blender_material.diffuse_intensity, blender_material.diffuse_color[1] * blender_material.diffuse_intensity, blender_material.diffuse_color[2] * blender_material.diffuse_intensity, alpha]
                if baseColorFactor[0] != 1.0 or baseColorFactor[1] != 1.0 or baseColorFactor[2] != 1.0 or baseColorFactor[3] != 1.0:
                    pbrMetallicRoughness['baseColorFactor'] = baseColorFactor
                    alpha = baseColorFactor[3]

                #
                # Metallic factor has to be 0.0 for not breaking the Metallic-Roughness workflow.
                #
                pbrMetallicRoughness['metallicFactor'] = 0.0

                #
                # Emissive factor
                #
                emissiveFactor = [blender_material.emit * blender_material.diffuse_color[0], blender_material.emit * blender_material.diffuse_color[1], blender_material.emit * blender_material.diffuse_color[2]]
                if emissiveFactor[0] != 0.0 or emissiveFactor[1] != 0.0 or emissiveFactor[2] != 0.0:
                    material['emissiveFactor'] = emissiveFactor

                #
                
                if export_settings['gltf_extras']:
                    extras = create_custom_property(blender_material)
                    
                    if extras is not None:
                        material['extras'] = extras 
        
                #

                material['name'] = blender_material.name
                
                #
                #

                materials.append(material)

    #
    #

    if len (materials) > 0:
        if KHR_materials_pbrSpecularGlossiness_Used:
            create_extensionsUsed(operator, context, export_settings, glTF, 'KHR_materials_pbrSpecularGlossiness')
            create_extensionsRequired(operator, context, export_settings, glTF, 'KHR_materials_pbrSpecularGlossiness')
            
        if KHR_materials_common_Used:
            create_extensionsUsed(operator, context, export_settings, glTF, 'KHR_materials_cmnBlinnPhong')
            create_extensionsRequired(operator, context, export_settings, glTF, 'KHR_materials_cmnBlinnPhong')
            
        if KHR_materials_displacement_Used:
            create_extensionsUsed(operator, context, export_settings, glTF, 'KHR_materials_displacement')
            create_extensionsRequired(operator, context, export_settings, glTF, 'KHR_materials_displacement')

        glTF['materials'] = materials


def generate_scenes(operator,
                    context,
                    export_settings,
                    glTF):
    """
    Generates the top level scenes entry.
    """

    scenes = []

    #

    for blender_scene in bpy.data.scenes:
        # 
        # Property: scene
        #

        scene = {}

        #
        
        nodes = []
            
        for blender_object in blender_scene.objects:
            if blender_object.parent is None:
                node_index = get_node_index(glTF, blender_object.name)
                
                if node_index < 0:
                    continue
                
                nodes.append(node_index)

        if len(nodes) > 0:
            scene['nodes'] = nodes
                
        #

        if export_settings['gltf_lights_cmn']:
            light = get_light_index_cmn(glTF, 'Ambient_' + blender_scene.name)
            if light >= 0:
                khr_lights_cmn = {'light' : light}
                extensions = {'KHR_lights_cmn' : khr_lights_cmn}
                scene['extensions'] = extensions

        #
        
        if export_settings['gltf_extras']:
            extras = create_custom_property(blender_scene.world)
            
            if extras is not None:
                scene['extras'] = extras 

        #

        scene['name'] = blender_scene.name

        #
        #
        
        scenes.append(scene)

    #
    #

    if len (scenes) > 0:
        glTF['scenes'] = scenes


def generate_scene(operator,
                    context,
                    export_settings,
                    glTF):
    """
    Generates the top level scene entry.
    """

    index = get_scene_index(glTF, bpy.context.screen.scene.name)
    
    #
    #

    if index >= 0:
        glTF['scene'] = index


def generate_glTF(operator,
                  context,
                  export_settings,
                  glTF):
    """
    Generates the main glTF structure.
    """

    profile_start()
    generate_asset(operator, context, export_settings, glTF)
    profile_end('asset')
    bpy.context.window_manager.progress_update(5)
    
    #

    if export_settings['gltf_materials']:
        profile_start()
        generate_images(operator, context, export_settings, glTF)
        profile_end('images')
        bpy.context.window_manager.progress_update(10)
    
        profile_start()
        generate_textures(operator, context, export_settings, glTF)
        profile_end('textures')
        bpy.context.window_manager.progress_update(20)
    
        profile_start()
        generate_materials(operator, context, export_settings, glTF)
        profile_end('materials')
        bpy.context.window_manager.progress_update(30)

    bpy.context.window_manager.progress_update(30)
    
    #

    if export_settings['gltf_cameras']:
        profile_start()
        generate_cameras(operator, context, export_settings, glTF)
        profile_end('cameras')
        bpy.context.window_manager.progress_update(40)
        
    if export_settings['gltf_lights_cmn']:
        profile_start()
        generate_lights_cmn(operator, context, export_settings, glTF)        
        profile_end('lights cmn')
        bpy.context.window_manager.progress_update(50)
    
    if export_settings['gltf_lights_pbr']:
        profile_start()
        generate_lights_pbr(operator, context, export_settings, glTF)        
        profile_end('lights pbr')
        bpy.context.window_manager.progress_update(50)
    
    bpy.context.window_manager.progress_update(50)
    
    #
    
    profile_start()
    generate_meshes(operator, context, export_settings, glTF)
    profile_end('meshes')
    bpy.context.window_manager.progress_update(60)
    
    #

    profile_start()
    generate_nodes(operator, context, export_settings, glTF)
    profile_end('nodes')
    bpy.context.window_manager.progress_update(70)
    
    #
    
    if export_settings['gltf_animations']:
        profile_start()
        generate_animations(operator, context, export_settings, glTF)
        profile_end('animations')
        bpy.context.window_manager.progress_update(80)
        
    bpy.context.window_manager.progress_update(80)
    
    #
    
    profile_start()
    generate_scenes(operator, context, export_settings, glTF)
    profile_end('scenes')
    
    bpy.context.window_manager.progress_update(95)
    
    profile_start()
    generate_scene(operator, context, export_settings, glTF)
    profile_end('scene')
    
    bpy.context.window_manager.progress_update(100)
    
    #
    
    byteLength = len(export_settings['gltf_binary']) 
    
    if byteLength > 0:
        glTF['buffers'] = []

        buffer = {
            'byteLength' : byteLength
        }
        
        if export_settings['gltf_format'] == 'ASCII':        
            uri = export_settings['gltf_binaryfilename']
            
            if export_settings['gltf_embed_buffers']:
                uri = 'data:application/octet-stream;base64,' + base64.b64encode(export_settings['gltf_binary']).decode('ascii')
                
            buffer['uri'] = uri
        
        glTF['buffers'].append(buffer)
