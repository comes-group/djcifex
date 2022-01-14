#include "public/libcifex.h"

#include "cxensure.h"
#include <string.h>

extern inline size_t
cifex_image_storage_size(uint32_t width, uint32_t height, cifex_channels_t channels);

cifex_result_t
cifex_alloc_image(
   cifex_image_t *image,
   cifex_allocator_t *allocator,
   uint32_t width,
   uint32_t height,
   cifex_channels_t channels)
{
   cx_ensure(image != NULL, "target image must not be NULL");
   cx_ensure(allocator != NULL, "allocator must not be NULL");

   size_t new_storage_size = cifex_image_storage_size(width, height, channels);
   if (new_storage_size == 0) {
      cifex_free_image(image);
      return cifex_ok;
   }

   size_t old_storage_size = cifex_image_storage_size(image->width, image->height, image->channels);
   if (new_storage_size > old_storage_size) {
      cifex_free(image->allocator, image->data);
      uint8_t *data = cifex_alloc(allocator, new_storage_size);
      if (data == NULL) {
         cifex_free_image(image);
         return cifex_out_of_memory;
      }
      image->allocator = allocator;
      image->data = data;
   }
   image->width = width;
   image->height = height;
   image->channels = channels;

   return cifex_ok;
}

void
cifex_free_image(cifex_image_t *image)
{
   cx_ensure(image != NULL, "image must not be NULL");

   image->width = 0;
   image->height = 0;
   image->channels = 0;
   cifex_free(image->allocator, image->data);
   image->data = NULL;
   image->allocator = NULL;
}

void
cifex_init_image_info(cifex_image_info_t *image_info, cifex_allocator_t *allocator)
{
   cx_ensure(image_info != NULL, "image info must not be NULL");

   image_info->allocator = allocator;
   image_info->version = CIFEX_FORMAT_VERSION;
   image_info->flags = cifex_flag_polish;
   image_info->metadata = NULL;
   image_info->metadata_last = NULL;
}

cifex_result_t
cifex_append_metadata_len(
   cifex_image_info_t *image_info,
   size_t key_len,
   const char *key,
   size_t value_len,
   const char *value)
{
   cx_ensure(image_info != NULL, "image info must not be NULL");
   cx_ensure(image_info->allocator, "to append metadata, the image info must have an allocator");
   cx_ensure(key != NULL, "metadata key must not be NULL");
   cx_ensure(value != NULL, "metadata value must not be NULL");

   // Ensure the key is not empty.
   // Note that the value *can* be empty.
   // These limitations stem from the format itself. The key is delimited from the value using a
   // space, so it must not be empty.
   // This is a soft error because metadata keys are potential user input.
   if (key_len == 0) {
      return cifex_empty_metadata_key;
   }

   // Allocate an extra byte for terminating NUL.
   char *key_buffer = cifex_alloc(image_info->allocator, key_len + 1);
   if (key_buffer == NULL) {
      return cifex_out_of_memory;
   }
   memcpy(key_buffer, key, key_len);

   char *value_buffer = cifex_alloc(image_info->allocator, value_len + 1);
   if (value_buffer == NULL) {
      return cifex_out_of_memory;
   }
   memcpy(value_buffer, value, value_len);

   cifex_metadata_pair_t *node = cifex_alloc(image_info->allocator, sizeof(cifex_metadata_pair_t));
   node->key = key_buffer;
   node->key_len = key_len;
   node->value = value_buffer;
   node->value_len = value_len;
   node->next = NULL;
   node->prev = image_info->metadata_last;
   if (image_info->metadata == NULL) {
      image_info->metadata = node;
      image_info->metadata_last = node;
   } else {
      image_info->metadata_last->next = node;
      image_info->metadata_last = node;
   }

   return cifex_ok;
}

cifex_result_t
cifex_append_metadata(cifex_image_info_t *image_info, const char *key, const char *value)
{
   cx_ensure(key != NULL, "metadata key must not be NULL");
   cx_ensure(value != NULL, "metadata value must not be NULL");

   size_t key_len = strlen(key), value_len = strlen(value);
   return cifex_append_metadata_len(image_info, key_len, key, value_len, value);
}

void
cifex_free_image_info(cifex_image_info_t *image_info)
{
   cifex_metadata_pair_t *node = image_info->metadata_last;
   while (node != NULL) {
      cifex_metadata_pair_t *to_free = node;
      node = node->prev;
      cifex_free(image_info->allocator, to_free->key);
      cifex_free(image_info->allocator, to_free->value);
      cifex_free(image_info->allocator, to_free);
   }
   image_info->allocator = NULL;
}
