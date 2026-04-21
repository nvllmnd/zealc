#include "ast/lex.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "ast/token.h"
#include "attributes.h"
#include "memory/cstr.h"

// static constexpr const char INVALID_CHAR = cast(char, -125);
// static constexpr const char LEXER_EOF_LITERAL = cast(char, -128);

METHOD
static inline void lexer_adv(LexState* self) {
  self->cursor.i += 1;
  self->cursor.col += 1;
}

METHOD
static inline void lexer_adv_nl(LexState* self) {
  self->cursor.i += 1;
  self->cursor.row += 1;
  self->cursor.col = 0;
}

METHOD
static inline sslice lexer_slice(const LexState* self, i32 start, i32 end) {
  return sslice_from_range(self->source.begin, start, end);
}

METHOD
PURE_FUNC
static inline char lexer_peekc(const LexState* self) { return self->source.begin[self->cursor.i]; }

METHOD
PURE_FUNC
static inline char lexer_peekc_next(const LexState* self) {
  assert(self->cursor.i + 1 < self->source.len);
  return self->source.begin[self->cursor.i + 1];
}

// METHOD
// static inline char lexer_peek_adv(LexState* self) {
//   lexer_adv(self);
//   return lexer_peekc(self);
// }

METHOD
PURE_FUNC
static inline bool lexer_is_eof(const LexState* self) {
  const i32 i = self->cursor.i;
  return i >= self->source.len || i < 0;
}

METHOD
PURE_FUNC
static inline bool lexer_next_is_eof(const LexState* self) {
  const i32 i = self->cursor.i + 1;
  return i >= self->source.len || i < 0;
}

METHOD
static inline void lexer_whitespace_skip(LexState* self) {
  while (!lexer_is_eof(self) && isspace(lexer_peekc(self))) {
    if (lexer_peekc(self) == '\n') {
      lexer_adv_nl(self);
    } else {
      lexer_adv(self);
    }
  }
}

PARAMS_NONNULL(1, 2)
static LexError lexer_integer(LexState* self, Token* tok);

PARAMS_NONNULL(1, 2)
static LexError lexer_identifier(LexState* self, Token* tok);

PARAMS_NONNULL(1, 2)
static LexError lexer_rune(LexState* self, Token* tok);

PARAMS_NONNULL(1, 3)
static LexError lexer_glyph(LexState* self, TokenType tt, Token* tok);

static const char* LEX_ERROR_STRINGS[_LexError__Count] = {
    STRINGIFY(LexError__Ok),
    STRINGIFY(LexError__UnexpectedCharacter),
    STRINGIFY(LexError__UnmatchedDoubleQuotString),
    STRINGIFY(LexError__UnmatchedSingleQuotString),
    STRINGIFY(LexError__IllegalIdentifier),
    STRINGIFY(LexError__FailedToOpenFile),
    STRINGIFY(LexError__InnerFileIO),
};

CONST_FUNC
sslice lex_error_sslice(LexError err) { return sslice_static_new(LEX_ERROR_STRINGS[err]); }

const char* lex_error_string(LexError err) {
  const sslice s = lex_error_sslice(err);
  return s.begin;
}

METHOD
void lexer_init_source(LexState* self, sslice source_string) {
  self->source = source_string;
  self->cursor = (Cursor){};
}

METHOD
PURE_FUNC
static inline bool lexer_is_rune(const LexState* self) {
  if (UNLIKELY(lexer_is_eof(self) || lexer_next_is_eof(self))) {
    return false;
  }

  const char c = lexer_peekc(self);
  const char next = lexer_peekc_next(self);

  return (c == ':' && (isalpha(next) || next == '_'));
}

METHOD
PURE_FUNC
static inline bool lexer_is_alpha_or_uscore(const LexState* self) {
  if (UNLIKELY(lexer_is_eof(self))) {
    return false;
  }
  const char c = lexer_peekc(self);
  return isalpha(c) || c == '_';
}

