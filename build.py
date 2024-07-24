#!/usr/bin/env python3

import sys
import subprocess


def run(args):
    res = subprocess.run(args)
    if res.returncode != 0:
        print("Exiting because of nonzero return code")
        sys.exit(1)


def build_file(filename):
    first_args = ["gcc", "-g", "-Wall", "-Wl,--no-as-needed"]
    run(first_args + ["-fPIC", "-c", filename, "-o", "entry.o", "-rdynamic"])
    run(first_args + ["-shared", "-o", "entry.so", "entry.o", "-rdynamic"])
    run(["rm", "entry.o"])
    run(["mkdir", "-p", "mvp-godot-project/build"])
    run(["mv", "entry.so", "mvp-godot-project/build"])


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} [source-file]")
        sys.exit(1)

    build_file(sys.argv[1])
