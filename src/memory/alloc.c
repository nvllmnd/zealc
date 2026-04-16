#include "memory/alloc.h"

#include <assert.h>
#include <stdatomic.h>
#include <string.h>

#include "core_types.h"
#include "mimalloc.h"

static void* global_vtable_alloc(void*, isize size) {
  return mi_malloc(size);  
}

static void* global_vtable_realloc(void*, void* ptr, isize new_size) {
  return mi_realloc(ptr, new_size);
}

static void* global_vtable_zalloc(void*, isize size) {
  return mi_zalloc(size);  
}

static void* global_vtable_expand(void*, void* ptr, isize new_size) {
  return mi_expand(ptr, new_size);  
}

static void global_vtable_free(void*, void* ptr) {
   mi_free(ptr);
}

static const AllocVTable GLOBAL_ALLOC_VTABLE =
    alloc_vtable_new(.allocate = global_vtable_alloc, .reallocate = global_vtable_realloc,
                     .zallocate = global_vtable_zalloc, .expand_in_place = global_vtable_expand,
                     .free = global_vtable_free);

const AllocVTable* global_allocator_vtable(void) { return &GLOBAL_ALLOC_VTABLE; }

Allocator global_allocator(void) { return (Allocator){.ctx = nullptr, .vtable = global_allocator_vtable()}; }

HeapAllocator heap_allocator_new(void) { return (HeapAllocator){.heap = mi_heap_new()}; }

static void* heap_vtable_alloc(void* ctx, isize size) {
  mi_heap_t* self = pcast(mi_heap_t, ctx);
  return mi_heap_malloc(self, size);
}

static void* heap_vtable_realloc(void* ctx, void* ptr, isize new_size) {
  mi_heap_t* self = pcast(mi_heap_t, ctx);
  return mi_heap_realloc(self, ptr, new_size);
}

static void* heap_vtable_zalloc(void* ctx, isize size) {
  mi_heap_t* self = pcast(mi_heap_t, ctx);
  return mi_heap_zalloc(self, size);
}

static void* heap_vtable_expand(void*, void* ptr, isize new_size) {
  return mi_expand(ptr,  new_size);
}

static void heap_vtable_free(void*, void* ptr) {
  mi_free(ptr);
}

/// forwards call  to [mi_heap_malloc]
void* heap_allocator_malloc(HeapAllocator self, isize size) { return heap_vtable_alloc(self.heap, size); }
/// forwards call to [mi_heap_realloc]
void* heap_allocator_realloc(HeapAllocator self, void* ptr, isize new_size) {
  return heap_vtable_realloc(self.heap, ptr, new_size);
}
/// forwards call to [mi_heap_zalloc]
void* heap_allocator_zalloc(HeapAllocator self, isize size) { return heap_vtable_zalloc(self.heap, size); }
/// forwards call to [mi_expand]
void* heap_allocator_expand(HeapAllocator, void* ptr, isize new_size) { return mi_expand(ptr, new_size); }
/// forwwards call to [mi_free]
void heap_allocator_free(HeapAllocator, void* ptr) { mi_free(ptr); }

static const AllocVTable HEAP_ALLOC_VTABLE =
    alloc_vtable_new(.allocate = heap_vtable_alloc, .reallocate = heap_vtable_realloc, .zallocate = heap_vtable_zalloc,
                     .expand_in_place = heap_vtable_expand, .free = heap_vtable_free);

/// Returns a const pointer to [HeapAllocator]'s vtable ([AllocVTable])
const AllocVTable* heap_allocator_vtable(void) { return &HEAP_ALLOC_VTABLE; }

/// forwards call to [mi_heap_delete]
void heap_allocator_delete(HeapAllocator* self) {
  if (self->heap) {
    mi_heap_delete(self->heap);
    self->heap = nullptr;
  }
}

/// forwards call to [mi_heap_destroy]
/// Caution as this can cause program crashes if you are not careful
void heap_allocator_destroy(HeapAllocator* self) {
  if (self->heap) {
    mi_heap_destroy(self->heap);
    self->heap = nullptr;
  }
}

Arena arena_new(isize capacity) {
  u8* mem = mi_calloc(1, capacity);
  if (is_null(mem)) {
    // return zeroed Arena to indicate allocation/init error.
    return (Arena){};
  }

  return (Arena){
      .mem = mem,
      .used = 0,
      .capacity = capacity,
  };
}

Arena arena_new_in(isize capacity, Allocator alloc) {
  u8* mem = allocator_allocate(alloc, capacity);
  if (is_null(mem)) {
    return (Arena){};
  }
  return (Arena){.mem = mem, .used = 0, .capacity = capacity};
}

static void* arena_vtable_alloc(void* ctx, isize size) {
  Arena* self = pcast(Arena, ctx);

  const isize next_used = (self->used + size);
  if (next_used >= self->capacity) {
    return nullptr;
  }
  void* ptr = &self->mem[self->used];
  self->used = next_used;

  return ptr;
}

static void* arena_vtable_zalloc(void* ctx, isize size) {
  Arena* self = pcast(Arena, ctx);

  void* ptr = arena_allocate(self, size);
  if (is_null(ptr)) {
    return nullptr;
  }
  memset(ptr, 0, size);
  return ptr;
}

void* arena_allocate(Arena* self, isize size) { return arena_vtable_alloc(pcast(void, self), size); }

void* arena_zallocate(Arena* self, isize size) { return arena_vtable_zalloc(pcast(Arena, self), size); }

void* vtable_alloc_no_impl(void*, isize) { return NO_IMPL_METHOD_RESULT; }

void* vtable_realloc_no_impl(void*, void*, isize) { return NO_IMPL_METHOD_RESULT; }
void* vtable_zalloc_no_impl(void*, isize) { return NO_IMPL_METHOD_RESULT; }
void* vtable_expand_no_impl(void*, void*, isize) { return NO_IMPL_METHOD_RESULT; }
void vtable_free_no_impl(void*, void*) {}

static const AllocVTable ARENA_ALLOC_VTABLE =
    alloc_vtable_new(.allocate = arena_vtable_alloc, .reallocate = NO_IMPL_REALLOCATE, .zallocate = arena_vtable_zalloc,
                     .expand_in_place = NO_IMPL_EXPAND, .free = NO_IMPL_FREE);

const AllocVTable* arena_alloc_vtable(void) { return &ARENA_ALLOC_VTABLE; }

void arena_destroy(Arena* self) {
  if (self->mem) {
    mi_free(self->mem);

    *self = (Arena){};
  }
}

/// cleans up memory used by [Arena]. Must use the same [Allocator] that was used to create this [Arena]!
void arena_destroy_in(Arena* self, Allocator alloc) {
  if (self->mem) {
    allocator_free(alloc, self->mem);
    *self = (Arena){};
  }
}

void arena_clear(Arena* self) {
  self->used = 0;
}

void arena_clear_zeroed(Arena* self) {
  arena_clear(self);
  memset(self->mem, 0, self->capacity);
}
