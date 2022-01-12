#ifndef LIBCIFEX_COMPILERS_H
#define LIBCIFEX_COMPILERS_H

#ifdef __GNUC__
# define cx_inline __attribute__((always_inline)) inline
#else
# define cx_inline inline
#endif

#endif
