#include <stdio.h>
#include "ast/lex.h"
#include "log.h"
#include "memory/cstr.h"

#define PROJECT_NAME "zeal"

int main(void) {
    
 static constexpr const char LANG_VALID_INPUT[] = "let x = 155;\nfn do_thing(n: i32) i32 {\n    return n;\n}\n   ";
 static const sslice LANG_VALID_INPTU_SLICE = sslice_static_new(LANG_VALID_INPUT);

  LexState lex = {};
  lexer_init_source(&lex, sslice_static_new(LANG_VALID_INPUT));

  Token t = {};
  LexError err = LexError__Ok;


  SLOG_DBG(LANG_VALID_INPTU_SLICE);

  while(t.type != Token__Eof && err == LexError__Ok) {
    err = lexer_next(&lex, &t);
    SLOG_DBG(t.lexeme);
  }

  if (err != LexError__Ok) {
      LOG_DBG("LEX ERROR: %s", lex_error_string(err));
  }
  
  
}
