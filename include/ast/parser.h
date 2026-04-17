#pragma once

#include "lex.h"


struct Parser {
  LexState lex;
  struct Expr* ast;
};
typedef struct Parser Parser;



