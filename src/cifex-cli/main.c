#include "libcifex.h"

#include <stdio.h>
#include <stdlib.h>

static void
cxc_try(cifex_result_t result)
{
   if (result != cifex_ok) {
      fprintf(stderr, "error: %s\n", cifex_result_to_string(result));
      exit(result);
   }
}

int
main(void)
{
   cifex_allocator_t allocator = cifex_libc_allocator();
   cifex_reader_t reader = { 0 };
   cifex_image_t image = { 0 };
   cifex_result_t result = cifex_ok;

   cxc_try(cifex_fopen(&reader, "images/test.cif"));

   cxc_try(cifex_decode(
      &(cifex_decode_config_t){
         .allocator = allocator,
         .reader = reader,
      },
      &image));

   cifex_free_image(&image);
   cifex_fclose(&reader);

   return result;
}
