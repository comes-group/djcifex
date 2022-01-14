#include "cxstrings.h"
#include "public/libcifex.h"

#include <string.h>

#include "cxcompilers.h"
#include "cxensure.h"
#include "cxutil.h"

#define CX_BUFFER_SIZE 256

typedef struct cx_encoder
{
   cifex_writer_t *writer;
   uint8_t write_buffer[CX_BUFFER_SIZE];
   size_t write_buffer_len;
} cx_encoder_t;

static cx_inline cifex_result_t
cx_enc_flush(cx_encoder_t *enc)
{
   cifex_result_t result;
   if (
      (result = enc->writer->write(enc->writer, enc->write_buffer, enc->write_buffer_len)) !=
      cifex_ok) {
      return result;
   }
   enc->write_buffer_len = 0;

   return cifex_ok;
}

static cx_inline cifex_result_t
cx_enc_write(cx_encoder_t *enc, size_t len, const char *data)
{
   cx_ensure(len < CX_BUFFER_SIZE, "length of data to be written must not exceed CX_BUFFER_SIZE");

   if (enc->write_buffer_len + len > CX_BUFFER_SIZE) {
      cx_enc_flush(enc);
   }
   memcpy(&enc->write_buffer[enc->write_buffer_len], data, len);
   enc->write_buffer_len += len;

   return cifex_ok;
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

// Encodes the `CIF: polish` flags.
cifex_result_t
cx_enc_dump_flags(cx_encoder_t *enc, cifex_flags_t flags)
{
   if ((flags & cifex_flag_polish) == 0) {
      return cifex_missing_language;
   }
   cx_enc_write(enc, cxstr("CIF: polish\n"));

   return cifex_ok;
}

cifex_result_t
cifex_encode_version(cx_encoder_t *enc, uint32_t version);

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
   if ((result = cx_enc_dump_flags(&enc, image_info->flags))) {
      return result;
   }

   return cifex_ok;
}
