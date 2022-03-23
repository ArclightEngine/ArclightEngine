#!/usr/bin/python3

from os import chdir, mkdir, path, environ
import shutil
import subprocess
import sys
import platform
import json
import http.server
from argparse import ArgumentParser, RawTextHelpFormatter, SUPPRESS

arclight_root = path.dirname(path.realpath(__file__))
build_platform = "host"
# Only applies to wasm platform
webserver_port = 8001

environ["ARCLIGHT_ENGINE_PATH"] = arclight_root

# Get build directory for platform
def platform_build_dir():
    if build_platform == "host":
        return "Build"

    return path.join("Build", build_platform)

# Function to run the relevant cmake command
def run_cmake(args):
    if build_platform == "wasm":
        subprocess.run(["emcmake", "cmake"] + args, check=True)
    else:
        assert(build_platform == "host")
        subprocess.run(["cmake"] + args, check=True)
        

def rebuild():
    if path.exists("Build"):
        shutil.rmtree("Build")
        mkdir("Build")

    builddir = platform_build_dir()

    run_cmake(["-B", builddir, "-GNinja",
                   f"-DARCLIGHT_ENGINE_PATH={arclight_root}"])
    subprocess.run(["ninja", "-C", builddir], check=True)


def build():
    builddir = platform_build_dir()
    if not path.isdir(builddir):
        run_cmake(["-B", builddir, "-GNinja",
                    f"-DARCLIGHT_ENGINE_PATH={arclight_root}"])

    subprocess.run(["ninja", "-C", builddir], check=True)

def create_project():
    project_file = open("project.arcproj", "w")
    cmake_file = open("CMakeLists.txt", "w")

    mkdir("src")
    main_file = open("src/Main.cpp", "w")

    main_template = open(path.join(arclight_root, "Data", "Main_template.cpp"))
    cmake_template = open(path.join(arclight_root, "Data", "CMakeLists.txt"))

    project = {
        "name": "Arclight Project",
    }

    cmake_contents = str(cmake_template.read()).replace("%ARCLIGHTBUILD_ENGINE_PATH%", arclight_root)

    project_file.write(json.dumps(project))
    cmake_file.write(cmake_contents)
    main_file.write(main_template.read())


def package():
    pass


def run():
    if build_platform == "wasm":
        if not path.isfile("game.html"):
            shutil.copy(path.join(arclight_root, "Data", "wasm.html"), "game.html")
        print(f"Open http://0.0.0.0:{webserver_port}/game.html in your browser")
        server = http.server.HTTPServer(('', webserver_port), http.server.SimpleHTTPRequestHandler)
        server.serve_forever()
    else:
        engine = path.join(path.dirname(
            path.realpath(__file__)), "Build", "arclight")
        subprocess.run([engine], check=True)


platforms = {
    "host": f"Host platform ({platform.system().lower()})",
    "wasm": "HTML5/WebAssembly (emscripten)"
}

commands = {
    "build": (build, "Build project"),
    "rebuild": (rebuild, "Reconfigure and build project"),
    "create": (create_project, "Create a new project"),
    "run": (run, "Run project")
}


def command_descriptions():
    text = "platforms:\n"
    for name, plat in platforms.items():
        text += f"  { name.ljust(12) }{ plat }\n"

    text += "\ncommands:\n"
    for name, cmd in commands.items():
        text += f"  { name.ljust(12) }{ cmd[1] }\n"

    return text


if __name__ == "__main__":
    parser = ArgumentParser(
        formatter_class=RawTextHelpFormatter, epilog=command_descriptions())
    parser.add_argument("command", help=SUPPRESS)
    parser.add_argument("--dir", action='store', type=str,
                        help="Set project directory")
    parser.add_argument("--platform", action='store',
                        type=str, help="Build for PLATFORM")

    args = parser.parse_args(sys.argv[1:])
    if args.dir != None:
        if not path.isdir(args.dir):
            print(f"Invalid game directory '{args.dir}'")
            exit(1)
        else:
            chdir(args.dir)

    if args.platform != None:
        build_platform = args.platform
        if not build_platform in platforms:
            print(f"arclight-build: Invalid platform '{args.platform}'")
            exit(1)

    try:
        commands[args.command][0]()
    except KeyError:
        print(f"arclight-build: Invalid command '{args.command}'")
