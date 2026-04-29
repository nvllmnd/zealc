#include "memory/gpa.h"
#include <assert.h>

#include "memory/arena.h"

static ArenaHeap* MAIN_ARENA = nullptr;


ArenaHeap* arena_main(void) {
  return MAIN_ARENA;
}

void memory_init(isize initial_capacity) {
  MAIN_ARENA = arena_heap_new(initial_capacity); 
}


void memory_free(void) {
  arena_heap_destroy(MAIN_ARENA);
  MAIN_ARENA = nullptr;
}

void* arena_calloc(isize size, isize count, isize align) {
  assert(MAIN_ARENA);

  return arena_heap_zalloc(MAIN_ARENA, count * size,  align);
}
