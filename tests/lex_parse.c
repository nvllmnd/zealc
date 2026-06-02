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
  // static constexpr const char INPUT[] =
  // "let x = 50;\n let y = 100;\n {\n let z = 500;\n }\n println x;\n println y;\n println z;\n";
  static constexpr const char INPUT[] = "5 + (5 * 10) - (600 + 200);";
  Arena* arena = arena_new(MEGABYTES(128), KILOBYTES(16));
  TEST_ASSERT_NOT_NULL(arena);

  Parser p = parser_new(arena);

  // Ast ast = parser_parse_ast(&p, INPUT, sizeof(INPUT));

  // TEST_ASSERT_NOT_NULL(ast.alloc);
  // TEST_ASSERT_NOT_NULL(ast.root);

  // const sslice sl = ast_stringify(&ast);

  // TEST_ASSERT_NOT_NULL(sl.begin);

  Expr* e = parser_parse_expr(&p, &INPUT[0], sizeof(INPUT));
  print_expression(e);
  TEST_ASSERT_NOT_NULL(e);
}

i32 main(void) {
  UNITY_BEGIN();

  RUN_TEST(lex_can_tokenize);
  RUN_TEST(parse_simple_ast);

  return UNITY_END();
}
