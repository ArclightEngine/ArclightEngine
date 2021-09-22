project('Game',
    default_options : ['warning_level=2', 'cpp_std=c++2a', 'b_lundef=false'])
add_languages('c', 'cpp')

cpp_args = [
    '-fPIC',
    '-g',
]

add_project_arguments(cpp_args, language : 'cpp')

inc_dirs = [
    include_directories(join_paths(get_option('arclight_engine_path'), 'Engine', 'include')),
    include_directories(join_paths(get_option('arclight_engine_path'), 'thirdparty'))
]

game_cpp_files = [
    'src/Main.cpp'
]

game = shared_library('game', game_cpp_files,
    name_prefix: '',
    include_directories : inc_dirs)
