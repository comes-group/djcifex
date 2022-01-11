#include "libcifex.h"

#include <stdio.h>

int
main(void)
{
   cifex_allocator_t allocator = cifex_libc_allocator();
   cifex_reader_t reader = cifex_fopen("images/test.cif");
   cifex_image_t image = { 0 };
   cifex_result_t result = cifex_ok;

   result = cifex_decode(
      &(cifex_decode_config_t){
         .allocator = allocator,
         .reader = reader,
      },
      &image);
   if (result != cifex_ok) {
      fprintf(stderr, "error: %s\n", cifex_result_to_string(result));
      return result;
   }

   cifex_free_image(&image);
   cifex_fclose(&reader);

   return result;
}
