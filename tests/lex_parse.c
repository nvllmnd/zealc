#include "ast/ast.h"
#include "ast/expr.h"
#include "ast/lex.h"
#include "ast/parser.h"
#include "ast/token.h"
#include "core/runes.h"
#include "nv/memory/arena.h"
#include "strpad.h"
#define LIBNV_DEBUG 1
#include "nv.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

/// we can put a global allocator in an [Allocator]
/// struct and everything works just fine
void lex_can_tokenize(void) {
  static constexpr const char LANG_VALID_INPUT[] =
      "let x = 155; let s = \"ayoo\"; \nfn do_thing(n: i32) i32 {\n return n;\n}\n   ";

  static constexpr const char INPUT[] = "5 + 5 * 10 - 600 + 200;";

  LexState lex = {};
  lexer_init_source(&lex, sslice_static_new(LANG_VALID_INPUT));

  LOG("#### Testing Keywords ####");

  LEXER_FOREACH(lex, ctx) {
    if (tokentype_is_keyword(ctx.tok.type)) {
      const sslice lexeme = ctx.tok.lexeme;
      const Keyword* kw = kw_lookup_str(lexeme.begin, lexeme.len);
      TEST_ASSERT_NOT_NULL(kw);
    }

    TEST_ASSERT_EQUAL(ctx.err, LexError__Ok);
  }

  lexer_reset_source(&lex, sslice_static_new(LANG_VALID_INPUT));

  LOG("#### Printing Tokens ####");
  LEXER_FOREACH(lex, ctx) {
    SLOG_DBG(ctx.tok.lexeme);
    TEST_ASSERT_EQUAL(ctx.err, LexError__Ok);
  }

  lexer_reset_source(&lex, sslice_static_new(INPUT));

  LEXER_FOREACH(lex, ctx) {
    SLOG_DBG(ctx.tok.lexeme);
    TEST_ASSERT_EQUAL(ctx.err, LexError__Ok);
  }
}



void parse_simple_ast(void) {

  static constexpr const char INPUT[] = "let x = 5 + (5 * 10) - (600 + 200);\nprintln x;\n\n";

  Arena* arena = arena_new(MEGABYTES(128), KILOBYTES(16));

  TEST_ASSERT_NOT_NULL(arena);

  Parser p = parser_new(arena);

  Ast ast = parser_parse_ast(&p, INPUT, sizeof(INPUT));
  TEST_ASSERT_TRUE(ast.root->type != ExprStmt__None);

  TEST_ASSERT_NOT_NULL(ast.alloc);
  TEST_ASSERT_NOT_NULL(ast.root);

  const sslice sl = ast_stringify(&ast);

  TEST_ASSERT_NOT_NULL(sl.begin);

  TEST_ASSERT_EQUAL_STRING("(let x (- (+ 5 (* 5 10)) (+ 600 200)))\n(println x)\n", sl.begin);
  println("%.*s", RSSPREAD(sl));

}

i32 main(void) {
  UNITY_BEGIN();

  RUN_TEST(lex_can_tokenize);
  RUN_TEST(parse_simple_ast);

  return UNITY_END();
}
