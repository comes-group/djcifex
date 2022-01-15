#include "public/libcifex.h"

#include <string.h>

extern inline cifex_result_t
cifex_errno_result(int err);

extern inline int
cifex_get_errno(cifex_result_t result);

static const char *cx_result_strings[] = {
   [cifex_ok] = "the operation completed successfully",
   [cifex_out_of_memory] = "ran out of memory while performing the operation",
   [cifex_empty_image_file] = "the image file is empty",
   [cifex_syntax_error] = "syntax error",
   [cifex_invalid_version] = "invalid format version (version cannot be zero)",
   [cifex_unsupported_version] = "unsupported format version. is the decoder too old?",
   [cifex_invalid_bpp] = "invalid bits per pixel (must be 24 for RGBA or 32 for RGB)",
   [cifex_channel_out_of_range] = "channel is out of range (must be in 0..255)",
   [cifex_empty_metadata_key] = "metadata key must not be empty",
   [cifex_missing_language] = "no language flag was provided",
   [cifex_number_too_large] = "number was too large to be encoded",
   [cifex_invalid_metadata_key] = "metadata key cannot contain spaces",
   [cifex_invalid_metadata_value] = "metadata key cannot contain line feeds",
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
