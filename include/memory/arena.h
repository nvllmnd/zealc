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

METHOD
void arean_heap_destroy(ArenaHeap* self);


CONST_FUNC
RETURNS_NON_NULL
const AllocVTable* arena_heap_alloc_vtable(void);

METHOD
Allocator arena_heap_allocator(ArenaHeap* self);
