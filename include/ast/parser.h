#pragma once

#include "memory/cstr.h"


struct Cursor {
  /// index into whatever buffer this cursor is iterating over
  isize i;
  /// row of item being iterated over
  isize row;
  /// column of item being iterated over
  isize col;  
};
typedef struct Cursor Cursor;


struct LexState {
  /// slice to source file / eval string being currently lexed 
  sslice source;

  Cursor cursor;
  /// The current lexeme being lexed
  sslice lexeme; 
};
typedef struct LexState LexState;

struct Parser {
  LexState lex;
  struct Expr* ast;
};
typedef struct Parser Parser;



