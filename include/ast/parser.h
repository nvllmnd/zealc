#pragma once


#include "ast/lex.h"
#include "ast/token.h"
#include "intdefs.h"

struct ParseState {
  LexState lex;
  Token prev;
  Token current;
};
alias(ParseState);

struct Expr* parse_source_str(const char* str, isize len);








