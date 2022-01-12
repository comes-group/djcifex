#ifndef LIBCIFEX_COMPILERS_H
#define LIBCIFEX_COMPILERS_H

#ifndef LIBCIFEX_NO_INLINE
# ifdef __GNUC__
#  define cx_inline __attribute__((always_inline)) inline
# endif
#endif

#ifndef cx_inline
# define cx_inline inline
#endif

#endif
