#!/usr/bin/python3

from os import path
import subprocess
import sys
from getopt import getopt

def help():
    print("arclight-build <path to game> <command>\n\nCommands:\nbuild\nrun\npackage\n\nPlatforms:\nhost (default)")

def build():
    pass

def run(game_path: str):
    engine = path.join(path.dirname(path.realpath(__file__)), "build", "arclight")
    subprocess.run([engine, game_path])

def package():
    pass

if __name__ == "__main__":
    options, args = getopt(sys.argv[1:], "h", ["platform="])

    if '-h' in options or len(args) <= 1:
        help()
    
    if args[1] == "run":
        run(args[0])
    else:
        print(f"Unknown command: {args[1]}")

