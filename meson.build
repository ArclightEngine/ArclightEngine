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
    dependency('freetype2'),
    dependency('icu-i18n')
]

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
endif

subdir('Engine')

executable('arclight', [ 'Main/Main.cpp' ], include_directories: inc_dirs,
    dependencies : [deps, engine_dep], link_args: ['-ldl', '-rdynamic'])
