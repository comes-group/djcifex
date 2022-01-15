#include "public/libcifex.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cxcompilers.h"
#include "cxensure.h"
#include "cxstrconsts.h"
#include "cxstrings.h"

#define CX_MAX_PATTERN_LEN 32

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

   uint8_t *buffer = cifex_alloc(allocator, (size_t)file_size + CX_MAX_PATTERN_LEN);
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
 if (!(expr)) \
  return cifex_syntax_error;

// Matches a single character.
static cx_inline bool
cx_dec_match(cx_decoder_t *dec, uint8_t byte)
{
   if (dec->buffer[dec->position] != byte) {
      return false;
   }
   ++dec->position;
   return true;
}

// Matches one or more of the provided character.
static cx_inline size_t
cx_dec_match_one_or_more(cx_decoder_t *dec, uint8_t byte)
{
   size_t matched = 0;
   while (cx_dec_match(dec, byte)) {
      ++matched;
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
   size_t line_breaks = cx_dec_match_one_or_more(dec, '\n');
   dec->line += line_breaks;
   return line_breaks > 0;
}

static cx_inline bool
cx_dec_match_strconst__impl(cx_decoder_t *dec, size_t len, bool (*match)(const uint8_t *))
{
   if (match(&dec->buffer[dec->position])) {
      dec->position += len;
      return true;
   }
   return false;
}

#define cx_dec_match_strconst(dec, strconst) \
 cx_dec_match_strconst__impl(dec, cx_sc_##strconst##_len, cx_sc_##strconst##_match)

// Parses a number.
static cx_inline bool
cx_dec_parse_number_up_to_hundreds(cx_decoder_t *dec, uint32_t *out_number)
{
   // NOTE(liquidev): Yes, this function is a giant chain of if-else statements, but there are a few
   // things that are keeping it from becoming less terrible:
   //  - such chains combined with cx_dec_match_string being always_inline, generates the fastest
   //    possible assembly
   //  - C has no sugar for such repeated if-elses so we're largely stuck with this unless somebody
   //    comes (haha) in and creates a code generator for this, which I think is more trouble than
   //    it's worth.
   // Had I used a more modern language like Zig this wouldn't be a problem, but alas, here we are.

   // Check for zero.
   if (cx_dec_match_strconst(dec, zero)) {
      return true;
   }

   // Check for hundreds.
   bool hundreds = true;
   if (cx_dec_match_strconst(dec, one_hundred)) {
      *out_number += 100;
   } else if (cx_dec_match_strconst(dec, two_hundred)) {
      *out_number += 200;
   } else if (cx_dec_match_strconst(dec, three_hundred)) {
      *out_number += 300;
   } else if (cx_dec_match_strconst(dec, four_hundred)) {
      *out_number += 400;
   } else if (cx_dec_match_strconst(dec, five_hundred)) {
      *out_number += 500;
   } else if (cx_dec_match_strconst(dec, six_hundred)) {
      *out_number += 600;
   } else if (cx_dec_match_strconst(dec, seven_hundred)) {
      *out_number += 700;
   } else if (cx_dec_match_strconst(dec, eight_hundred)) {
      *out_number += 800;
   } else if (cx_dec_match_strconst(dec, nine_hundred)) {
      *out_number += 900;
   } else {
      hundreds = false;
   }
   if (hundreds && !cx_dec_match_ws(dec)) {
      return true;
   }

   // Check for ten and n-teens.
   bool nteens = true;
   if (cx_dec_match_strconst(dec, ten)) {
      *out_number += 10;
   } else if (cx_dec_match_strconst(dec, eleven)) {
      *out_number += 11;
   } else if (cx_dec_match_strconst(dec, twelve)) {
      *out_number += 12;
   } else if (cx_dec_match_strconst(dec, thirteen)) {
      *out_number += 13;
   } else if (cx_dec_match_strconst(dec, fourteen)) {
      *out_number += 14;
   } else if (cx_dec_match_strconst(dec, fifteen)) {
      *out_number += 15;
   } else if (cx_dec_match_strconst(dec, sixteen)) {
      *out_number += 16;
   } else if (cx_dec_match_strconst(dec, seventeen)) {
      *out_number += 17;
   } else if (cx_dec_match_strconst(dec, eighteen)) {
      *out_number += 18;
   } else if (cx_dec_match_strconst(dec, nineteen)) {
      *out_number += 19;
   } else {
      nteens = false;
   }
   // If a match was found, there can't be any words afterwards.
   if (nteens) {
      return true;
   }

   // Check for tens.
   bool tens = true;
   if (cx_dec_match_strconst(dec, twenty)) {
      *out_number += 20;
   } else if (cx_dec_match_strconst(dec, thirty)) {
      *out_number += 30;
   } else if (cx_dec_match_strconst(dec, fourty)) {
      *out_number += 40;
   } else if (cx_dec_match_strconst(dec, fifty)) {
      *out_number += 50;
   } else if (cx_dec_match_strconst(dec, sixty)) {
      *out_number += 60;
   } else if (cx_dec_match_strconst(dec, seventy)) {
      *out_number += 70;
   } else if (cx_dec_match_strconst(dec, eighty)) {
      *out_number += 80;
   } else if (cx_dec_match_strconst(dec, ninety)) {
      *out_number += 90;
   } else {
      tens = false;
   }
   if (tens && !cx_dec_match_ws(dec)) {
      return true;
   }

   // Check for ones.
   if (cx_dec_match_strconst(dec, one)) {
      *out_number += 1;
   } else if (cx_dec_match_strconst(dec, two)) {
      *out_number += 2;
   } else if (cx_dec_match_strconst(dec, three)) {
      *out_number += 3;
   } else if (cx_dec_match_strconst(dec, four)) {
      *out_number += 4;
   } else if (cx_dec_match_strconst(dec, five)) {
      *out_number += 5;
   } else if (cx_dec_match_strconst(dec, six)) {
      *out_number += 6;
   } else if (cx_dec_match_strconst(dec, seven)) {
      *out_number += 7;
   } else if (cx_dec_match_strconst(dec, eight)) {
      *out_number += 8;
   } else if (cx_dec_match_strconst(dec, nine)) {
      *out_number += 9;
   }

   return out_number != 0;
}

static bool
cx_dec_parse_number(cx_decoder_t *dec, uint32_t *out_number)
{
   // Use a temporary variable to avoid pointer indirections.
   // The calls to `cx_dec_parse_number_up_to_hundreds` are inlined so the compiler will optimize
   // the pointer away in its IR.
   uint32_t number = 0;
   if (cx_dec_match_strconst(dec, thousand)) {
      number = 1000;
      if (!cx_dec_match_ws(dec)) {
         goto out;
      }
   }
   bool ok = cx_dec_parse_number_up_to_hundreds(dec, &number);
   if (ok && cx_dec_match_ws(dec)) {
      uint32_t ones = number % 10;
      bool thousand = (number == 1 && cx_dec_match_strconst(dec, thousand)) ||
         (ones >= 2 && ones <= 4 && cx_dec_match_strconst(dec, thousands1)) ||
         cx_dec_match_strconst(dec, thousands2);
      if (thousand) {
         number *= 1000;
      }
      uint32_t rest = 0;
      ok = cx_dec_parse_number_up_to_hundreds(dec, &rest);
      if (ok) {
         number += rest;
      }
   }

out:
   *out_number = number;
   return true;
}

// Parses the `CIF: polish` flags.
static cx_inline cifex_result_t
cx_dec_parse_flags(cx_decoder_t *dec, cifex_flags_t *out_flags)
{
   cx_dec_try(cx_dec_match_strconst(dec, k_header));
   cx_dec_try(cx_dec_match_ws(dec));
   // TODO: support for other flags, such as `english`, `compact`, `quadtree`.
   cx_dec_try(cx_dec_match_strconst(dec, flag_polish));
   cx_dec_try(cx_dec_match_lf(dec));
   *out_flags |= cifex_flag_polish;

   return cifex_ok;
}

// Parses the `WERSJA jeden` version header.
static cx_inline cifex_result_t
cx_dec_parse_version(cx_decoder_t *dec, uint32_t *out_version)
{
   cx_dec_try(cx_dec_match_strconst(dec, k_version));
   cx_dec_try(cx_dec_match_ws(dec));
   cx_dec_try(cx_dec_parse_number(dec, out_version));
   cx_dec_try(cx_dec_match_lf(dec));

   return cifex_ok;
}

// Parses the `ROZMIAR` dimensions header and allocates memory in the provided image.
static cx_inline cifex_result_t
cx_dec_parse_dimensions(cx_decoder_t *dec, cifex_image_t *out_image, cifex_allocator_t *allocator)
{
   uint32_t width, height;
   uint32_t bpp;

   cx_dec_try(cx_dec_match_strconst(dec, k_dimensions));
   cx_dec_try(cx_dec_match_ws(dec));

   cx_dec_try(cx_dec_match_strconst(dec, k_width));
   cx_dec_try(cx_dec_match_ws(dec));
   cx_dec_try(cx_dec_parse_number(dec, &width));
   cx_dec_try(cx_dec_match(dec, ','));
   cx_dec_try(cx_dec_match_ws(dec));

   cx_dec_try(cx_dec_match_strconst(dec, k_height));
   cx_dec_try(cx_dec_match_ws(dec));
   cx_dec_try(cx_dec_parse_number(dec, &height));
   cx_dec_try(cx_dec_match(dec, ','));
   cx_dec_try(cx_dec_match_ws(dec));

   cx_dec_try(cx_dec_match_strconst(dec, k_bpp));
   cx_dec_try(cx_dec_match_ws(dec));
   cx_dec_try(cx_dec_parse_number(dec, &bpp));
   cx_dec_try(cx_dec_match_lf(dec));

   cifex_channels_t channels = bpp / 8;
   cifex_result_t result;
   if ((result = cifex_alloc_image(out_image, allocator, width, height, channels)) != cifex_ok) {
      return result;
   }

   return cifex_ok;
}

// Parses a single metadata field.
// This does not perform any allocations, but the output strings are NOT null-terminated.
static cx_inline cifex_result_t
cx_dec_parse_metadata_field(
   cx_decoder_t *dec,
   uint8_t **out_key,
   size_t *out_key_len,
   uint8_t **out_value,
   size_t *out_value_len)
{
   cx_dec_try(cx_dec_match_strconst(dec, k_metadata));
   cx_dec_try(cx_dec_match_ws(dec));

   size_t key_start = dec->position;
   while (dec->position < dec->buffer_len && dec->buffer[dec->position] != ' ') {
      ++dec->position;
   }
   size_t key_end = dec->position;
   cx_dec_try(cx_dec_match_ws(dec));

   size_t value_start = dec->position;
   while (dec->position < dec->buffer_len && dec->buffer[dec->position] != '\n') {
      ++dec->position;
   }
   size_t value_end = dec->position;
   cx_dec_try(cx_dec_match_lf(dec));

   *out_key = &dec->buffer[key_start];
   *out_key_len = key_end - key_start;

   *out_value = &dec->buffer[value_start];
   *out_value_len = value_end - value_start;

   return cifex_ok;
}

// Parses the `METADANE` metadata header and allocates metadata fields.
// The allocator can be `NULL`, in which case no allocations are performed.
static cx_inline cifex_result_t
cx_dec_parse_metadata(
   cx_decoder_t *dec,
   cifex_image_info_t *out_image_info,
   cifex_allocator_t *allocator)
{
   uint8_t *key, *value;
   size_t key_len, value_len;

   if (allocator != NULL) {
      while (cx_dec_parse_metadata_field(dec, &key, &key_len, &value, &value_len) == cifex_ok) {
         // Casting through the signedness here is safe because in the end it's all just characters.
         // I just use `uint8_t` in the decoder because `char`s stink, but that's what string
         // literals are so storing them in metadata that way makes more sense.
         cifex_append_metadata_len(out_image_info, key_len, (char *)key, value_len, (char *)value);
      }
   } else {
      while (cx_dec_parse_metadata_field(dec, &key, &key_len, &value, &value_len) == cifex_ok)
         ;
   }

   return cifex_ok;
}

// Parses all the pixels in an image. The amount of pixels to be parsed is taken from the
// `out_image`.
static cx_inline cifex_result_t
cx_dec_parse_pixels(cx_decoder_t *dec, cifex_image_t *inout_image, size_t *out_error_line)
{
   size_t syntax_error = 0;
   size_t range_error = 0;

   switch (inout_image->channels) {
      case cifex_rgb:
         for (uint32_t y = 0; y < inout_image->height; ++y) {
            for (uint32_t x = 0; x < inout_image->width; ++x) {
               size_t offset =
                  ((size_t)x + (size_t)y * (size_t)inout_image->width) * inout_image->channels;

               // Parse the pixel.
               uint32_t r, g, b;
               bool syntax = false;
               syntax |= !cx_dec_parse_number(dec, &r);
               syntax |= !cx_dec_match(dec, ';');
               syntax |= !cx_dec_match_ws(dec);
               syntax |= !cx_dec_parse_number(dec, &g);
               syntax |= !cx_dec_match(dec, ';');
               syntax |= !cx_dec_match_ws(dec);
               syntax |= !cx_dec_parse_number(dec, &b);
               syntax |= !cx_dec_match_lf(dec);
               if (syntax != 0) {
                  syntax_error = dec->line;
               }

               // Check if all channels are in the correct range.
               if (r > 255 || g > 255 || b > 255) {
                  range_error = dec->line;
               }

               // Set the pixel.
               inout_image->data[offset] = r;
               inout_image->data[offset + 1] = g;
               inout_image->data[offset + 2] = b;
            }
         }
         break;
      case cifex_rgba:
         for (uint32_t y = 0; y < inout_image->height; ++y) {
            for (uint32_t x = 0; x < inout_image->width; ++x) {
               size_t offset =
                  ((size_t)x + (size_t)y * (size_t)inout_image->width) * inout_image->channels;

               // Parse the pixel.
               uint32_t r, g, b, a;
               bool syntax = false;
               syntax |= !cx_dec_parse_number(dec, &r);
               syntax |= !cx_dec_match(dec, ';');
               syntax |= !cx_dec_match_ws(dec);
               syntax |= !cx_dec_parse_number(dec, &g);
               syntax |= !cx_dec_match(dec, ';');
               syntax |= !cx_dec_match_ws(dec);
               syntax |= !cx_dec_parse_number(dec, &b);
               syntax |= !cx_dec_match(dec, ';');
               syntax |= !cx_dec_match_ws(dec);
               syntax |= !cx_dec_parse_number(dec, &a);
               syntax |= !cx_dec_match_lf(dec);
               if (syntax) {
                  syntax_error = dec->line;
               }

               // Check if all channels are in the correct range.
               if (r > 255 || g > 255 || b > 255 || a > 255) {
                  range_error = dec->line;
               }

               // Set the pixel.
               inout_image->data[offset] = r;
               inout_image->data[offset + 1] = g;
               inout_image->data[offset + 2] = b;
               inout_image->data[offset + 3] = a;
            }
         }
         break;
   }

   if (syntax_error != 0) {
      *out_error_line = syntax_error;
      return cifex_syntax_error;
   }
   if (range_error > 0) {
      *out_error_line = range_error;
      return cifex_channel_out_of_range;
   }

   return cifex_ok;
}

// Constructs a decoding error.
static cx_inline cifex_decode_result_t
cx_dec_error(const cx_decoder_t *dec, cifex_result_t result)
{
   return (cifex_decode_result_t){
      .result = result,
      .position = dec->position,
      .line = dec->line,
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

   cifex_result_t result;

   // Reading all the data at once is faster than having to seek around and all that.
   // It also lets us seek throughout the whole file however we see fit.
   uint8_t *buffer = NULL;
   size_t buffer_len = 0;
   if ((result = cx_read_all(config.reader, config.allocator, &buffer, &buffer_len)) != cifex_ok) {
      return (cifex_decode_result_t){ .result = result, .line = 0, .position = 0 };
   }

   cx_decoder_t dec = {
      .buffer = buffer,
      .buffer_len = buffer_len,
      .position = 0,
      .line = 1,
   };

   cifex_image_info_t image_info = {
      .allocator = config.allocator,
      .version = 0,
      .flags = 0,
      .metadata = NULL,
   };

   if ((result = cx_dec_parse_flags(&dec, &image_info.flags)) != cifex_ok) {
      goto err;
   }

   if ((result = cx_dec_parse_version(&dec, &image_info.version)) != cifex_ok) {
      goto err;
   }
   if (image_info.version < 1) {
      result = cifex_invalid_version;
      goto err;
   } else if (image_info.version > CIFEX_FORMAT_VERSION) {
      result = cifex_unsupported_version;
      goto err;
   }

   if ((result = cx_dec_parse_dimensions(&dec, out_image, config.allocator)) != cifex_ok) {
      goto err;
   }

   bool load_metadata = (config.load_metadata && out_image_info != NULL);
   if (
      (result = cx_dec_parse_metadata(
          &dec, &image_info, load_metadata ? config.allocator : NULL)) != cifex_ok) {
      goto err;
   }

   size_t error_line = 0;
   if ((result = cx_dec_parse_pixels(&dec, out_image, &error_line)) != cifex_ok) {
      dec.line = error_line;
      goto err;
   }

   if (out_image_info != NULL) {
      *out_image_info = image_info;
   }

   goto ok;

err:
   cifex_free(config.allocator, dec.buffer);
   return cx_dec_error(&dec, result);

ok:
   cifex_free(config.allocator, dec.buffer);
   return (cifex_decode_result_t){ .result = cifex_ok, .position = 0, .line = 0 };
}
