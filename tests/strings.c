
#include <stdio.h>

#include "ast/symbol.h"
#include "ast/token.h"
#include "intdefs.h"
#include "memory/cstr.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void keyword_lookup_table(void) {
  static const sslice KEYWORDS[Token__KeywordCount] = {
      sslice_static_new("true"),  sslice_static_new("false"),    sslice_static_new("let"),
      sslice_static_new("if"),    sslice_static_new("else"),     sslice_static_new("mut"),
      sslice_static_new("when"),  sslice_static_new("fn"),       sslice_static_new("struct"),
      sslice_static_new("trait"), sslice_static_new("and"),      sslice_static_new("or"),
      sslice_static_new("impl"),  sslice_static_new("return"),   sslice_static_new("self"),
      sslice_static_new("const"), sslice_static_new("loop"),     sslice_static_new("for"),
      sslice_static_new("while"), sslice_static_new("continue"), sslice_static_new("break"),
      sslice_static_new("match"), sslice_static_new("pub"),      sslice_static_new("ref"),
      sslice_static_new("error"), sslice_static_new("enum"),     sslice_static_new("type"),
      sslice_static_new("await"), sslice_static_new("comptime"), sslice_static_new("static"),
      sslice_static_new("mod"),   sslice_static_new("macro"),    sslice_static_new("derive"),
      sslice_static_new("dyn"),   sslice_static_new("default"),  sslice_static_new("sizeof"),
  };

  for (i32 i = 0; i < Token__KeywordCount; i++) {
    const sslice sl = KEYWORDS[i];
    const Keyword* kw = kw_lookup_str(sl.begin, sl.len);

    {
      static constexpr char FMT[] = "Input: %s :: %d";
      const i32 len = snprintf(nullptr, 0, FMT, sl.begin, sl.len) + 1;
      char pf[len] = {};
      snprintf(pf, len, FMT, sl.begin, sl.len);

      TEST_ASSERT_NOT_NULL_MESSAGE(kw, pf);
    }



    // check slice variant too just in case :D
    const Keyword* kw2 = kw_lookup(sl);
    TEST_ASSERT_NOT_NULL(kw2);

    TEST_ASSERT_EQUAL_PTR(kw, kw2);

    TEST_ASSERT_EQUAL_STRING(kw_string(kw), sl.begin);
  }
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
  RUN_TEST(keyword_lookup_table);

  return UNITY_END();
}
