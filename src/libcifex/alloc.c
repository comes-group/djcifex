#include "public/libcifex.h"

#include "cxensure.h"
#include <stdlib.h>
#include <string.h>

void *
cifex_alloc(cifex_allocator_t *allocator, size_t size)
{
   cx_ensure(allocator != NULL, "allocator must not be NULL");
   void *ptr = allocator->malloc(allocator, size);
   memset(ptr, 0, size);
   return ptr;
}

void
cifex_free(cifex_allocator_t *allocator, void *ptr)
{
   if (allocator != NULL && ptr != NULL) {
      allocator->free(allocator, ptr);
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
cifex_libc_allocator(void)
{
   return (cifex_allocator_t){
      .malloc = cx_libc_malloc_adapter,
      .free = cx_libc_free_adapter,
   };
}
