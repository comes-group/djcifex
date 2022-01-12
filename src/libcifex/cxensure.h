#ifndef CIFEX_ENSURE_H
#define CIFEX_ENSURE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/// `assert` but also lets you pass in an error message.
static inline void
cx_ensure__impl(bool cond, const char *filename, size_t line, const char *message, const char *expr)
{
#ifndef NDEBUG
   if (!cond) {
      fprintf(stderr, "\nassertion failed: %s\n  at %s:%lu: %s\n\n", message, filename, line, expr);
      abort();
   }
#endif
}

#define cx_ensure(cond, message) cx_ensure__impl(cond, __FILE__, __LINE__, message, #cond)

#endif
