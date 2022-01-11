libcifex_src = [
   'alloc.c',
   'decode.c',
   'errors.c',
   'image.c',
   'io.c',
]

libcifex = library('cifex', sources: libcifex_src)
libcifex_dependency = declare_dependency(link_with: libcifex, include_directories: 'public')
