#include "cxstrings.h"
#include "public/libcifex.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "cxcompilers.h"
#include "cxensure.h"
#include "cxutil.h"

#define CX_BUFFER_SIZE 256

// The encoder state.
typedef struct cx_encoder
{
   cifex_writer_t *writer;
   uint8_t write_buffer[CX_BUFFER_SIZE];
   size_t write_buffer_len;
} cx_encoder_t;

#define cx_enc_try(expr) \
 if ((result = expr) != cifex_ok) \
 return result

// Flushes the encoder's write buffer to the writer.
static cx_inline cifex_result_t
cx_enc_flush(cx_encoder_t *enc)
{
   errno = 0;
   if (
      enc->writer->write(enc->writer, enc->write_buffer, enc->write_buffer_len) <
         enc->write_buffer_len &&
      errno != 0) {
      return cifex_errno_result(errno);
   }
   enc->write_buffer_len = 0;

   return cifex_ok;
}

// Writes a string to the write buffer, flushing the writer if needed.
static cx_inline cifex_result_t
cx_enc_write(cx_encoder_t *enc, size_t len, const char *data)
{
   cx_ensure(len < CX_BUFFER_SIZE, "length of data to be written must not exceed CX_BUFFER_SIZE");

   if (enc->write_buffer_len + len > CX_BUFFER_SIZE) {
      cifex_result_t result;
      cx_enc_try(cx_enc_flush(enc));
   }
   memcpy(&enc->write_buffer[enc->write_buffer_len], data, len);
   enc->write_buffer_len += len;

   return cifex_ok;
}

// This is a version of `cx_enc_write` that writes data to the writer in chunks, which allows for
// writing data of arbitrary length. Because of the loop involved, it's also slower.
static cx_inline cifex_result_t
cx_enc_write_chunks(cx_encoder_t *enc, size_t len, const char *data)
{
   cifex_result_t result;
   cx_enc_try(cx_enc_flush(enc));

   for (size_t i = 0; i < len; i += CX_BUFFER_SIZE) {
      size_t write_len = cx_min(len - i, CX_BUFFER_SIZE);
      memcpy(enc->write_buffer, &data[i], write_len);
      enc->write_buffer_len = write_len;
      cx_enc_try(cx_enc_flush(enc));
   }

   return cifex_ok;
}

// Shortcut for writing a string constant into an encoder.
#define cx_try_write_string(enc, str) cx_enc_try(cx_enc_write(enc, cxstr(str)))

// Writes a number up to the hundreds place, to the given encoder.
static cx_inline cifex_result_t
cx_enc_write_number_up_to_hundreds(cx_encoder_t *enc, uint32_t number)
{
   cifex_result_t result = cifex_ok;

   if (number == 0) {
      cx_try_write_string(enc, "zero");
      return cifex_ok;
   }

   uint32_t hundreds = number % 1000 / 100;
   bool had_hundreds = true;
   switch (hundreds) {
      case 1:
         cx_try_write_string(enc, "sto");
         break;
      case 2:
         cx_try_write_string(enc, "dwieście");
         break;
      case 3:
         cx_try_write_string(enc, "trzysta");
         break;
      case 4:
         cx_try_write_string(enc, "czterysta");
         break;
      case 5:
         cx_try_write_string(enc, "pięćset");
         break;
      case 6:
         cx_try_write_string(enc, "sześćset");
         break;
      case 7:
         cx_try_write_string(enc, "siedemset");
         break;
      case 8:
         cx_try_write_string(enc, "osiemset");
         break;
      case 9:
         cx_try_write_string(enc, "dziewięćset");
         break;
      default:
         had_hundreds = false;
         break;
   }

   uint32_t ones_and_tens = number % 100;
   if (ones_and_tens >= 10 && ones_and_tens <= 19 && had_hundreds) {
      cx_try_write_string(enc, " ");
   }

   switch (ones_and_tens) {
      case 10:
         cx_try_write_string(enc, "dziesięć");
         return result;
      case 11:
         cx_try_write_string(enc, "jedenaście");
         return result;
      case 12:
         cx_try_write_string(enc, "dwanaście");
         return result;
      case 13:
         cx_try_write_string(enc, "trzynaście");
         return result;
      case 14:
         cx_try_write_string(enc, "czternaście");
         return result;
      case 15:
         cx_try_write_string(enc, "piętnaście");
         return result;
      case 16:
         cx_try_write_string(enc, "szesnaście");
         return result;
      case 17:
         cx_try_write_string(enc, "siedemnaście");
         return result;
      case 18:
         cx_try_write_string(enc, "osiemnaście");
         return result;
      case 19:
         cx_try_write_string(enc, "dziewiętnaście");
         return result;
   }

   uint32_t tens = (number % 100) / 10;
   if (tens > 0 && had_hundreds) {
      cx_try_write_string(enc, " ");
      had_hundreds = false;
   }

   bool had_tens = true;
   switch (tens) {
      case 1:
         cx_try_write_string(enc, "dziesięć");
         break;
      case 2:
         cx_try_write_string(enc, "dwadzieścia");
         break;
      case 3:
         cx_try_write_string(enc, "trzydzieści");
         break;
      case 4:
         cx_try_write_string(enc, "czterdzieści");
         break;
      case 5:
         cx_try_write_string(enc, "pięćdziesiąt");
         break;
      case 6:
         cx_try_write_string(enc, "sześćdziesiąt");
         break;
      case 7:
         cx_try_write_string(enc, "siedemdziesiąt");
         break;
      case 8:
         cx_try_write_string(enc, "osiemdziesiąt");
         break;
      case 9:
         cx_try_write_string(enc, "dziewięćdziesiąt");
         break;
      default:
         had_tens = false;
         break;
   }

   uint32_t ones = number % 10;
   if (ones > 0 && (had_tens || had_hundreds)) {
      cx_try_write_string(enc, " ");
   }

   switch (ones) {
      case 1:
         cx_try_write_string(enc, "jeden");
         break;
      case 2:
         cx_try_write_string(enc, "dwa");
         break;
      case 3:
         cx_try_write_string(enc, "trzy");
         break;
      case 4:
         cx_try_write_string(enc, "cztery");
         break;
      case 5:
         cx_try_write_string(enc, "pięć");
         break;
      case 6:
         cx_try_write_string(enc, "sześć");
         break;
      case 7:
         cx_try_write_string(enc, "siedem");
         break;
      case 8:
         cx_try_write_string(enc, "osiem");
         break;
      case 9:
         cx_try_write_string(enc, "dziewięć");
         break;
   }

   return cifex_ok;
}

