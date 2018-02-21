import argparse
import bpy
import os

if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    # get all script args
    # relevant arguments for this script are the ones after '--' and the first one
    _, all_arguments = parser.parse_known_args()
    double_dash_index = all_arguments.index('--')
    script_args = all_arguments[double_dash_index + 1: ]

    parser.add_argument('--output-folder', dest='output_folder', help="output folder")

    args, _ = parser.parse_known_args(script_args)

    if args.output_folder is not None:
        # Ensure folder
        if not os.path.exists(args.output_folder):
            os.makedirs(args.output_folder)

        # Blend file to open will be the first argument from the whole list
        blend_file = all_arguments[0]
        world_name = os.path.basename(blend_file).rsplit('.')[0] + '.world'
        # end the path with a slash as blender requires it
        output_dir = os.path.normpath(os.path.abspath(args.output_folder)) + os.path.sep

        print("Exporting scene in folder '" + output_dir + "' with main .world file '" + world_name + "'")

        # export scene
        bpy.ops.export_scene.zmey(filepath=world_name, directory=output_dir)