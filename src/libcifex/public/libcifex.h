#ifndef LIBCIFEX_H
#define LIBCIFEX_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* --------------
   Constants
   -------------- */

#define CIFEX_FORMAT_VERSION 1

/* --------------
   Error handling
   -------------- */

typedef enum cifex_result
{
   cifex_ok,
   /// Ran out of memory while performing the operation.
   cifex_out_of_memory,
   /// The image file is empty.
   cifex_empty_image_file,
   /// A syntax error was found.
   cifex_syntax_error,
   /// This format version is invalid.
   ///
   /// Emitted when the format version is `zero`.
   cifex_invalid_version,
   /// This decoder is too old to decode files of the given version.
   cifex_unsupported_version,
   /// This number of bits per pixel is not supported.
   cifex_invalid_bpp,
   /// A channel's value was out of the 0..255 range.
   cifex_channel_out_of_range,

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
///
/// The memory is cleared with zeroes.
void *
cifex_alloc(cifex_allocator_t *allocator, size_t size);

/// Deallocates a memory region using the allocator. `ptr` must point to an allocation made by
/// the allocator.
void
cifex_free(cifex_allocator_t *allocator, void *ptr);

/// Returns the libc allocator.
cifex_allocator_t
cifex_libc_allocator(void);

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

typedef enum cifex_flags
{
   cifex_flag_polish = 0x1,
} cifex_flags_t;

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

typedef struct cifex_metadata_pair cifex_metadata_pair_t;

/// A key-value metadata pair, stored as a linked list.
typedef struct cifex_metadata_pair
{
   char *key;
   size_t key_len;

   char *value;
   size_t value_len;

   /// If not `NULL`, points to the next metadata field.
   cifex_metadata_pair_t *next;
   cifex_metadata_pair_t *prev;
} cifex_metadata_pair_t;

/// CIF-specific image information.
typedef struct cifex_image_info
{
   /// The allocator that was used for populating the image info.
   cifex_allocator_t *allocator;

   /// Format version.
   uint32_t version;

   /// Format flags.
   ///
   /// Must contain `cifex_flag_polish`.
   cifex_flags_t flags;

   /// Metadata pairs.
   cifex_metadata_pair_t *metadata;
   cifex_metadata_pair_t *metadata_last;
} cifex_image_info_t;

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
/// It is safe to call this on an already freed image.
void
cifex_free_image(cifex_image_t *image);

/// Frees image info.
///
/// It is safe to call this on already freed image info.
void
cifex_free_image_info(cifex_image_info_t *image_info);

/* --------------
   Image decoding
   -------------- */

/// The decoding configuration.
typedef struct cifex_decode_config
{
   /// The allocator used for memory allocations.
   cifex_allocator_t *allocator;
   cifex_reader_t *reader;

   /// Pass `false` to not load the metadata fields into the image info.
   ///
   /// Default: `true`
   bool load_metadata;
} cifex_decode_config_t;

cifex_decode_config_t
cifex_default_decode_config(cifex_allocator_t *allocator, cifex_reader_t *reader);

/// The result of decoding an image.
typedef struct cifex_decode_result
{
   cifex_result_t result;
   /// Populated with the byte on which the error occured, or `0` if not applicable.
   size_t position;
   /// Populated with the line on which the error occured, or `0` if not applicable.
   size_t line;
} cifex_decode_result_t;

/// Decodes an image into `out_image`.
///
/// `out_image_info` can be NULL if CIF-specific metadata isn't needed.
cifex_decode_result_t
cifex_decode(
   cifex_decode_config_t config,
   cifex_image_t *out_image,
   cifex_image_info_t *out_image_info);

#endif
