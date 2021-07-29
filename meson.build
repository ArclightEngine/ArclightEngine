project('ArclightEngine',
    default_options : [
        'warning_level=2',
        'cpp_std=c++2a',
    ])
add_languages('c', 'cpp')

cpp_args = [
    '-Wno-missing-field-initializers',
    '-Wno-unused-parameter',
    '-fPIC',
    '-g',
]

add_project_arguments(cpp_args, language : 'cpp')

inc_dirs = [ include_directories('Engine/include'),  include_directories('thirdparty') ]

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

subdir('Engine')

executable('arclight', [ 'Main/Main.cpp' ], include_directories: inc_dirs,
    dependencies : [deps, engine_dep], link_args: ['-ldl', '-rdynamic'])
