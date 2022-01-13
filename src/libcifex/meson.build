libcifex_src = [
   'alloc.c',
   'decode.c',
   'errors.c',
   'image.c',
   'io.c',
]

libcifex_c_args = []
if get_option('no_inlining')
   libcifex_c_args += '-DLIBCIFEX_NO_INLINE'
endif

libcifex = static_library('cifex', sources: libcifex_src, c_args: libcifex_c_args)
libcifex_dependency = declare_dependency(link_with: libcifex, include_directories: 'public')
