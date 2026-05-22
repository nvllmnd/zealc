#pragma once

#include "ast/token.h"
#include "nv/core/attributes.h"
#include "nv/core_types.h"

typedef enum LexError {
  /// No Error! OK!
  LexError__Ok = 0,
  LexError__UnexpectedCharacter,
  LexError__UnexpectedCharacterInNumericToken,
  LexError__UnmatchedDoubleQuotString,
  LexError__UnmatchedSingleQuotString,
  LexError__FailedToParseFloat0,
  LexError__FailedToParseInt0,
  LexError__FloatParseFail,
  LexError__IntegerParseFail,
  LexError__UnexpectedRunePrefix,
  /// Lexer encountered a '\' character in an unexpected/illegal location!
  LexError__UnexpectedEscapeCharacter,
  /// Lexer encountered an EOF/end of source string unexpectedly in the middle of tokenization
  LexError__UnexpectedEndOfSource,
  /// i.e. let x = 1.5.7 // too many '.'!
  LexError__InvalidFloatingPointLiteral,
  /// Invalid identifer: I.E. one that starts with a number: let 7asdf = 5;
  LexError__IllegalIdentifier,
  LexError__FailedToOpenFile,
  LexError__InnerFileIO,

  /// Some inner lexer function expected a TokenType value in a certain range,
  /// but received one out of that range. i.e. a fuction expected a keyword token type (between Token__KeywordStart -
  /// Token__KeywordEnd)
  /// This is most likely a logic error in the lexer implementation
  LexError__TokenTypeOutOfRange,

  /// used internally
  _LexError__Count,

} LexError;

struct LexState {
  /// slice to source file / eval string being currently lexed
  sslice source;

  /// current position in source file
  Cursor cursor;
};
typedef struct LexState LexState;

/// Returns a pointer to a static null-terminated string representing
/// the display value of given glyph token type.
///
/// If given @param (tt) is outside of the range of [Token__GlyphStart] and [Token__GlyphEnd],
///
/// Then a stringified version of one of the relevant negative TokenType variants, i.e.
/// [Token__CannotGetStringOfNonGlyph]
///
CONST_FUNC
RETURNS_NON_NULL
const char* tokentype_glyph_string(TokenType tt);

CONST_FUNC
RETURNS_NON_NULL
const char* tokentype_keyword_string(TokenType tt);

CONST_FUNC
sslice tokentype_keyword_slice(TokenType tt);

CONST_FUNC
sslice tokentype_glyph_sslice(TokenType tt);

CONST_FUNC
sslice lex_error_sslice(LexError err);

CONST_FUNC
RETURNS_NON_NULL
const char* tokentype_string(TokenType tt);

CONST_FUNC
RETURNS_NON_NULL
const char* lex_error_string(LexError err);

/// Helper function for checking that a token that was just filled in by [lexer_next]
/// is not Eof and the error returned by it (passed into this function's second param) is LexError__Ok (null/nil/none/0)
PURE_FUNC
PARAMS_NONNULL(1)
static inline bool lex_check_ok(const Token* current_token, LexError current_error) {
  return (current_token->type != Token__Eof && current_error == LexError__Ok);
}

/// Initialize a new LexState to tokenize a given string of zeal source code
METHOD
void lexer_init_source(LexState* self, sslice source_string);
// LexError lexer_file_init(LexState* self, sslice src_filepath);
// LexError lexer_memory_init(LexState* self, sslice src);

/// Helper method for retargeting a lexer that has finished tokenizing a source string to
/// tokenize a new source string
METHOD
void lexer_reset_source(LexState* self, sslice source_string);

// METHOD
PARAMS_NONNULL(1, 2)
LexError lexer_next(LexState* self, Token* next_token);

/// Peeks next token by returning a new [LexState], where calling [lexer_next] on
/// returned state will allow for peeking next token without modifying current lexer state
METHOD
LexState lexer_peek_next(const LexState* self);

// #define USING(init, freer) \
//   for(int __i__ = 0; __i__ == 0; __i__++) \
//     for (init; __i__ == 0; __i__++, freer((x)))
                        
struct LexIter {
  Token tok;
  LexError err;
};
typedef struct LexIter LexIter;
  

#define LEXER_FOREACH(self, ctx) /* a convieneince macro for iterating over a source string given a lexer and a name \
                                    for the variable of the context struct that contains the current [Token] and a   \
                                    [LexError] value. You can also use the [LEXER_FOREACH] macro that is the same    \
                                    thing as this macro, but defaults the struct value name to be 'ctx' */           \
  for (LexIter ctx = {.tok = {}, .err = LexError__Ok};                                                                     \
       lex_check_ok(&ctx.tok, ctx.err); ctx.err = lexer_next(&self, &ctx.tok))

#define LEXER_FOREACH_CTX(self) /* Same as the [LEXER_FOREACH] macro, but provides 'ctx' as its second parameter, \
                                   making the context struct variable name: 'ctx' */                              \
  LEXER_FOREACH((self), ctx)
