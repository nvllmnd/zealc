#pragma once


#include "memory/cstr.h"
typedef enum LexErrorType {
  /// No Error! OK!
  LexError__Ok = 0,
  LexError__UnexpectedCharacter,
  LexError__UnmatchedDoubleQuotString,
  LexError__UnmatchedSingleQuotString,
  /// Invalid identifer: I.E. one that starts with a number: let 7asdf = 5;
  LexError__IllegalIdentifier,

} LexError;


CONST_FUNC
sslice lex_error_sslice(LexError err);

CONST_FUNC
const char* lex_error_string(LexError err);

