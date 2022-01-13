#include "libcifex.h"

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendor/stb_image_write.h"

static void
cxc_try(cifex_result_t result)
{
   if (result != cifex_ok) {
      fprintf(stderr, "error: %s\n", cifex_result_to_string(result));
      exit(result);
   }
}

int
main(int argc, char *argv[])
{
   cifex_allocator_t allocator = cifex_libc_allocator();
   cifex_reader_t reader = { 0 };
   cifex_image_t image = { 0 };
   cifex_image_info_t image_info = { 0 };
   cifex_result_t result = cifex_ok;

   if (argc != 3) {
      fprintf(stderr, "usage: cifex <input.cif> <output>\n");
      return -1;
   }

   char *input_file_name = argv[1];
   char *output_file_name = argv[2];

   cxc_try(cifex_fopen(&reader, input_file_name));

   cifex_decode_result_t decode_result = cifex_decode(
      (cifex_decode_config_t){
         .allocator = &allocator,
         .reader = &reader,
         .load_metadata = true,
      },
      &image,
      &image_info);
   if (decode_result.result != cifex_ok) {
      fprintf(
         stderr,
         "line %lu (byte %lu): %s\n",
         decode_result.line,
         decode_result.position,
         cifex_result_to_string(decode_result.result));
      exit(cifex_syntax_error);
   }

   printf("format version: %i\n", image_info.version);
   printf("dimensions: %u×%u\tchannels: %u\n", image.width, image.height, image.channels);

   printf("metadata:\n");
   cifex_metadata_pair_t *pair = image_info.metadata;
   for (; pair != NULL; pair = pair->next) {
      printf("%s\t | %s\n", pair->key, pair->value);
   }

   printf("writing PNG to %s\n", output_file_name);
   stbi_write_png(
      output_file_name,
      image.width,
      image.height,
      image.channels,
      image.data,
      image.width * image.channels);

   cifex_free_image(&image);
   cifex_free_image_info(&image_info);
   cifex_fclose(&reader);

   return result;
}
