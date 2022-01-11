#include "public/libcifex.h"

#include <stdio.h>

#include "ensure.h"

static size_t
stdio_fread(cifex_reader_t *reader, void *out, size_t n_bytes)
{
   cx_ensure(reader != NULL, "reader must not be NULL");
   cx_ensure(reader->user_data != NULL, "attempt to read from closed reader");

   FILE *file = reader->user_data;
   return fread(out, n_bytes, 1, file);
}

cifex_reader_t
cifex_fopen(const char *filename)
{
   FILE *file = fopen(filename, "rb");
   return (cifex_reader_t){
      .user_data = file,
      .fread = stdio_fread,
   };
}

void
cifex_fclose(cifex_reader_t *reader)
{
   cx_ensure(reader != NULL, "reader must not be NULL");
   cx_ensure(reader->user_data != NULL, "attempt to close an already closed reader");

   FILE *file = reader->user_data;
   fclose(file);
   reader->user_data = NULL;
}
