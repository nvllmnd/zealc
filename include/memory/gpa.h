#pragma once



/// Returns a pointer to the main arena used for general allocations,
/// this arena lives for the lifetime of the compiler's runtime and is freed at the end
/// This function may return a nullptr if [memory_init] isnt called first
#include "attributes.h"
#include "intdefs.h"


PURE_FUNC
struct ArenaHeap* arena_main(void);


void memory_init(isize initial_capacity);

void memory_free(void);

void* arena_calloc(isize size, isize count, isize align);

