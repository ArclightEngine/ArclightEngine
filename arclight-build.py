#!/usr/bin/python3

from os import chdir, mkdir, path
import subprocess
import sys
import json
from argparse import ArgumentParser, RawTextHelpFormatter, SUPPRESS

arclight_root = path.dirname(path.realpath(__file__))

def build():
    if not path.isdir("Build"):
        subprocess.run(["meson", "setup", "Build", f"-Darclight_engine_path={arclight_root}"], check=True)

    subprocess.run(["ninja", "-CBuild"], check=True)

def create_project():
    project_file = open("project.arcproj", "w")
    meson_file = open("meson.build", "w")
    meson_options_file = open("meson_options.txt", "w")

    mkdir("src")
    main_file = open("src/Main.cpp", "w")

    main_template = open(path.join(arclight_root, "Data", "Main_template.cpp"))
    meson_template = open(path.join(arclight_root, "Data", "project_template.meson"))
    meson_options_template = open(path.join(arclight_root, "Data", "project_template_meson_options.txt"))

    project = {
        "name": "Arclight Project",
    }

    project_file.write(json.dumps(project))
    meson_file.write(meson_template.read())
    meson_options_file.write(meson_options_template.read())
    main_file.write(main_template.read())

def package():
    pass

def run():
    engine = path.join(path.dirname(path.realpath(__file__)), "Build", "arclight")
    subprocess.run([engine, 'Build'], check=True)

commands = {
    "build": (build, "Build project"),
    "create": (create_project, "Create a new project"),
    "run": (run, "Run project")
}

def command_descriptions():
    text = "commands:\n"
    for name, cmd in commands.items():
        text += f"  { name.ljust(12) }{ cmd[1] }\n";

    return text

if __name__ == "__main__":
    parser = ArgumentParser(formatter_class=RawTextHelpFormatter, epilog=command_descriptions())
    parser.add_argument("command", help=SUPPRESS)
    parser.add_argument("--dir", action='store', type=str)

    args = parser.parse_args(sys.argv[1:])
    if args.dir != None:
        if not path.isdir(args.dir):
            print(f"Invalid game directory '{args.dir}'");
            exit(1)
        else:
            chdir(args.dir)

    try:
        commands[args.command][0]()
    except KeyError:
        print(f"arclight-build: Invalid command '{args.command}'")
