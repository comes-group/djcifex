#include "libcifex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendor/stb_image_write.h"

#include "argparse.c"

static void
cxc_try(cifex_result_t result)
{
   if (result != cifex_ok) {
      fprintf(stderr, "error: %s\n", cifex_result_to_string(result));
      exit(result);
   }
}

typedef struct cxc_decode_config
{
   const char *input_file_name, *output_file_name;
   bool dry_run;
} cxc_decode_config_t;

static cifex_result_t
cxc_decode(cxc_decode_config_t c)
{
   cifex_allocator_t allocator = cifex_libc_allocator();
   cifex_reader_t reader = { 0 };
   cifex_image_t image = { 0 };
   cifex_image_info_t image_info = { 0 };
   cifex_result_t result = cifex_ok;

   if (c.input_file_name == NULL || c.output_file_name == NULL) {
      fprintf(
         stderr,
         "error: no input or output filename provided.\n"
         "usage: cifex decode <input-file.cif> <output-file.png>\n");
      exit(-1);
   }

   cxc_try(cifex_fopen_read(&reader, c.input_file_name));

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

   if (!c.dry_run) {
      stbi_write_png(
         c.output_file_name,
         image.width,
         image.height,
         image.channels,
         image.data,
         image.width * image.channels);
   }

   cifex_free_image(&image);
   cifex_free_image_info(&image_info);
   cifex_fclose_read(&reader);

   return result;
}

typedef struct cxc_encode_config
{
   const char *input_file_name, *output_file_name;
} cxc_encode_config_t;

static cifex_result_t
cxc_encode(cxc_encode_config_t c)
{
   cifex_writer_t writer = { 0 };
   cifex_result_t result = cifex_ok;
   cifex_image_t image = { 0 };

   if (c.input_file_name == NULL || c.output_file_name == NULL) {
      fprintf(
         stderr,
         "error: no input or output filename provided.\n"
         "usage: cifex encode <input-file.png> <output-file.cif>\n");
      exit(-1);
   }

   int image_width, image_height, image_channels;
   image.data = stbi_load(c.input_file_name, &image_width, &image_height, &image_channels, 0);
   if (image.data == NULL) {
      fprintf(stderr, "error: could not load image\n");
      return cifex_out_of_memory;
   }
   if (image_channels < 3) {
      fprintf(stderr, "error: non-RGB(A) images are not yet supported.\n");
      exit(-2);
   }
   image.width = image_width;
   image.height = image_height;
   image.channels = image_channels;

   cxc_try(cifex_fopen_write(&writer, c.output_file_name));
   cxc_try(cifex_encode(&writer, &image, NULL));

   stbi_image_free(image.data);
   cifex_fclose_write(&writer);

   return result;
}

typedef enum cxc_mode
{
   cxc_mode_decode,
   cxc_mode_encode,
} cxc_mode_t;

int
main(int argc, char *argv[])
{
   cxc_arg_parser_t argp = cxc_init_arg_parser(argc, argv);
   char *mode_str = NULL, *input_file_name = NULL, *output_file_name = NULL;
   bool dry_run = false;

   char **positional_args[] = {
      &mode_str,
      &input_file_name,
      &output_file_name,
   };
   while (cxc_has_args(&argp)) {
      cxc_positional_args(
         &argp, sizeof(positional_args) / sizeof(positional_args[0]), positional_args);
      cxc_named_arg(&argp, 0, "dry-run", cxc_bool, &dry_run);
      cxc_finish_arg(&argp);
   }
   cxc_free_arg_parser(&argp);

   if (mode_str == NULL) {
      fprintf(
         stderr,
         "error: no mode provided.\n"
         "usage: cifex decode|encode\n");
      exit(-1);
   }

   cxc_mode_t mode;
   if (strcmp(mode_str, "decode") == 0) {
      mode = cxc_mode_decode;
   } else if (strcmp(mode_str, "encode") == 0) {
      mode = cxc_mode_encode;
   } else {
      fprintf(
         stderr,
         "error: invalid mode: %s\n"
         "usage: cifex {decode,encode} <arguments...>\n",
         mode_str);
      exit(-1);
   }

   switch (mode) {
      case cxc_mode_decode:
         return cxc_decode((cxc_decode_config_t){
            .input_file_name = input_file_name,
            .output_file_name = output_file_name,
            .dry_run = dry_run,
         });
      case cxc_mode_encode:
         return cxc_encode((cxc_encode_config_t){
            .input_file_name = input_file_name,
            .output_file_name = output_file_name,
         });
   }
}
