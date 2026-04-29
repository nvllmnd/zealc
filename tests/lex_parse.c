#include "ast/lex.h"
#include "ast/symbol.h"
#include "ast/token.h"
#include "core_types.h"
#include "log.h"
#include "memory/cstr.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

/// we can put a global allocator in an [Allocator]
/// struct and everything works just fine
void lex_can_tokenize(void) {
  static constexpr const char LANG_VALID_INPUT[] = "let x = 155; let s = \"ayoo\"; \nfn do_thing(n: i32) i32 {\n return n;\n}\n   ";
  // static constexpr const char LANG_VALID_INPUT[] =
  //     "let x = 155;"
  //     "let s = \"ayoo\";"
  //     "fn do_thing(n: i32) i32 {"
  //     "    return n;"
  //     "}";

  LexState lex = {};
  lexer_init_source(&lex, sslice_static_new(LANG_VALID_INPUT));

  LEXER_FOREACH(lex, ctx) {
    if (tokentype_is_keyword(ctx.tok.type)) {
      const sslice lexeme = ctx.tok.lexeme;
      const Keyword* kw = kw_lookup_str(lexeme.begin, lexeme.len);
      TEST_ASSERT_NOT_NULL(kw);
    }

    TEST_ASSERT_EQUAL(ctx.err, LexError__Ok);
  }

  lexer_reset_source(&lex, sslice_static_new(LANG_VALID_INPUT));

  LEXER_FOREACH(lex, ctx) {
    SLOG_DBG(ctx.tok.lexeme);
    TEST_ASSERT_EQUAL(ctx.err, LexError__Ok);
  }
}

i32 main(void) {
  UNITY_BEGIN();

  RUN_TEST(lex_can_tokenize);

  return UNITY_END();
}
