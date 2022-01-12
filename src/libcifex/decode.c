#include "public/libcifex.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

#include "cxcompilers.h"
#include "cxensure.h"
#include "cxstrings.h"

// Reads all the data from the reader, and allocates it into a buffer.
static cifex_result_t
cx_read_all(
   cifex_reader_t *reader,
   cifex_allocator_t *allocator,
   uint8_t **out_buffer_ptr,
   size_t *out_buffer_len)
{
   long file_size;
   if (reader->seek(reader, 0, SEEK_END) != 0) {
      return cifex_errno_result(errno);
   }
   if ((file_size = reader->tell(reader)) < 0) {
      return cifex_errno_result(errno);
   }
   if (reader->seek(reader, 0, SEEK_SET) != 0) {
      return cifex_errno_result(errno);
   }

   uint8_t *buffer = cifex_alloc(allocator, (size_t)file_size + 1);
   if (buffer == NULL) {
      return cifex_out_of_memory;
   }

   errno = 0;
   if (reader->read(reader, buffer, (size_t)file_size) < file_size && errno != 0) {
      return cifex_errno_result(errno);
   }

   *out_buffer_ptr = buffer;
   *out_buffer_len = file_size;

   return cifex_ok;
}

// The decoder state.
typedef struct cx_decoder
{
   uint8_t *buffer;
   size_t buffer_len;
   size_t position;
   size_t line;
} cx_decoder_t;

#define cx_dec_try(expr) \
 if (!(expr)) { \
  return cifex_syntax_error; \
 }

// Matches a literal string.
static cx_inline bool
cx_dec_match_string(cx_decoder_t *dec, size_t string_len, const char *string)
{
   if (dec->position + string_len >= dec->buffer_len) {
      return false;
   }
   for (size_t i = 0; i < string_len; ++i) {
      if (dec->buffer[dec->position + i] != string[i]) {
         return false;
      }
   }
   dec->position += string_len;
   return true;
}

// Matches a single character.
static cx_inline bool
cx_dec_match(cx_decoder_t *dec, uint8_t byte)
{
   if (dec->position + 1 >= dec->buffer_len) {
      return false;
   }
   if (dec->buffer[dec->position] != byte) {
      return false;
   }
   ++dec->position;
   return true;
}

// Matches one or more of the provided character.
static cx_inline bool
cx_dec_match_one_or_more(cx_decoder_t *dec, uint8_t byte)
{
   bool matched = false;
   while (cx_dec_match(dec, byte)) {
      ++dec->line;
      matched = true;
   }
   return matched;
}

// Matches white space.
static cx_inline bool
cx_dec_match_ws(cx_decoder_t *dec)
{
   return cx_dec_match_one_or_more(dec, ' ');
}

// Matches a line feed.
static cx_inline bool
cx_dec_match_lf(cx_decoder_t *dec)
{
   return cx_dec_match_one_or_more(dec, '\n');
}

// Parses the `CIF: polish` flags.
static cx_inline cifex_result_t
cx_dec_parse_flags(cx_decoder_t *dec, cifex_flags_t *flags)
{
   cx_dec_try(cx_dec_match_string(dec, cxstr("CIF:")));
   cx_dec_try(cx_dec_match_ws(dec));
   // TODO: support for additional flags.
   cx_dec_try(cx_dec_match_string(dec, cxstr("polish")));
   cx_dec_try(cx_dec_match_lf(dec));
   *flags |= cifex_flag_polish;
   return cifex_ok;
}

// Constructs a decoding error.
static cx_inline cifex_decode_result_t
cx_dec_error(cx_decoder_t *dec, cifex_result_t result)
{
   return (cifex_decode_result_t){
      .line = dec->line,
      .result = result,
   };
}

cifex_decode_result_t
cifex_decode(
   cifex_decode_config_t config,
   cifex_image_t *out_image,
   cifex_image_info_t *out_image_info)
{
   cx_ensure(config.allocator != NULL, "decoding allocator cannot be NULL");
   cx_ensure(config.reader != NULL, "decoding reader cannot be NULL");
   cx_ensure(out_image != NULL, "output image cannot be NULL");

   // Reading all the data at once is faster than having to seek around and all that.
   // It also lets us seek throughout the whole file however we see fit.
   uint8_t *buffer;
   size_t buffer_len;
   cx_read_all(config.reader, config.allocator, &buffer, &buffer_len);

   cx_decoder_t dec = {
      .buffer = buffer,
      .buffer_len = buffer_len,
      .position = 0,
      .line = 0,
   };

   cifex_image_info_t image_info = {
      .flags = 0,
      .metadata = NULL,
   };

   cifex_result_t result;
   if ((result = cx_dec_parse_flags(&dec, &image_info.flags)) != cifex_ok) {
      return cx_dec_error(&dec, result);
   }

   if (out_image_info != NULL) {
      *out_image_info = image_info;
   }

   return (cifex_decode_result_t){ .result = cifex_ok, .line = 0 };
}
