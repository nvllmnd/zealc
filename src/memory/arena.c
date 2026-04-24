#include "memory/arena.h"

#include <assert.h>
#include <string.h>

#include "attributes.h"
#include "core/constants.h"
#include "core_types.h"
#include "log.h"
#include "memory/alloc.h"
#include "mimalloc-override.h"

struct Block {
  struct Block* prev;
  i64 used;
  i64 capacity;
  u8 mem[];
};
typedef struct Block Block;

struct ParentAllocator {
  union {
    mi_heap_t* heap;
    Allocator alloc;
  };

  /// Is this parent allocator of type [Allocator]
  bool is_trait;
};
typedef struct ParentAllocator ParentAllocator;

CONST_FUNC
static inline ParentAllocator palloc_heap_new(void) {
  mi_heap_t* heap = mi_heap_new();
  assert(is_not_null(heap));

  return make(ParentAllocator, .heap = heap, .is_trait = false);
}

PURE_FUNC
static inline ParentAllocator palloc_allocator_new(Allocator inner) {
  return make(ParentAllocator, .alloc = inner, .is_trait = true);
}

// static inline void* palloc_allocate(ParentAllocator self, isize size, isize align) {
//   if (self.is_trait) {
//     return allocator_allocate(self.alloc, size, align);
//   } else {
//     return mi_heap_malloc_aligned(self.heap, size, align);
//   }
// }

static inline void* palloc_zallocate(ParentAllocator self, isize size, isize align) {
  if (self.is_trait) {
    return allocator_zallocate(self.alloc, size, align);
  } else {
    return mi_heap_zalloc_aligned(self.heap, size, align);
  }
}


[[maybe_unused]]
static inline void palloc_free(ParentAllocator self, void* ptr) {
  if (self.is_trait) {
    allocator_free(self.alloc, ptr);
  } else {
    mi_free(ptr);
  }
}

[[maybe_unused]]
static inline void palloc_delete(ParentAllocator self) {
  if (!self.is_trait) {
    mi_heap_delete(self.heap);
  }
}

static inline Block* palloc_block_new(ParentAllocator self, Block* current, isize size) {
  const isize capacity = sizeof(Block) + size;

  Block* block = palloc_zallocate(self, capacity, alignof(Block));

  if (UNLIKELY(is_null(block))) {
    ELOG_DBG(
        "ArenaHeap::palloc_block_new Failed to allocate block from parent allocator! Parent allocator returned "
        "nullptr!");

    assert(block);
    return nullptr;
  }

  block->capacity = capacity;
  block->prev = current;
  return block;
}

struct BlockSlice {
  /// Top of the memory region to allocate from
  u8* top;
  /// remaining space of this block in bytes
  i64 remaining;
};
typedef struct BlockSlice BlockSlice;

struct ArenaHeap {
  ParentAllocator parent;

  struct ArenaHeapStats {
    i64 total_used;
    i64 total_allocated;
  } stats;

  struct Block* root;

  // NOTE: We use these last fields as our inital size allocation,
  // then there is less pointer indirection, as i dont like the idea
  // of allocation this [ArenaHeap] struct if its only used for a couple pointers and some stat tracking

  i64 mem_used;
  i64 mem_cap;
  u8 mem[];
};

typedef struct ArenaHeapStats ArenaHeapStats;


METHOD
static inline void* ah_try_inner_allocate(ArenaHeap* self, isize size, isize align) {
  const isize end = self->mem_used + size;
  if (end < self->mem_cap) {
    u8* start = &self->mem[self->mem_used];
    u8* aligned_start = align_ptr(start, align);
    const u8* alloc_end = aligned_start + size;

    const u8* mem_end = &self->mem[self->mem_cap - 1];
    if LIKELY (alloc_end < mem_end) {
      const isize alloc_size = (size + (aligned_start - start));
      self->mem_used += alloc_size;
      self->stats.total_used += alloc_size;
      return aligned_start;
    }
  }

  return nullptr;
}

METHOD
PURE_FUNC
static inline bool ah_alloc_in_block_ok(ArenaHeap* self, isize size, isize align) {
  const Block* root = self->root;
  if LIKELY (self->root) {
    const isize size_end = self->root->used + size;

    if (size_end >= self->root->capacity) {
      return false;
    }

    const u8* alloc_start = &root->mem[root->used];
    const u8* aligned_start = align_ptr(alloc_start, align);
    const u8* alloc_end = aligned_start + size;

    const u8* mem_end = &root->mem[root->capacity - 1];

    if UNLIKELY (alloc_end >= mem_end) {
      return false;
    }

    return true;
  }
  return false;
}

