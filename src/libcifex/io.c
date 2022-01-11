#include "public/libcifex.h"

#include <errno.h>
#include <stdio.h>

#include "ensure.h"

static size_t
cx_stdio_fread(cifex_reader_t *reader, void *out, size_t n_bytes)
{
   cx_ensure(reader->user_data != NULL, "attempt to read from closed reader");

   FILE *file = reader->user_data;
   return fread(out, n_bytes, 1, file);
}

static int
cx_stdio_fseek(cifex_reader_t *reader, long offset, int whence)
{
   cx_ensure(reader->user_data != NULL, "attempt to seek in a closed reader");

   FILE *file = reader->user_data;
   return fseek(file, offset, whence);
}

static long
cx_stdio_ftell(cifex_reader_t *reader)
{
   cx_ensure(reader->user_data != NULL, "attempt to seek in a closed reader");

   FILE *file = reader->user_data;
   return ftell(file);
}

cifex_result_t
cifex_fopen(cifex_reader_t *reader, const char *filename)
{
   cx_ensure(reader != NULL, "reader must not be NULL");

   FILE *file = fopen(filename, "rb");
   if (file == NULL) {
      return cifex_errno_result(errno);
   }

   *reader = (cifex_reader_t){
      .user_data = file,
      .read = cx_stdio_fread,
      .seek = cx_stdio_fseek,
      .tell = cx_stdio_ftell,
   };

   return cifex_ok;
}

cifex_result_t
cifex_fclose(cifex_reader_t *reader)
{
   cx_ensure(reader != NULL, "reader must not be NULL");
   cx_ensure(reader->user_data != NULL, "attempt to close an already closed reader");

   FILE *file = reader->user_data;
   if (fclose(file) != 0) {
      return cifex_errno_result(errno);
   }

   reader->user_data = NULL;

   return cifex_ok;
}
