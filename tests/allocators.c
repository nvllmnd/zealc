

#include <string.h>

#include "nv/nv.h"
#include "unity.h"

// static Arena* ALLOC = nullptr;

void setUp(void) {
  // ALLOC = arena_new(MEGABYTES(4), KILOBYTES(16));
  // mi_option_set_enabled(mi_option_show_stats, true);
  // mi_option_set_enabled(mi_option_verbose, true);
  // mi_option_set_enabled(mi_option_show_errors, true);
}

void tearDown(void) {
  // arena_destroy(ALLOC);
  // ALLOC = nullptr;
}

// struct Stuff {
//   char buf[255];

//   struct Point {
//     f32 x;
//     f32 y;
//   } points[20];

//   i64 counter;
// };
// alias(Stuff);

// void arena_heap_alignment_nofragment(void) {
//   Stuff* s = arena_zalloc(ALLOC, mlayout_new(Stuff));
//   TEST_ASSERT_NOT_NULL(s);

//   *s = make(Stuff, .buf = {}, .points = {}, .counter = 5);
// }

// void arena_heap_can_grow_and_destroy(void) {
//   for (i32 i = 0; i < 50; i++) {
//     char* b1 = arena_zalloc(ALLOC, mlayout_bytes(1024));
//     char* b2 = arena_zalloc(ALLOC, mlayout_bytes(2048));

//     TEST_ASSERT_NOT_NULL(b1);
//     TEST_ASSERT_NOT_NULL(b2);

//     strncpy(b1, "ayooo", sizeof("ayooo"));

//     TEST_ASSERT_EQUAL_STRING(b1, "ayooo");
//   }

//   const ArenaStats stats = arena_stats(ALLOC);
//   println("TOTAL ALLOCATED IN BYTES : %li", stats.total_used);
// }

i32 main(void) {
  UNITY_BEGIN();


  return UNITY_END();
}
