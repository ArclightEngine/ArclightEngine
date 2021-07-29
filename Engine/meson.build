cpp_files = [
    'src/ResourceManager.cpp',

    'src/Core/Input.cpp',
    'src/Core/Logger.cpp',
    'src/Core/Node.cpp',
    'src/Core/Scene.cpp',
    'src/Core/ThreadPool.cpp',

    'src/Graphics/Image.cpp',
    'src/Graphics/Matrix.cpp',
    'src/Graphics/Sprite.cpp',
    'src/Graphics/Texture.cpp',
    'src/Graphics/Transform.cpp',

    'src/Graphics/Rendering/Pipeline.cpp',
    'src/Graphics/Rendering/RendererBackend.cpp',
    'src/Graphics/Rendering/RenderObject.cpp',
    'src/Graphics/Rendering/Shader.cpp',
    'src/Graphics/Rendering/Vulkan/VulkanMemory.cpp',
    'src/Graphics/Rendering/Vulkan/VulkanPipeline.cpp',
    'src/Graphics/Rendering/Vulkan/VulkanRenderer.cpp',
    'src/Graphics/Rendering/Vulkan/VulkanTexture.cpp',

    'src/Platform/Platform.cpp',

    'src/Window/WindowContext.cpp',
]

inc_dirs = [ include_directories('include'),  include_directories('../thirdparty') ]

add_project_arguments(cpp_args, language : 'cpp')
lib = static_library('arclight', cpp_files, include_directories: inc_dirs,
    dependencies : deps)

engine_dep = declare_dependency(link_with: lib, link_whole: lib, include_directories: inc_dirs)