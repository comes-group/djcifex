#include "public/libcifex.h"

#include "ensure.h"

void *
cifex_alloc(cifex_allocator_t *allocator, size_t size)
{
   cx_ensure(allocator != NULL, "allocator must not be NULL");
   return allocator->malloc(allocator, size);
}

void
cifex_free(cifex_allocator_t *allocator, void **varptr)
{
   cx_ensure(allocator != NULL, "allocator must not be NULL");
   cx_ensure(varptr != NULL, "the pointer to the variable must not be NULL");

   if (*varptr != NULL) {
      allocator->free(allocator, *varptr);
      *varptr = NULL;
   }
}
