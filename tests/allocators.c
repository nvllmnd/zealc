

#include "intdefs.h"
#include "memory/alloc.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}



/// we can put a global allocator in an [Allocator]
/// struct and everything works just fine
void global_allocator_trait(void) {
  const Allocator g = global_allocator();

  TEST_ASSERT_NULL(g.ctx);
  TEST_ASSERT_NOT_NULL(g.vtable);

  TEST_ASSERT_NOT_NULL(g.vtable->allocate);

  u8* mem = allocator_allocate(g, 64, alignof(u8[64]));
  TEST_ASSERT_NOT_NULL(mem);

  TEST_ASSERT_NOT_NULL(g.vtable->free);

  allocator_free(g, mem);

  
}

i32 main(void) {
  UNITY_BEGIN();

  RUN_TEST(global_allocator_trait);

  return UNITY_END();
}
