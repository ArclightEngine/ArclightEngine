project('RPG',
    default_options : [
        'warning_level=2',
        'cpp_std=c++2a',
    ])
add_languages('c', 'cpp')

cpp_args = [
    
]

cpp_files = [
    'src/Main.cpp',
    'src/ResourceManager.cpp',

    'src/Graphics/Rendering/Pipeline.cpp',
    'src/Graphics/Rendering/RendererBackend.cpp',
    'src/Graphics/Rendering/ShaderBase.cpp',
    'src/Graphics/Rendering/Vulkan/VulkanMemory.cpp',
    'src/Graphics/Rendering/Vulkan/VulkanPipeline.cpp',
    'src/Graphics/Rendering/Vulkan/VulkanRenderer.cpp',
    'src/Graphics/Rendering/Vulkan/VulkanShader.cpp',

    'src/Window/WindowContext.cpp',
]

inc_dirs = include_directories('src')

sdl2 = dependency('sdl2')
vulkan = dependency('vulkan')

add_project_arguments(cpp_args, language : 'cpp')
executable('game', cpp_files, include_directories: inc_dirs,
    dependencies : [ sdl2, vulkan ])
