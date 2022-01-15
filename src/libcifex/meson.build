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

python = import('python').find_installation('python3')
supports_bytewise = [
   'x86',
   'x86_64',
].contains(host_machine.cpu_family())
strconsts = custom_target(
   'strconsts',
   input: ['strconsts.txt'],
   output: ['cxstrconsts.h'],
   command: [
      python,
      '@CURRENT_SOURCE_DIR@/build_scripts/generate_strconsts.py',
      '@INPUT@', '@OUTPUT@',
      '--bytewise', supports_bytewise.to_string(),
      '--endianness', host_machine.endian(),
   ],
)

libcifex = static_library(
   'cifex', libcifex_src,
   dependencies: [libm],
   c_args: libcifex_c_args,
)
libcifex_dependency = declare_dependency(
   sources: [strconsts],
   link_with: libcifex,
   include_directories: 'public',
)