// Writes an arbitrary number into the encoder.
static cifex_result_t
cx_enc_write_number(cx_encoder_t *enc, uint32_t number)
{
   if (number > 999999) {
      return cifex_number_too_large;
   }

   cifex_result_t result;

   uint32_t hundreds_of_thousands = number / 1000;
   bool had_thousands = true;
   if (hundreds_of_thousands == 1) {
      cx_try_write_string(enc, "tysiąc");
   } else if (hundreds_of_thousands > 1) {
      uint32_t thousands = hundreds_of_thousands % 10;
      cx_enc_try(cx_enc_write_number_up_to_hundreds(enc, hundreds_of_thousands));
      if (thousands >= 2 && thousands <= 4) {
         cx_try_write_string(enc, " tysiące");
      } else {
         cx_try_write_string(enc, " tysięcy");
      }
   } else {
      had_thousands = false;
   }

   uint32_t hundreds = number % 1000;
   if (hundreds > 0 || (hundreds_of_thousands == 0 && hundreds == 0)) {
      if (had_thousands) {
         cx_try_write_string(enc, " ");
      }
      cx_enc_try(cx_enc_write_number_up_to_hundreds(enc, hundreds));
   }

   return cifex_ok;
}

// Encodes the `CIF: polish` flags.
static cx_inline cifex_result_t
cx_enc_dump_flags(cx_encoder_t *enc, cifex_flags_t flags)
{
   cifex_result_t result;
   if ((flags & cifex_flag_polish) == 0) {
      return cifex_missing_language;
   }
   cx_try_write_string(enc, "CIF: polish\n");

   return cifex_ok;
}

// Encodes the `WERSJA` version header.
static cx_inline cifex_result_t
cx_enc_dump_version(cx_encoder_t *enc, uint32_t version)
{
   cifex_result_t result;

   cx_try_write_string(enc, "WERSJA ");
   cx_enc_try(cx_enc_write_number(enc, version));
   cx_try_write_string(enc, "\n");

   return cifex_ok;
}

// Encodes the `ROZMIAR` dimensions header.
static cx_inline cifex_result_t
cx_enc_dump_dimensions(cx_encoder_t *enc, const cifex_image_t *image)
{
   cifex_result_t result;

   cx_try_write_string(enc, "ROZMIAR szerokość: ");
   cx_enc_try(cx_enc_write_number(enc, image->width));
   cx_try_write_string(enc, ", wysokość: ");
   cx_enc_try(cx_enc_write_number(enc, image->height));
   cx_try_write_string(enc, ", bitów_na_piksel: ");
   cx_enc_try(cx_enc_write_number(enc, image->channels * 8));
   cx_try_write_string(enc, "\n");

   return cifex_ok;
}

// Checks if the provided cxstr is a valid metadata key (does not contain spaces).
static cx_inline cifex_result_t
cx_check_valid_metadata_key(size_t len, const char *key)
{
   for (size_t i = 0; i < len; ++i) {
      if (key[i] == ' ') {
         return cifex_invalid_metadata_key;
      }
   }
   return cifex_ok;
}

