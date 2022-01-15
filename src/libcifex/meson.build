libcifex_src = [
   'alloc.c',
   'decode.c',
   'encode.c',
   'errors.c',
   'image.c',
   'io.c',
]

libcifex_c_args = []
if get_option('no_inlining')
   libcifex_c_args += '-DLIBCIFEX_NO_INLINE'
endif

cc = meson.get_compiler('c')
libm = cc.find_library('m', required: false)

libcifex = static_library(
   'cifex', libcifex_src,
   dependencies: [libm],
   c_args: libcifex_c_args,
)
libcifex_dependency = declare_dependency(link_with: libcifex, include_directories: 'public')
