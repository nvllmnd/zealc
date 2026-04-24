#pragma once


#include "attributes.h"
#include "intdefs.h"
#include "memory/alloc.h"
typedef struct ArenaHeap ArenaHeap;


RETURNS_RESOURCE
ArenaHeap* arena_heap_new(isize capacity);

RETURNS_RESOURCE
ArenaHeap* arena_heap_new_in(isize capacity, Allocator parent);

METHOD
void* arena_heap_alloc(ArenaHeap* self, isize size, isize align);

METHOD
void* arena_heap_zalloc(ArenaHeap* self, isize size, isize align);

METHOD
void arena_heap_clear(ArenaHeap* self);

/// Starts from the last Block (or the first block allocated as root, not including the memory block in ArenaHeap root)
/// and frees/releases the memory used by that block back to the system. Tries to release up to @param (nblocks).
/// if @param (nblocks) <= 0, then all blocks used by this [ArenaHeap] are freed, and the memory in [ArenaHeap] root is zeroed
///
/// If you want to keep all blocks currently allocated, but would like to reset/clear all memory and reset all blocks used counters,
/// see [arena_heap_clear]
METHOD
void arena_heap_release(ArenaHeap* self, isize nblocks);

METHOD
void arean_heap_destroy(ArenaHeap* self);


CONST_FUNC
RETURNS_NON_NULL
const AllocVTable* arena_heap_alloc_vtable(void);

METHOD
Allocator arena_heap_allocator(ArenaHeap* self);
