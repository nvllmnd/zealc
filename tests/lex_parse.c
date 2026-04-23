#include "ast/lex.h"
#include "ast/token.h"
#include "core_types.h"
#include "log.h"
#include "memory/cstr.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}



/// we can put a global allocator in an [Allocator]
/// struct and everything works just fine
void lex_can_tokenize(void) {
  static constexpr const char LANG_VALID_INPUT[] = "let x = 155;\nfn do_thing(n: i32) i32 {\n    return n;\n}\n   ";

  LexState lex = {};
  lexer_init_source(&lex, sslice_static_new(LANG_VALID_INPUT));

  Token t = {};
  LexError err = LexError__Ok;

  while(t.type != Token__Eof && err == LexError__Ok) {
    err = lexer_next(&lex, &t); 
    TEST_ASSERT_EQUAL_INT32_MESSAGE(LexError__Ok, err, "Lexer Error!");
    sprintln(t.lexeme);
  }
  
  TEST_ASSERT_EQUAL_INT32_MESSAGE(Token__Eof, t.type, "Lexer did not reach Eof!");
  
}

i32 main(void) {
  UNITY_BEGIN();

  RUN_TEST(lex_can_tokenize);

  return UNITY_END();
}
