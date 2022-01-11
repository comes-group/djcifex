#include "public/libcifex.h"

#include "ensure.h"
#include <stdlib.h>

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

static void *
cx_libc_malloc_adapter(cifex_allocator_t *allocator, size_t size)
{
   (void)allocator;
   return malloc(size);
}

static void
cx_libc_free_adapter(cifex_allocator_t *allocator, void *ptr)
{
   (void)allocator;
   return free(ptr);
}

cifex_allocator_t
cifex_libc_allocator()
{
   return (cifex_allocator_t){
      .malloc = cx_libc_malloc_adapter,
      .free = cx_libc_free_adapter,
   };
}
