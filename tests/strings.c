
#include "memory/cstr.h"
#include "unity.h"



#include "intdefs.h"

void setUp(void)
{
}

void tearDown(void)
{
}



void string_compare(void) {
  
  static constexpr const char STR[] = "this is a test string!";
  const cstr l = cstr_new(STR);
  const cstr r = cstr_new(STR); 

  TEST_ASSERT_TRUE_MESSAGE(cstr_eq(&l, &r), "cstr_cmp between 2 strings that should be the same failed!");

  const cstr diff = cstr_new("this is a different string!");

  TEST_ASSERT_FALSE_MESSAGE(cstr_eq(&l, &diff), "Strings should be diff");

  const sslice slice_this = sslice_from_range(cstr_as_ptr(&diff), 0, 4);
  const sslice slice_that = sslice_from_range(cstr_as_ptr(&l), 0, 4);

  TEST_ASSERT_TRUE_MESSAGE(sslice_eq(slice_this, slice_that), "slices should match");


}


i32 main(void) {
  UNITY_BEGIN();

  RUN_TEST(string_compare);

  return UNITY_END();
}
