#include "public/libcifex.h"

static const char *result_strings[] = {
   [cifex_ok] = "the operation completed successfully",
   [cifex_out_of_memory] = "ran out of memory while performing the operation",
};

static const char *invalid_result = "<invalid result value>";

const char *
cifex_result_to_string(cifex_result_t result)
{
   if (result < cifex__last_result) {
      return result_strings[result];
   } else {
      return invalid_result;
   }
}
