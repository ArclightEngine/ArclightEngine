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

inc_dirs = [ include_directories('Engine/include'),  include_directories('thirdparty') ]

deps = [

]

if host_machine.system() == 'emscripten'
    cpp_args += [
        '-sUSE_FREETYPE=2',
        '-sUSE_ICU=1',
        '-sUSE_PTHREADS=1'
    ]
else
    deps += [
        dependency('freetype2'),
        dependency('icu-i18n')
    ]
endif

is_unix = false
if host_machine.system() == 'linux'
    is_unix = true
elif host_machine.system() == 'darwin'
    is_unix = true
elif host_machine.system() == 'freebsd'
    is_unix = true
elif host_machine.system() == 'lemon'
    is_unix = true
elif host_machine.system() == 'netbsd'
    is_unix = true
elif host_machine.system() == 'openbsd'
    is_unix = true
elif host_machine.system() == 'haiku'
    is_unix = true
elif host_machine.system() == 'dragonfly'
    is_unix = true
elif host_machine.system() == 'emscripten'
    is_unix = true # Emscripten emulates a unix-ish environment
endif

add_project_arguments(cpp_args, language : 'cpp')

subdir('Engine')

link_args = [

]

ex_name = 'arclight'

if host_machine.system() == 'emscripten'
    link_args += ['-s', 'USE_SDL=2']
    #link_args += ['-s', 'USE_ICU=1']
    link_args += ['-s', 'USE_FREETYPE=2']
    link_args += ['-s', 'USE_PTHREADS=1']
    link_args += ['-s', 'PTHREAD_POOL_SIZE=2']

    static_library(ex_name, [ 'Main/Main.cpp' ], include_directories: inc_dirs,
        dependencies : [deps, engine_dep], link_args: link_args)
elif is_unix
    link_args += ['-ldl', '-rdynamic']

    executable(ex_name, [ 'Main/Main.cpp' ], include_directories: inc_dirs,
        dependencies : [deps, engine_dep], link_args: link_args)
endif
