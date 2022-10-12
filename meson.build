# USF library less template more compact version meson project
usf_inc = [ include_directories('include') ]

usf_src = [
    'src/integer.cpp',
    'src/float.cpp',
    'src/arg_format.cpp',
    'src/argument.cpp',
    'src/main.cpp',
    'src/error.cpp'
]


usf_lib = static_library( 'usf', sources: usf_src, 
    include_directories: usf_inc 
)

usf_lib_native = static_library( 'usf_native', sources: usf_src, 
    include_directories: usf_inc, native: true
)

usf_dep = declare_dependency( link_with: usf_lib , include_directories: usf_inc)
usf_native_dep = declare_dependency( link_with: usf_lib_native , include_directories: usf_inc)