LexError lexer_next(LexState* self, Token* next_token) {
  lexer_whitespace_skip(self);
  *next_token = (Token){};

  const char c = lexer_peekc(self);

  switch (c) {
    case Token__OpenBrace: {
      if (UNLIKELY(lexer_next_is_eof(self))) {
        // something probably wrong if source string ends with '{'
        return LexError__UnexpectedEndOfSource;
      }
      const char next = lexer_peekc_next(self);
      if (next == Token__CloseBrace) {
        return lexer_glyph(self, Token__EmptyBrace, next_token);
      }
    } break;
    case Token__CloseBrace: {
    } break;
    case Token__OpenParen: {
    } break;
    case Token__CloseParen: {
    } break;
    case Token__OpenBracket: {
    } break;
    case Token__CloseBracket: {
    } break;
    case Token__Period: {
    } break;
    case Token__Eq: {
    }

    case Token__DoubleQuot: {
    } break;

    case Token__SingleQuot: {
    } break;
    case Token__Bang: {
    } break;
    case Token__Percent: {
    } break;
    case Token__ChevronUp: {
    } break;
    case Token__Ampersand: {
    } break;
    case Token__Star: {
    } break;
    case Token__Minus: {
    } break;
    case Token__Plus: {
    } break;

    case Token__UnaryUnderscore: {
    } break;

    case Token__Pipe: {
    } break;
    case Token__Comma: {
    } break;
    case Token__Gt: {
    } break;
    case Token__Lt: {
    } break;
    case Token__Semicolon: {
    } break;

    case Token__Colon: {
    } break;
    case Token__BackSlash: {
    } break;
    case Token__ForwardSlash: {
    } break;
    case Token__QMark: {
    } break;

    default: {
    } break;
  }

  if (lexer_is_rune(self)) {
    return lexer_rune(self, next_token);

  } else if (lexer_is_alpha_or_uscore(self)) {
    // switch (c) {
    //   case 't': {} break;
    //   case 'f': {} break;
    // }

    const LexError err = lexer_identifier(self, next_token);
    if (err == LexError__Ok) {
      // const sslice lexeme = next_token->lexeme;

      // if (sslice_eq(lexeme, sslice_static_new("true"))) {

      // }

    } else {
      return err;
    }

  } else if (isdigit(c)) {
    return lexer_integer(self, next_token);

  } else {
  }

  return LexError__UnexpectedCharacter;
  // } else if (ispunct(c)) {

  // }

  // switch (c) {

  // }
}

/// Peeks next token by returning a new [LexState], where calling [lexer_next] on
/// returned state will allow for peeking next token without modifying current lexer state
LexState lexer_peek_next(const LexState* self) {
  UNUSED(self);
  return (LexState){};
}

static const char* TOKEN_TYPE_KEYWORD_STRINGS[Token__KeywordCount] = {

    // Token__True,
    "true",
    // Token__False,
    "false",
    // Token__Let,
    "let",
    // Token__If,
    "if",
    // Token__Else,
    "else",
    // Token__Mut,
    "mut",
    // Token__When,
    "when",
    // Token__Fn,
    "fn",
    // Token__Struct,
    "struct",
    // Token__Trait,
    "trait",
    // Token__Impl,
    "impl",
    // Token__And,
    "and",
    // Token__Or,
    "or",
    // Token__Return,
    "return",
    // Token__Self,
    "self",
    // Token__Const,
    "const",
    // Token__Loop,
    "loop",
    // Token__For,
    "for",
    // Token__While,
    "while",
    // Token__Break,
    "break",
    // Token__Match,
    "match",
    // Token__Continue,
    "continue",
    // Token__Pub,
    "pub",
    // Token__Ref,
    "ref",
    // Token__Error,
    "error",
    // Token__Enum,
    "enum",
    // Token__Type,
    "type",
    // Token__Await,
    "await",
    // Token__Comptime,
    "comptime",
    // Token__Static,
    "static",
    // Token__Mod,
    "mod",
    // Token__Macro,
    "macro",
    // Token__Derive,
    "derive",
    // Token__Dyn,
    "dyn",
    // Token__Default,
    "default"};

static const char* TOKEN_TYPE_GLYPH_STRINGS[Token__GlyphsCount] = {
    // Token__StarEq,
    "*=",
    // Token__AmpersandEq,
    "&=",
    // Token__PercentEq,
    "%=",
    // Token__BangEq,
    "!=",
    // Token__DoubleEq,
    "==",
    // Token__PlusEq,
    "+=",
    // Token__MinusEq,
    "-=",
    // Token__DblQMark,
    "??",
    // Token__DblForwardSlash,
    "//",
    // Token__ForwardSlashEq,
    "/=",
    // Token__DblBackSlash,
    "\\",
    // Token__DblColon,
    "::",
    // Token__LtEq,
    "<=",
    // Token__GtEq,
    ">=",
    // Token__Elipses,
    "...",
    // Token__DblPipe,
    "||",
    // Token__EmptyParen,
    "()",
    // Token__EmptyBracket,
    "[]",
    // Token__EmptyBrace,
    "()",
    // Token__ArrowRight,
    "->",
    // Token__ArrowLeft,
    "<-",
    // Token__FatArrow,
    "=>"};

