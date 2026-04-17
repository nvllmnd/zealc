#pragma once


#include "memory/cstr.h"

typedef enum LexError {
  /// No Error! OK!
  LexError__Ok = 0,
  LexError__UnexpectedCharacter,
  LexError__UnmatchedDoubleQuotString,
  LexError__UnmatchedSingleQuotString,
  /// Invalid identifer: I.E. one that starts with a number: let 7asdf = 5;
  LexError__IllegalIdentifier,
  LexError__FailedToOpenFile,
  LexError__InnerFileIO,

  /// used internally
  _LexError__Count,

} LexError;

struct Cursor {
  /// index into whatever buffer this cursor is iterating over
  isize i;
  /// row of item being iterated over
  isize row;
  /// column of item being iterated over
  isize col;  
};
typedef struct Cursor Cursor;

/// asdf
typedef Cursor SourceLocation;

struct LexState {
  /// slice to source file / eval string being currently lexed 
  sslice source;

  Cursor cursor;
  /// The current lexeme being lexed
  sslice lexeme; 
};
typedef struct LexState LexState;

CONST_FUNC
sslice lex_error_sslice(LexError err);

CONST_FUNC
const char* lex_error_string(LexError err);


LexError lexer_file_init(LexState* self, sslice src_filepath);
LexError lexer_memory_init(LexState* self, sslice src);

LexError lexer_next(LexState* lex);

