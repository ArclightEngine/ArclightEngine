cpp_files = [
    'src/Core/Application.cpp',
    'src/Core/File.cpp',
    'src/Core/Input.cpp',
    'src/Core/Logger.cpp',
    'src/Core/Resource.cpp',
    'src/Core/ResourceManager.cpp',
    'src/Core/ThreadPool.cpp',

    'src/ECS/System.cpp',
    'src/ECS/World.cpp',

    'src/Graphics/Font.cpp',
    'src/Graphics/Image.cpp',
    'src/Graphics/Matrix.cpp',
    'src/Graphics/Text.cpp',
    'src/Graphics/Texture.cpp',
    'src/Graphics/Transform.cpp',

    'src/Graphics/Rendering/Pipeline.cpp',
    'src/Graphics/Rendering/RendererBackend.cpp',
    'src/Graphics/Rendering/Shader.cpp',

    'src/Platform/Platform.cpp',

    'src/State/StateManager.cpp',

    'src/Systems/Renderer2D.cpp',

    'src/Window/WindowContext.cpp',
]

inc_dirs = [ include_directories('include'),  include_directories('../thirdparty'), include_directories('.') ]

if get_option('use_sdl2')
    deps += dependency('sdl2')

    add_global_arguments('-DARCLIGHT_SDL2=1', language : ['c', 'cpp'])
else
    error('No windowing library specified (SDL2)')
endif

if get_option('use_vulkan')
    deps += dependency('vulkan')

    subdir('Rendering/Vulkan')

    add_global_arguments('-DARCLIGHT_VULKAN=1', language : ['c', 'cpp'])
    endif
endif
if get_option('use_dummy_renderer')
    add_global_arguments('-DARCLIGHT_DUMMY_RENDERER=1', language : ['c', 'cpp'])
endif

if is_unix == true
    subdir('Platform/UNIX')
elif host_machine.system() == 'windows'
    subdir('Platform/Windows')
endif

add_project_arguments(cpp_args, language : 'cpp')
lib = static_library('arclight', cpp_files, include_directories: inc_dirs,
    dependencies : deps)

engine_dep = declare_dependency(link_with: lib, link_whole: lib, include_directories: inc_dirs)
