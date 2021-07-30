#!/usr/bin/python3

from os import close, path
import subprocess

arclight_root = path.dirname(__file__)

def dump_shader_file(file, name: str) -> str:
    text = "std::vector<uint8_t> " + name + "Data = {\n    "

    lineCount = 0 # Put 8 bytes on each line
    data = file.read()
    for byte in data:
        if lineCount >= 8:
            text += "\n    "
            lineCount = 0

        text += f"{hex(byte)}, ".ljust(5)
        lineCount += 1

    text += "\n};\n"
    return text

if __name__ == "__main__":
    subprocess.run(["glslc", path.join(arclight_root, "Data/shaders/default.vert"), "-o", path.join(arclight_root, "Build/default_vert.spv")], check=True)
    subprocess.run(["glslc", path.join(arclight_root, "Data/shaders/default.frag"), "-o", path.join(arclight_root, "Build/default_frag.spv")], check=True)
    
    frag = open(path.join(arclight_root, "Build/default_frag.spv"), "rb")
    vert = open(path.join(arclight_root, "Build/default_vert.spv"), "rb")
    output = open(path.join(arclight_root, "Engine/src/Graphics/Rendering/Vulkan/DefaultShaderBytecode.h"), "w")

    header_file = dump_shader_file(frag, "defaultFragmentShader") + "\n" + dump_shader_file(vert, "defaultVertexShader")
    output.write(header_file)

    frag.close()
    vert.close()
    output.close()
