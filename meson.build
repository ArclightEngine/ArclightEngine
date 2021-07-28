project('ArclightEngine',
    default_options : [
        'warning_level=2',
        'cpp_std=c++2a',
    ])
add_languages('c', 'cpp')

cpp_args = [
    '-Wno-missing-field-initializers',
    '-Wno-unused-parameter',
]

cpp_files = [
    'src/Main.cpp',
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

    'src/Window/WindowContext.cpp',
]

inc_dirs = [ include_directories('include'),  include_directories('thirdparty') ]

deps = [

]

if get_option('use_sdl2')
    deps += dependency('sdl2')

    add_global_arguments('-DARCLIGHT_SDL2=1', language : ['c', 'cpp'])
else
    error('No windowing library specified (SDL2)')
endif

if get_option('use_vulkan')
    deps += dependency('vulkan')

    add_global_arguments('-DARCLIGHT_VULKAN=1', language : ['c', 'cpp'])
else
    error('No rendering library specified (Vulkan)')
endif

add_project_arguments(cpp_args, language : 'cpp')
executable('arclight', cpp_files, include_directories: inc_dirs,
    dependencies : deps)

run_target('run',
    command : ['build/arclight testgame'])
