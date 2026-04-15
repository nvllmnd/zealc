#pragma once
#include <stdatomic.h>
#include <string.h>

#include "attributes.h"
#include "intdefs.h"

/// Function pointer typedef for [Allocator] [AllocVTable] allocate method.
///
/// void* self  - Pointer to self (may be null if Allocator has no state!)
///              Typically you will cast this to your derived Allocator type in this method implementation
/// usize size  - Size of allocation requested in bytes
typedef void* (*VTableAllocate)(void* self, isize size);
/// Function pointer typedef for [Allocator] [AllocVTable] reallocate method
///
/// void* self     - Pointer to self (may be null if Allocator has no state!)
///                  Typically you will cast this to your derived Allocator type in this method implementation
/// void* ptr      - Pointer to begging of block of memory to be reallocated
/// usize new_size - Size of requested reallocation in bytes
typedef void* (*VTableReallocate)(void* self, void* ptr, isize new_size);

/// Function pointer typedef for [Allocator] [AllocVTable] zallocate method
/// This is the same as [VTableAllocate], but ensures allocated memory is zeroed
///
/// void* self  - Pointer to self (may be null if Allocator has no state!)
///               Typically you will cast this to your derived Allocator type in this method implementation
/// usize size  - Size of allocation requested in bytes
typedef void* (*VTableZallocate)(void* self, isize size);

/// Function pointer typedef for [Allocator] [AllocVTable] expand method
/// This is the same as [VTableReallocate], but does nothing  if
/// memory cannot be expanded in place to new_size
///
/// void* self     - Pointer to self (may be null if Allocator has no state!)
///                  Typically you will cast this to your derived Allocator type in this method implementation
/// void* ptr      - Pointer to begging of block of memory to be expanded in place
/// usize new_size - Size of requested reallocation in bytes
typedef void* (*VTableExpand)(void* self, void* ptr, isize new_size);

/// Function pointer typedef for [Allocator] [AllocVTable] free method.
///
/// void* self - Pointer to self (may be null if Allocator has no state!)
///              Typically you will cast this to your derived Allocator type in this method implementation
/// void* ptr  - Pointer to block of memory to be freed by this allocator
typedef void (*VTableFree)(void* self, void* ptr);

/// [Allocator] VTable struct that contains function pointers
/// to Allocator implementations
struct AllocVTable {
  /// See [VTableAllocate]
  VTableAllocate allocate;
  /// See [VTableReallocate]
  VTableReallocate reallocate;
  /// See [VTableZallocate]
  VTableZallocate zallocate;
  /// See [VTableExpand]
  VTableExpand expand_in_place;
  /// See [VTableFree]
  VTableFree free;
};
typedef struct AllocVTable AllocVTable;

CONST_FUNC
const AllocVTable* global_allocator_vtable(void);

#define alloc_vtable_new(...) ((AllocVTable){__VA_ARGS__})

/// C-Style Allocator Interface
struct Allocator {
  void* ctx;
  const AllocVTable* vtable;
};
typedef struct Allocator Allocator;

CONST_FUNC
Allocator global_allocator(void);

[[nodiscard("Must not discard pointer returned from allocator! possible memory leak!")]]
static inline void* allocator_allocate(Allocator self, isize size) { return self.vtable->allocate(self.ctx, size); }


[[nodiscard("Must not discard pointer returned from allocator! possible memory leak!")]]
static inline void* allocator_reallocate(Allocator self, void* ptr, isize new_size) {
  return self.vtable->reallocate(self.ctx, ptr, new_size);
}

[[nodiscard("Must not discard pointer returned from allocator! possible memory leak!")]]
static inline void* allocator_zallocate(Allocator self, isize size) { return self.vtable->zallocate(self.ctx, size); }

[[nodiscard("Must not discard pointer returned from allocator! possible memory leak!")]]
static inline void* allocator_expand(Allocator self, void* ptr, isize new_size) {
  return self.vtable->expand_in_place(self.ctx, ptr, new_size);
}

static inline void allocator_free(Allocator self, void* ptr) { self.vtable->free(self.ctx, ptr); }

/// A simple Arena Allocator
///
/// If this Arena is not really meant to resize the buffer
/// it owns, as that would cause a nightmare where pointers allocated up to that point
/// are going to be invalidated after arena resize.
///
/// TODO: Implement a roped arena, where each arena has a prev field
/// of its own type (a linked list of Arenas, where the top-most arena is the root of the
/// chain, going back, each arena block has a pointer to the previous (full) Arena.) This way
/// we append new blocks of memory when we run out, we allocate a new block, set the current root to the next
/// arena block previous field, then set the new arena block as the new root. boom. We are able
/// to grow in size and pointers dont get invalidated. This is at the cost of fragmenting memory a little bit,
/// but we can get even crazier with it, and allow for the roped/chained Arena to take a parent allocator to
/// allocate out of. That way theoretically, you could have a master allocator that allocates out of a huge block
/// of static memory, and then have several roped/chained arena allocate our of that every time they need more space,
/// and there you have a growable, non-pointer-invalidating, Arena that is contiguous in memory. But all thats for another day
/// i have some other things to write 
struct Arena {
  u8* mem;
  isize capacity;
  isize used;
};
typedef struct Arena Arena;

/// Creates a new [Arena] struct.
/// If this function fails to allocate with the global allocator,
/// or runs into an unexpected error during its execution at runtime,
/// then this function will return a zeroed/null [Arena] instance.
///
/// You can use [arena_is_ok] function to check that the returned [Arena] instance
/// is valid and ready to be used.
[[nodiscard("Must check returned Arena is not zeroed, in which case it must be freed before going out of scope")]]
Arena arena_new(isize capacity);

/// Checks that a newly created/initialized Arena non-null/non-zeroed
/// and has a valid pointer to memory and a valid capacity
///
/// This is necessary as ZII (Zero Is Initialization), or rather, zero as error.
/// so if [arena_new] cannot allocate for some reason, or runs into an unexpected error,
/// it will return a zeroed [Arena] struct instead of one that has valid fields and is
/// ready to be used
///
static inline bool arena_is_ok(const Arena* self) {
  return self && self->mem && self->capacity > 0;
}

void* arena_allocate(Arena* self, isize size);

CONST_FUNC
const AllocVTable* arena_alloc_vtable(void);

static inline Allocator arena_allocator(Arena* self) {
  return (Allocator){.ctx = (void*)self, .vtable = arena_alloc_vtable()};
}

void arena_destroy(Arena* self);
