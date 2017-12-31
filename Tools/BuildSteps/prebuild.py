import sys
assert (sys.version_info.major, sys.version_info.minor) >= (3, 6)

import argparse
from distutils import dir_util
import json
import os
from os import path
import subprocess
import time

def find_repo_root():
    return subprocess.check_output(("git", "rev-parse", "--show-toplevel")).strip().decode("utf8")


REPO_ROOT_DIR = path.abspath(find_repo_root())


def get_last_modified_time(directory):
    unflattened_tree = ((path.join(subdir, f) for f in files)
                        for subdir, _, files in os.walk(directory))
    flattened_tree = (f for list in unflattened_tree for f in list)
    stat_time = max(os.stat(str(f)).st_mtime for f in flattened_tree)
    return time.localtime(stat_time)


def run_tools(tools):
    print("Running checks for changed content...")
    prebuild_data_path = path.join(path.dirname(__file__), "prebuilddata.json")
    if path.isfile(prebuild_data_path):
        with open(prebuild_data_path, mode="r", encoding="utf8") as prebuild_data_file:
            data_storage = json.loads(prebuild_data_file.read(), encoding="utf8")
    else:
        data_storage = json.loads("{}")

    for tool_description in tools:
        last_changed = get_last_modified_time(tool_description.directory)
        last_recorded = time.strptime(data_storage.get(tool_description.directory) or time.ctime(0))
        if last_recorded < last_changed:
            print(f"Detected that dir {tool_description.directory} has changed. "
                  f"Running {tool_description.cmd}...")
            process_handle = subprocess.run(tool_description.cmd, cwd=REPO_ROOT_DIR, timeout=20,
                                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            print(process_handle.stdout.decode("utf8"))
            try:
                process_handle.check_returncode()
            except subprocess.CalledProcessError as err:
                print(f"Processed failed with return code: {err.returncode}")
                sys.exit(1)
            data_storage[tool_description.directory] = time.asctime()

    with open(prebuild_data_path, mode="w", encoding="utf8") as prebuild_data_file:
        prebuild_data_file.write(json.dumps(data_storage))


class ToolDescription:
    def __init__(self, directory, cmd):
        self.directory = directory
        self.cmd = cmd


def main():
    parser = argparse.ArgumentParser(description="Prebuild step that runs engine tools if necessary.")
    parser.add_argument("--output", type=str, required=True, dest="output",
                        help="the path to the engine's build directory")
    parser.add_argument("--game", dest="isgame", default=False, action="store_true",
                        help="run the tools for the game instead of the tools for the engine")
    parser.add_argument("--configuration", dest="configuration", required=True, action="store",
                        choices=["Debug", "Release"],
                        help="run the tools for the game instead of the tools for the engine")

    args = parser.parse_args()
    if not args.isgame:
        # Copy all dependencies to the output
        print("Copying third party dependencies to the output directory")
        thirdparties = path.join(REPO_ROOT_DIR, 'ThirdParty/binx64')
        thirdparties_per_config = path.join(REPO_ROOT_DIR, 'ThirdParty/binx64', args.configuration)
        dir_util.copy_tree(thirdparties, args.output, update=1)
        dir_util.copy_tree(thirdparties_per_config, args.output, update=1)

    shaders_dir = path.join(REPO_ROOT_DIR, "Source/Graphics/Shaders/Source")
    shader_compiler = path.join(args.output, "ShaderCompiler.exe")

    content_dir = path.join(REPO_ROOT_DIR, "Games/GiftOfTheSanctum/Content")
    incinerator_compiler = path.join(args.output, "Incinerator.exe")

    engine_tools = (
        ToolDescription(shaders_dir, shader_compiler),
    )
    game_tools = (
        ToolDescription(content_dir, (incinerator_compiler, "--game", "Games/GiftOfTheSanctum")),
    )
    tools_to_run = (engine_tools, game_tools)[args.isgame]
    run_tools(tools_to_run)


if __name__ == "__main__":
    main()
