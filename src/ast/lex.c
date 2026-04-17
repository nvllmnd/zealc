#include "ast/lex.h"
#include "memory/cstr.h"

static const char* LEX_ERROR_STRINGS[_LexError__Count] = {
    STRINGIFY(LexError__Ok), STRINGIFY(LexError__UnexpectedCharacter), STRINGIFY(LexError__UnmatchedDoubleQuotString),
    STRINGIFY(LexError__UnmatchedSingleQuotString), STRINGIFY(LexError__IllegalIdentifier), STRINGIFY(LexError__FailedToOpenFile),
  STRINGIFY(LexError__InnerFileIO),};


sslice lex_error_sslice(LexError err) {
    return sslice_static_new(LEX_ERROR_STRINGS[err]);
}

const char* lex_error_string(LexError err) {
    const sslice s = lex_error_sslice(err);
    return s.begin;
}
