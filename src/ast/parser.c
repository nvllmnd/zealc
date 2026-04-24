#include "ast/parser.h"
#include "ast/expr.h"
#include "ast/lex.h"



Expr* parse_source_str(const char* str, isize len) {

  LexState lex = {};

  const sslice src = sslice_new(str, len);

  lexer_init_source(&lex, src);

  
  
  return nullptr;
}