// Checks is the provided cxstr is a valid metadata value (does not contain line feeds).
static cx_inline cifex_result_t
cx_check_valid_metadata_value(size_t len, const char *value)
{
   for (size_t i = 0; i < len; ++i) {
      if (value[i] == '\n') {
         return cifex_invalid_metadata_key;
      }
   }
   return cifex_ok;
}

// Encodes the `METADANE` metadata pairs.
static cx_inline cifex_result_t
cx_enc_dump_metadata(cx_encoder_t *enc, const cifex_metadata_pair_t *first)
{
   cifex_result_t result;

   for (const cifex_metadata_pair_t *pair = first; pair != NULL; pair = pair->next) {
      cx_enc_try(cx_check_valid_metadata_key(pair->key_len, pair->key));
      cx_enc_try(cx_check_valid_metadata_value(pair->value_len, pair->value));

      cx_try_write_string(enc, "METADANE ");
      cx_enc_try(cx_enc_write_chunks(enc, pair->key_len, pair->key));
      cx_try_write_string(enc, " ");
      cx_enc_try(cx_enc_write_chunks(enc, pair->value_len, pair->value));
      cx_try_write_string(enc, "\n");
   }

   return cifex_ok;
}

// Encodes the pixel data.
static cx_inline cifex_result_t
cx_enc_dump_pixels(cx_encoder_t *enc, const cifex_image_t *image)
{
   cifex_result_t result = cifex_ok;
   cifex_result_t expr_result;

#define cx_try_branchless(expr) \
 if ((expr_result = (expr))) \
  result = expr_result;

   switch (image->channels) {
      case cifex_rgb:
         for (uint32_t y = 0; y < image->height; ++y) {
            for (uint32_t x = 0; x < image->width; ++x) {
               size_t offset =
                  ((size_t)x + ((size_t)y * (size_t)image->width)) * (size_t)image->channels;
               cx_try_branchless(cx_enc_write_number(enc, image->data[offset]));
               cx_try_branchless(cx_enc_write(enc, cxstr("; ")));
               cx_try_branchless(cx_enc_write_number(enc, image->data[offset + 1]));
               cx_try_branchless(cx_enc_write(enc, cxstr("; ")));
               cx_try_branchless(cx_enc_write_number(enc, image->data[offset + 2]));
               cx_try_branchless(cx_enc_write(enc, cxstr("\n")));
            }
         }
         break;
      case cifex_rgba:
         for (uint32_t y = 0; y < image->height; ++y) {
            for (uint32_t x = 0; x < image->width; ++x) {
               size_t offset =
                  ((size_t)x + ((size_t)y * (size_t)image->width)) * (size_t)image->channels;
               cx_try_branchless(cx_enc_write_number(enc, image->data[offset]));
               cx_try_branchless(cx_enc_write(enc, cxstr("; ")));
               cx_try_branchless(cx_enc_write_number(enc, image->data[offset + 1]));
               cx_try_branchless(cx_enc_write(enc, cxstr("; ")));
               cx_try_branchless(cx_enc_write_number(enc, image->data[offset + 2]));
               cx_try_branchless(cx_enc_write(enc, cxstr("; ")));
               cx_try_branchless(cx_enc_write_number(enc, image->data[offset + 3]));
               cx_try_branchless(cx_enc_write(enc, cxstr("\n")));
            }
         }
         break;
   }

#undef cx_try_branchless

   return result;
}

// Not too happy about this not being const. But it's not like it's public interface anyways,
// so who cares.
//
// I know I don't.
static cifex_metadata_pair_t cx_encoder_pair = {
   .key = "encoder",
   .key_len = 7,
   .value = "DJ Cifex",
   .value_len = 8,
   .next = NULL,
   .prev = NULL,
};

static const cifex_image_info_t cx_default_image_info = {
   .allocator = NULL,
   .version = CIFEX_FORMAT_VERSION,
   .flags = cifex_flag_polish,

   .metadata = &cx_encoder_pair,
   .metadata_last = &cx_encoder_pair,
};

cifex_result_t
cifex_encode(
   cifex_writer_t *writer,
   const cifex_image_t *image,
   const cifex_image_info_t *image_info)
{
   cx_ensure(writer != NULL, "writer cannot be NULL");
   cx_ensure(image != NULL, "input image cannot be NULL");

   if (image_info == NULL) {
      image_info = &cx_default_image_info;
   }

   cx_encoder_t enc = {
      .writer = writer,
      .write_buffer = { 0 },
      .write_buffer_len = 0,
   };

   cifex_result_t result = cifex_ok;
   cx_enc_try(cx_enc_dump_flags(&enc, image_info->flags));
   cx_enc_try(cx_enc_dump_version(&enc, image_info->version));
   cx_enc_try(cx_enc_dump_dimensions(&enc, image));
   cx_enc_try(cx_enc_dump_metadata(&enc, image_info->metadata));
   cx_enc_try(cx_enc_dump_pixels(&enc, image));

   cx_enc_flush(&enc);

   return cifex_ok;
}