/// This function checks if self->root is not null, however it does
/// not verify that the requested allocation will fit in this block, so be sure
/// to check that this block can fit an allocation of @param (size) in bytes
METHOD
static inline void* ah_block_allocate(ArenaHeap* self, isize size, isize align) {
  if UNLIKELY (is_null(self->root)) {
    assert(false);
    return nullptr;
  }

  Block* root = self->root;
  u8* start = &root->mem[root->used];
  u8* aligned = align_ptr(start, align);

  const isize alloc_size = (size + (aligned - start));
  root->used += alloc_size;
  self->stats.total_used += alloc_size;
  return aligned;
}

METHOD
static inline void* ah_allocate(ArenaHeap* self, isize size, isize align) {
  // try to allocate from ArenaHeap's inner memory buffer first
  // this function returns a nullptr if its out of memory
  // or it is unable to fit this allocation
  void* ptr = ah_try_inner_allocate(self, size, align);
  if (is_not_null(ptr)) {
    return ptr;
  }

  if (is_null(self->root) || !ah_alloc_in_block_ok(self, size, align)) {
    const isize block_size = max(self->root->capacity * 2, size);
    self->root = palloc_block_new(self->parent, self->root, block_size);
  }

  return ah_block_allocate(self, size, align);
}

static void* arena_heap_vtalloc_impl(void* ctx, isize size, isize align) {
  ArenaHeap* self = pcast(ArenaHeap, ctx);
  return arena_heap_alloc(self, size, align);
}

static void* arena_heap_vtzalloc_impl(void* ctx, isize size, isize align) {
  ArenaHeap* self = pcast(ArenaHeap, ctx);
  return arena_heap_zalloc(self, size, align);
}

static const AllocVTable HEAP_VTABLE =
    alloc_vtable_new(.allocate = arena_heap_vtalloc_impl, .zallocate = arena_heap_vtzalloc_impl,
                     .expand_in_place = NO_IMPL_EXPAND, .free = NO_IMPL_FREE, .reallocate = NO_IMPL_REALLOCATE);

ArenaHeap* arena_heap_new(isize capacity) {
  ParentAllocator parent = palloc_heap_new();

  const isize size = sizeof(ArenaHeap) + capacity;
  ArenaHeap* self = palloc_zallocate(parent, size, alignof(ArenaHeap));

  if UNLIKELY (is_null(self)) {
    assert(false);
    return nullptr;
  }

  *self = make(ArenaHeap, .parent = parent, .stats = make(ArenaHeapStats, .total_used = 0, .total_allocated = capacity),
               .root = nullptr, .mem_used = 0, .mem_cap = capacity);

  return self;
}

ArenaHeap* arena_heap_new_in(isize capacity, Allocator parent) {
  ParentAllocator palloc = palloc_allocator_new(parent);
  const isize size = sizeof(ArenaHeap) + capacity;
  ArenaHeap* self = palloc_zallocate(palloc,  size,  alignof(ArenaHeap));

  if UNLIKELY (is_null(self)) {
    assert(false);
    return nullptr;
  }

  const auto stats = make(ArenaHeapStats, .total_used = 0, .total_allocated = capacity);
  *self = make(ArenaHeap, .parent = palloc, .stats = stats, .root = nullptr, .mem_used = 0, .mem_cap = capacity);

  return self;
  
}

void* arena_heap_alloc(ArenaHeap* self, isize size, isize align) { return ah_allocate(self, size, align); }

void* arena_heap_zalloc(ArenaHeap* self, isize size, isize align) {
  u8* mem = arena_heap_alloc(self, size, align);
  if UNLIKELY (is_null(mem)) {
    return nullptr;
  }
  memset(mem, 0, size);
  return mem;
}

// void arena_heap_clear(ArenaHeap* self) {
  
// }

// void arean_heap_destroy(ArenaHeap* self) {
  
// }

const AllocVTable* arena_heap_alloc_vtable(void) { return &HEAP_VTABLE; }

Allocator arena_heap_allocator(ArenaHeap* self) {
  return make(Allocator, .ctx = self, .vtable = &HEAP_VTABLE);
}
