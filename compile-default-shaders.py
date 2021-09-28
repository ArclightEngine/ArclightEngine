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
    subprocess.run(["glslc", path.join(arclight_root, "Data/shaders/default_vulkan.vert"), "-o", path.join(arclight_root, "Build/default_vert.spv")], check=True)
    subprocess.run(["glslc", path.join(arclight_root, "Data/shaders/default_vulkan.frag"), "-o", path.join(arclight_root, "Build/default_frag.spv")], check=True)
    
    frag_gl = open(path.join(arclight_root, "Data/shaders/default_gles.frag"), "rb")
    vert_gl = open(path.join(arclight_root, "Data/shaders/default_gles.vert"), "rb")
    output_gl = open(path.join(arclight_root, "Engine/Rendering/OpenGL/DefaultShaderSource.h"), "w")

    gl_header_file = dump_shader_file(frag_gl, "defaultFragmentShader") + "\n" + dump_shader_file(vert_gl, "defaultVertexShader")
    output_gl.write(gl_header_file)

    frag_gl.close()
    vert_gl.close()
    output_gl.close()

    frag_spv = open(path.join(arclight_root, "Build/default_frag.spv"), "rb")
    vert_spv = open(path.join(arclight_root, "Build/default_vert.spv"), "rb")
    output_spv = open(path.join(arclight_root, "Engine/Rendering/Vulkan/DefaultShaderBytecode.h"), "w")

    vulkan_header_file = dump_shader_file(frag_spv, "defaultFragmentShader") + "\n" + dump_shader_file(vert_spv, "defaultVertexShader")
    output_spv.write(vulkan_header_file)

    frag_spv.close()
    vert_spv.close()
    output_spv.close()