static constexpr const char GLYPH_ERR_STRING[] = STRINGIFY(Token__CannotGetStringOfNonGlyph);
static constexpr const char KEYWORD_ERR_STRING[] = STRINGIFY(Token__CannotGetStringOfNonKeyword);

const char* tokentype_keyword_string(TokenType tt) {
  if (tt > Token__KeywordsStart && tt < Token__KeywordsEnd) {
    const i32 i = (tt - (Token__KeywordsStart + 1));
    assert(i >= 0 && i < Token__KeywordsEnd);
    return TOKEN_TYPE_KEYWORD_STRINGS[i];
  } else {
    return KEYWORD_ERR_STRING;
  }
}

sslice tokentype_keyword_slice(TokenType tt) {
  const char* s = tokentype_keyword_string(tt);
  if (s == KEYWORD_ERR_STRING) {
    return sslice_static_new(KEYWORD_ERR_STRING);
  }

  const i32 len = stringlen(s);
  return sslice_new(s, len);
}

sslice tokentype_glyph_sslice(TokenType tt) {
  const char* s = tokentype_glyph_string(tt);
  if (s == GLYPH_ERR_STRING) {
    return sslice_static_new(GLYPH_ERR_STRING);
  }

  const i32 len = stringlen(s);
  return sslice_new(s, len);
}

const char* tokentype_glyph_string(TokenType tt) {
  if (tt > Token__GlyphStart && tt < Token__GlyphsEnd) {
    const i32 i = (tt - (Token__GlyphStart + 1));
    assert(i >= 0 && i < Token__GlyphsCount);
    return TOKEN_TYPE_GLYPH_STRINGS[i];
  } else {
    return STRINGIFY(Token__CannotGetStringOfNonGlyph);
  }
}

/// Returns a slice from current lexer token index + len,
///
/// An assert checks if len + self->cusor.i > self->source.len
/// in debug builds, passing an index that would be out of range would
/// result in UB otherwise
// static inline sslice lexer_peek_slice(LexState* self, i32 len) {
//   const i32 start = self->cursor.i;
//   const i32 end = start + len;

//   assert(end < self->source.len);

//   return lexer_slice(self, start, end);
// }

// static bool lexer_match_keyword(LexState* self, i32 start, sslice rest, TokenType expected, Token* tok) {
//   const i32 end = self->cursor.i + rest.len;

//   assert(end < self->source.len);

//   // const i32 src_start = self->cursor.i;
//   // const i32 src_end = self->cursor.i + rest.len;

//   const sslice src = lexer_peek_slice(self, rest.len);
//   if (sslice_eq(src, rest)) {
//     tok->type = expected;
//     tok->lexeme = lexer_slice(self, start, end);
//     tok->loc = self->cursor;
//   }

//   return Token__Identifier;
// }

// static inline TokenType check_keyword(sslice ident, i32 offset, sslice rest, TokenType expected) {
//   assert(ident.begin && ident.len > 0 && offset <= ident.len && offset >= 0);

//   const sslice src = sslice_from_range(ident.begin, offset, ident.len);
//   if (sslice_eq(src, rest)) {
//     return expected;
//   }
//   return Token__Identifier;
//   // const char* start = ident.begin + offset;
//   // const i32 len = ident.len - offset;
// }


PARAMS_NONNULL(1, 2)
static LexError lexer_identifier(LexState* self, Token* tok) {
  const i32 start = self->cursor.i;
  char c = lexer_peekc(self);

  // // check the beginning of this token is indeed a valid identifier
  // // valid idents can begin with underscore or alpha character
  if (isalpha(c) || c == '_') {
    do {
      lexer_adv(self);
      c = lexer_peekc(self);
    } while (isalnum(c) || c == '_');

    const i32 end = self->cursor.i - 1;
    tok->lexeme = lexer_slice(self, start, end);
    tok->loc = self->cursor;

    // tok->type = Token__Identifier;
    return LexError__Ok;
  } else {
    // otherwise this is not a valid identifier
    return LexError__IllegalIdentifier;
  }
}

