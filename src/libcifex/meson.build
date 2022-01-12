libcifex_src = [
   'alloc.c',
   'decode.c',
   'errors.c',
   'image.c',
   'io.c',
]

libcifex = library('cifex', sources: libcifex_src, c_args: '-DLIBCIFEX_NO_INLINE')
libcifex_dependency = declare_dependency(link_with: libcifex, include_directories: 'public')
