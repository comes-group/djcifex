#include "public/libcifex.h"

#include <errno.h>
#include <stdio.h>

#include "cxensure.h"

static size_t
cx_stdio_fread(cifex_reader_t *reader, void *out, size_t n_bytes)
{
   cx_ensure(reader->user_data != NULL, "attempt to read from closed reader");

   FILE *file = reader->user_data;
   return fread(out, 1, n_bytes, file);
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
cifex_fopen_read(cifex_reader_t *reader, const char *filename)
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

static size_t
cx_stdio_fwrite(cifex_writer_t *writer, const void *in, size_t n_bytes)
{
   cx_ensure(writer->user_data != NULL, "attempt to write to a closed writer");

   FILE *file = writer->user_data;
   return fwrite(in, 1, n_bytes, file);
}

cifex_result_t
cifex_fopen_write(cifex_writer_t *writer, const char *filename)
{
   cx_ensure(writer != NULL, "writer must not be NULL");

   FILE *file = fopen(filename, "wb");
   if (file == NULL) {
      return cifex_errno_result(errno);
   }

   *writer = (cifex_writer_t){
      .user_data = file,
      .write = cx_stdio_fwrite,
   };

   return cifex_ok;
}

cifex_result_t
cifex_fclose_read(cifex_reader_t *reader)
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

cifex_result_t
cifex_fclose_write(cifex_writer_t *writer)
{
   cx_ensure(writer != NULL, "writer must not be NULL");
   cx_ensure(writer->user_data != NULL, "attempt to close an already closed writer");

   FILE *file = writer->user_data;
   if (fclose(file) != 0) {
      return cifex_errno_result(errno);
   }

   writer->user_data = NULL;

   return cifex_ok;
}
