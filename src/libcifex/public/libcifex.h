#ifndef LIBCIFEX_H
#define LIBCIFEX_H

#include <stdint.h>
#include <stdlib.h>

/* --------------
   Error handling
   -------------- */

typedef enum cifex_result
{
   cifex_ok,
   /// Ran out of memory while performing the operation.
   cifex_out_of_memory,

   cifex__last_own_result,

   /// The first `errno` value. Results can be compared against this to check whether they are
   /// `errno`s or not.
   cifex_errno = 0x8000,
} cifex_result_t;

/// Creates a result from an `errno` value.
inline cifex_result_t
cifex_errno_result(int err)
{
   return cifex_errno | err;
}

/// Extracts an `errno` from a result.
inline int
cifex_get_errno(cifex_result_t result)
{
   return result & ~cifex_errno;
}

/// Converts a `cifex_result_t` to a string.
const char *
cifex_result_to_string(cifex_result_t result);

/* ----------
   Allocation
   ---------- */

struct cifex_allocator;

typedef void *(*cifex_malloc_fn)(struct cifex_allocator *allocator, size_t size);

typedef void (*cifex_free_fn)(struct cifex_allocator *allocator, void *ptr);

/// An allocator.
typedef struct cifex_allocator
{
   cifex_malloc_fn malloc;
   cifex_free_fn free;
} cifex_allocator_t;

/// Allocates a memory region using the allocator. Returns `NULL` if no more memory is available.
void *
cifex_alloc(cifex_allocator_t *allocator, size_t size);

/// Deallocates a memory region using the allocator. `ptr` must point to a variable that holds a
/// pointer to the allocation. This variable is `NULL`ified by this function.
void
cifex_free(cifex_allocator_t *allocator, void **varptr);

/// Returns the libc allocator.
cifex_allocator_t
cifex_libc_allocator();

/* --------------
   I/O facilities
   -------------- */

struct cifex_reader;

typedef size_t (*cifex_fread_fn)(struct cifex_reader *reader, void *out, size_t n_bytes);

typedef int (*cifex_fseek_fn)(struct cifex_reader *reader, long offset, int whence);

typedef long (*cifex_ftell_fn)(struct cifex_reader *reader);

/// A file reader.
///
/// The functions in this reader are expected to exhibit behavior similar to that of libc functions,
/// that is, they shoukd use errno and sentinel values for error handling.
typedef struct cifex_reader
{
   void *user_data;
   cifex_fread_fn read;
   cifex_fseek_fn seek;
   cifex_ftell_fn tell;
} cifex_reader_t;

/// `fopen`s a file reader.
cifex_result_t
cifex_fopen(cifex_reader_t *reader, const char *filename);

/// `fclose`s a file reader. This must only be used on readers opened with `cifex_reader_fopen`.
cifex_result_t
cifex_fclose(cifex_reader_t *reader);

/* --------------
   Image handling
   -------------- */

/// The number of channels in an image.
typedef enum cifex_channels
{
   cifex_rgb = 3,
   cifex_rgba = 4,
} cifex_channels_t;

/// An image.
typedef struct cifex_image
{
   /// The allocator this image was allocated with.
   cifex_allocator_t *allocator;

   uint32_t width, height;
   cifex_channels_t channels;

   /// The spacing of individual pixels depends on the `pixel_type`.
   ///
   /// - for `cifex_rgb`, it's 3 bytes per pixel.
   /// - for `cifex_rgba`, it's 4 bytes per pixel.
   uint8_t *data;
} cifex_image_t;

/// Calculates the amount of storage needed to store the given image's data.
inline size_t
cifex_image_storage_size(uint32_t width, uint32_t height, cifex_channels_t channels)
{
   return width * height * channels;
}

/// Allocates memory for the image and clears it with zeroes.
///
/// This will only allocate a new image if the existing storage size does not match the provided
/// storage size.
///
/// Note that using this on uninitialized images will result in unwanted behavior! Always initialize
/// your image with `= {0}` before using this.
cifex_result_t
cifex_alloc_image(
   cifex_image_t *image,
   cifex_allocator_t *allocator,
   uint32_t width,
   uint32_t height,
   cifex_channels_t channels);

/// Frees the image's data.
///
/// Calling this on an already freed image is safe.
void
cifex_free_image(cifex_image_t *image);

/* --------------
   Image decoding
   -------------- */

/// The decoding configuration.
typedef struct cifex_decode_config
{
   /// The allocator used for memory allocations.
   cifex_allocator_t allocator;
   cifex_reader_t reader;
} cifex_decode_config_t;

/// Decodes an image into `out_image`.
cifex_result_t
cifex_decode(const cifex_decode_config_t *config, cifex_image_t *out_image);

#endif
