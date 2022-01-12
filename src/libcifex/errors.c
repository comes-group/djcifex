#include "public/libcifex.h"

#include <string.h>

extern inline cifex_result_t
cifex_errno_result(int err);

extern inline int
cifex_get_errno(cifex_result_t result);

static const char *cx_result_strings[] = {
   [cifex_ok] = "the operation completed successfully",
   [cifex_out_of_memory] = "ran out of memory while performing the operation",
};

static const char *cx_invalid_result = "<invalid result value>";

const char *
cifex_result_to_string(cifex_result_t result)
{
   if (result < cifex__last_own_result) {
      return cx_result_strings[result];
   } else if (result >= cifex_errno) {
      return strerror(cifex_get_errno(result));
   } else {
      return cx_invalid_result;
   }
}
