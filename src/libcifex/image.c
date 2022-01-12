#include "public/libcifex.h"

#include "cxensure.h"

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
      cifex_free(image->allocator, (void **)&image->data);
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
   cifex_free(image->allocator, (void **)&image->data);
   image->allocator = NULL;
}

void
cifex_free_image_info(cifex_image_info_t *image_info)
{
   cifex_metadata_pair_t *node = image_info->metadata_last;
   while (node != NULL) {
      cifex_metadata_pair_t *to_free = node;
      node = node->prev;
      cifex_free(image_info->allocator, (void **)&to_free->key);
      cifex_free(image_info->allocator, (void **)&to_free->value);
      cifex_free(image_info->allocator, (void **)&to_free);
   }
   image_info->allocator = NULL;
}
