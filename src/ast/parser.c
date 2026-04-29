#include "ast/parser.h"
#include "ast/expr.h"
#include "ast/lex.h"

// struct ParseError {
   
// };
// alias(ParseError);

struct Parser {
  LexState lex;
  struct AstList {
    Expr* start;
    i32 len; 
  } tree;
};
alias(Parser);
alias(AstList);

[[maybe_unused]]
static Expr expression(Parser* lex);

[[maybe_unused]]
static Expr assignment(Parser* lex);

[[maybe_unused]]
static Expr logical_bitwise(Parser* lex);

[[maybe_unused]]
static Expr comparison(Parser* lex);

[[maybe_unused]]
static Expr term(Parser* lex);

[[maybe_unused]]
static Expr factor(Parser* lex);

[[maybe_unused]]
static Expr unary(Parser* lex);



[[maybe_unused]]
static Expr equality(Parser* lex);

[[maybe_unused]]
static Expr primary(Parser* lex);

Expr* parse_source_str(const char* str, isize len) {

  LexState lex = {};

  const sslice src = sslice_new(str, len);

  lexer_init_source(&lex, src);

  
  
  return nullptr;
}




