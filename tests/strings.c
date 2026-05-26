

#include "ast/token.h"
#include "core/runes.h"
#include "nv.h"
#include "nv/core/algo.h"
#include "unity.h"

void setUp(void) {
  const ZError err = runetab_init(KILOBYTES(1), 4);
  if (err != ZOK) {
    LOG("Failed to initialize RuneTable! ERR: %d", err);
  }
}

void tearDown(void) { runetab_destroy(); }

struct KW {
  sslice name;
  KeywordType type;
};
alias(KW);

static constexpr const KW KWS[] = {
    make(KW, .name = sslice_static_new("if"), .type = Keyword__If),
    make(KW, .name = sslice_static_new("struct"), .type = Keyword__Struct),

    make(KW, sslice_static_new("while"), Keyword__While),

    make(KW, sslice_static_new("else"), Keyword__Else),

    make(KW, sslice_static_new("when"), Keyword__When),

    make(KW, sslice_static_new("trait"), Keyword__Trait),

    make(KW, sslice_static_new("const"), Keyword__Const),

    make(KW, sslice_static_new("let"), Keyword__Let),

    make(KW, sslice_static_new("fn"), Keyword__Fn),

    make(KW, sslice_static_new("return"), Keyword__Return),
    make(KW, sslice_static_new("loop"), Keyword__Loop),

};

void runetab_keywords(void) {
  static constexpr const i32 LEN = sizeof(KWS) / sizeof(KW);
  for (i32 i = 0; i < LEN; i++) {
    const Rune kw = runetab_lookup(KWS[i].name);
    TEST_ASSERT_NOT_NULL(kw.name.begin);
    TEST_ASSERT_NOT_EQUAL(0, kw.name.len);
    TEST_ASSERT_EQUAL(KWS[i].type, kw.kwtype);
    TEST_ASSERT_EQUAL_STRING_LEN(KWS[i].name.begin, kw.name.begin, KWS[i].name.len);
  }
}

void runetab_add_and_lookup(void) {
  static constexpr const sslice VALUE = sslice_static_new("value");
  const Rune a = runetab_add(VALUE);
  TEST_ASSERT_FALSE(rune_is_none(a));
  TEST_ASSERT_EQUAL_STRING_LEN(VALUE.begin, a.name.begin, VALUE.len);

  const Rune b = runetab_lookup(VALUE);
  TEST_ASSERT_TRUE(rune_eq(a, b));
  TEST_ASSERT_EQUAL_PTR(b.name.begin, a.name.begin);
}

void move_memory_helpers(void) {
  typedef struct Resource {
    const char* buf;
  } Resource;

  {
    static const char* INPUT = "test";
    static const char* OLD = "old value";

    Resource a = make(Resource, INPUT);
    Resource b = make_zeroed(Resource);

    b.buf = move_exchange(a.buf, OLD);

    TEST_ASSERT_EQUAL_STRING(b.buf, INPUT);
    TEST_ASSERT_EQUAL_STRING(a.buf, OLD);
  }
  {
    static const char* INPUT = "input value";

    Resource a = make(Resource, INPUT);
    Resource b = make_zeroed(Resource);

    b.buf = move(a.buf);

    TEST_ASSERT_EQUAL_STRING(b.buf, INPUT);
    TEST_ASSERT_NULL(a.buf);
  }

  {
    static const char* INPUT = "input value";

    Resource a = make(Resource, INPUT);
    Resource b = make_zeroed(Resource);

    static constexpr const char* none = nullptr;
    b.buf = move_exchange(a.buf, none);

    TEST_ASSERT_EQUAL_STRING(b.buf, INPUT);
    TEST_ASSERT_NULL(a.buf);
  }

  {
    static const char* INPUT = "input value";

    Resource a = make(Resource, INPUT);
    Resource b = make_zeroed(Resource);

    move_into(a.buf, b.buf);

    TEST_ASSERT_EQUAL_STRING(b.buf, INPUT);
    TEST_ASSERT_NULL(a.buf);
  }
}

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

      sslice_static_new("print"), sslice_static_new("println"),
  };


  for (i32 i = 0; i < Token__KeywordCount; i++) {
    const sslice sl = KEYWORDS[i];
    const Keyword* kw = kw_lookup_str(sl.begin, sl.len);

    TEST_ASSERT_NOT_NULL(kw);

    // check slice variant too just in case :D
    const Keyword* kw2 = kw_lookup(sl);
    TEST_ASSERT_NOT_NULL(kw2);

    TEST_ASSERT_EQUAL_PTR(kw, kw2);

    TEST_ASSERT_EQUAL_STRING(kw_string(kw), sl.begin);
  }
}
i32 main(void) {
  UNITY_BEGIN();

  RUN_TEST(keyword_lookup_table);
  RUN_TEST(move_memory_helpers);
  RUN_TEST(runetab_keywords);
  RUN_TEST(runetab_add_and_lookup);

  return UNITY_END();
}