METHOD
static LexError lexer_scan_float(LexState* self) {
  char c = lexer_peekc(self);

  while (isdigit(c)) {
    lexer_adv(self);
    c = lexer_peekc(self);

    // we already had to match a '.' to get into this function, so if we find another
    // one, that means this is not a proper/valid float literal
    if (c == '.') {
      return LexError__InvalidFloatingPointLiteral;
    }

    // marks this a float literal, like in c/c++: 1.0f
    if (c == 'f') {
      lexer_adv(self);
      return LexError__Ok;
    }
  }

  return LexError__Ok;
}

PARAMS_NONNULL(1, 2)
static LexError lexer_float(LexState* self, Token* tok, i32 start) {
  LexError err = lexer_scan_float(self);

  if (UNLIKELY(err != LexError__Ok)) {
    return err;
  }

  const i32 end = self->cursor.i;

  const sslice lexeme = lexer_slice(self, start, end);

  tok->type = Token__Float;
  tok->loc = self->cursor;
  tok->lexeme = lexeme;

  char parse_buf[lexeme.len + 1] = {};
  strncpy(parse_buf, lexeme.begin, lexeme.len);

  char* parse_end = parse_buf;

  f64 lit = strtod(parse_buf, &parse_end);

  if (((lit == 0.0f) && (parse_end == parse_buf)) || errno == ERANGE) {
    return LexError__FloatParseFail;
  }

  tok->literal.fp = lit;

  return LexError__Ok;
}

PARAMS_NONNULL(1, 2)
static LexError lexer_integer(LexState* self, Token* tok) {
  const i32 start = self->cursor.i;

  {
    char c = lexer_peekc(self);
    while (isdigit(c)) {
      lexer_adv(self);
      c = lexer_peekc(self);

      if (c == '.') {
        lexer_adv(self);
        return lexer_float(self, tok, start);
      }
    }
  }

  const i32 end = self->cursor.i;

  const sslice lexeme = lexer_slice(self, start, end);

  char parse_buf[lexeme.len + 1] = {};
  strncpy(parse_buf, lexeme.begin, lexeme.len);

  tok->type = Token__Int;
  tok->loc = self->cursor;
  tok->lexeme = lexeme;

  char* parse_end = parse_buf;

  const i64 lit = strtoll(parse_buf, &parse_end, 10);

  if ((lit == 0 && parse_end == parse_buf) || errno == ERANGE) {
    return LexError__IntegerParseFail;
  }

  tok->literal.integer = lit;

  return LexError__Ok;
}

// static LexError lexer_number(LexState* self, Token* tok) {
//   const i32 start = self->cursor.i;
//   char c = lexer_peekc(self);

//   assert(isdigit(c));

//   if (LIKELY(isdigit(c))) {

//     // const char nextc = lexer_peekc_next(self);

//     // if (nextc == '.') {
//     //   lexer_adv(self);
//     //   return lexer_float(self, tok, start);
//     // }
//     return lexer_integer(self, tok);
//   }
//   return LexError__UnexpectedCharacterInNumericToken;
// }

PARAMS_NONNULL(1, 2)
static LexError lexer_rune(LexState* self, Token* tok) {
  const i32 start = self->cursor.i;

  char c = lexer_peekc(self);
  if (UNLIKELY(c != ':')) {
    return LexError__UnexpectedRunePrefix;
  }
  do {
    lexer_adv(self);
    c = lexer_peekc(self);
  } while (isalnum(c) || c == '_');

  const i32 end = self->cursor.i;
  tok->type = Token__Rune;
  tok->lexeme = lexer_slice(self, start, end);
  tok->loc = self->cursor;

  return LexError__Ok;
}

PARAMS_NONNULL(1, 3)
static LexError lexer_glyph(LexState* self, TokenType tt, Token* tok) {
  if (LIKELY(tokentype_is_glyph(tt))) {
    tok->type = tt;
    tok->lexeme = tokentype_glyph_sslice(tt);
    tok->loc = self->cursor;
    return LexError__Ok;
  }
  return LexError__TokenTypeOutOfRange;
}



bool tokentype_is_keyword(TokenType self) { return self > Token__KeywordsStart && self < Token__KeywordsEnd; }

bool tokentype_is_glyph(TokenType self) { return self > Token__GlyphStart && self < Token__GlyphsEnd; }
