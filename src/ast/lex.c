#include "ast/lex.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "ast/token.h"
#include "attributes.h"
#include "log.h"
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

/// Same as [lexer_adv], but peeks the current character after advancing
METHOD
static inline char lexer_adv_peekc(LexState* self) {
  lexer_adv(self);
  return lexer_peekc(self);
}

/// Same as [lexer_adv_nl], but peeks the current character after advancing
METHOD
static inline char lexer_adv_nl_peekc(LexState* self) {
  lexer_adv_nl(self);
  return lexer_peekc(self);
}

METHOD
PURE_FUNC
static inline char lexer_next_peekc(const LexState* self) {
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
  char c = lexer_peekc(self);
  while (!lexer_is_eof(self) && isspace(c)) {
    if (c == '\n') {
      c = lexer_adv_nl_peekc(self);
    } else {
      c = lexer_adv_peekc(self);
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

PARAMS_NONNULL(1, 2)
static LexError lexer_string(LexState* self, Token* tok);

static const char* LEX_ERROR_STRINGS[_LexError__Count] = {
    STRINGIFY(LexError__Ok),
    STRINGIFY(LexError__UnexpectedCharacter),
    STRINGIFY(LexError__UnexpectedCharacterInNumericToken),
    STRINGIFY(LexError__UnmatchedDoubleQuotString),
    STRINGIFY(LexError__UnmatchedSingleQuotString),
    STRINGIFY(LexError__FailedToParseFloat0),
    STRINGIFY(LexError__FailedToParseInt0),
    STRINGIFY(LexError__FloatParseFail),
    STRINGIFY(LexError__IntegerParseFail),
    STRINGIFY(LexError__UnexpectedRunePrefix),
    STRINGIFY(LexError__UnexpectedEscapeCharacter),
    STRINGIFY(LexError__UnexpectedEndOfSource),
    STRINGIFY(LexError__InvalidFloatingPointLiteral),
    STRINGIFY(LexError__IllegalIdentifier),
    STRINGIFY(LexError__FailedToOpenFile),
    STRINGIFY(LexError__InnerFileIO),
    STRINGIFY(LexError__TokenTypeOutOfRange),
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
  const char next = lexer_next_peekc(self);

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

  // LOG_DBG("Lex cursor :: i = %d, col = %d, row = %d", self->cursor.i, self->cursor.col, self->cursor.row);
  lexer_whitespace_skip(self);
  *next_token = (Token){};

  if (UNLIKELY(lexer_is_eof(self))) {
    next_token->type = Token__Eof;
    next_token->lexeme = sslice_static_new("<<EOF>>");
    return LexError__Ok;
  }

  char c = lexer_peekc(self);

  switch (c) {
    case Token__OpenBrace: {
      lexer_adv(self);
      return lexer_glyph(self, Token__OpenBrace, next_token);
    } break;
    case Token__CloseBrace: {
      lexer_adv(self);
      return lexer_glyph(self, Token__CloseBrace, next_token);
    } break;
    case Token__OpenParen: {
      lexer_adv(self);
      return lexer_glyph(self, Token__OpenParen, next_token);
    } break;
    case Token__CloseParen: {
      lexer_adv(self);
      return lexer_glyph(self, Token__CloseParen, next_token);
    } break;
    case Token__OpenBracket: {
      lexer_adv(self);
      return lexer_glyph(self, Token__OpenBracket, next_token);
    } break;
    case Token__CloseBracket: {
      lexer_adv(self);

      return lexer_glyph(self, Token__CloseBracket, next_token);
    } break;
    case Token__Period: {
      lexer_adv(self);
      return lexer_glyph(self, Token__Period, next_token);
    } break;
    case Token__Eq: {

      c = lexer_adv_peekc(self);

      switch (c) {
        case '>': {
          lexer_adv(self);
          return lexer_glyph(self, Token__FatArrow, next_token);
        } break;
        case '=': {
          lexer_adv(self);
          return lexer_glyph(self, Token__DoubleEq, next_token);
        } break;
        default: {
          return lexer_glyph(self, Token__Eq, next_token);
        } break;
      }
    }
    case Token__DoubleQuot: {
      lexer_adv(self);
      return lexer_string(self, next_token);

    } break;
    // TODO: Implement char tokenization, or just make single quote strings the same as double quote ones like
    // javascript...
    // case Token__SingleQuot: { } break;
    case Token__Bang: {
      c = lexer_adv_peekc(self);
      if (c == '=') {
        lexer_adv(self);
        return lexer_glyph(self, Token__BangEq, next_token);
      } else {
        return lexer_glyph(self, Token__Bang, next_token);
      }
    } break;
    case Token__Percent: {
      c = lexer_adv_peekc(self);
      if (c == '=') {
        lexer_adv(self);
        return lexer_glyph(self, Token__PercentEq, next_token);
      } else {
        return lexer_glyph(self, Token__Percent, next_token);
      }

    } break;
    case Token__ChevronUp: {
      c = lexer_adv_peekc(self);
      if (c == '=') {
        lexer_adv(self);
        return lexer_glyph(self, Token__ChevronEq, next_token);
      } else {
        return lexer_glyph(self, Token__ChevronUp, next_token);
      }
    } break;
    case Token__Ampersand: {
      c = lexer_adv_peekc(self);
      if (c == '=') {
        lexer_adv(self);
        return lexer_glyph(self, Token__AmpersandEq, next_token);
      } else {
        return lexer_glyph(self, Token__Ampersand, next_token);
      }
    } break;
    case Token__Star: {
      c = lexer_adv_peekc(self);
      if (c == '=') {
        lexer_adv(self);
        return lexer_glyph(self, Token__StarEq, next_token);
      } else {
        return lexer_glyph(self, Token__Star, next_token);
      }
    } break;
    case Token__Minus: {
      c = lexer_adv_peekc(self);
      switch (c) {
        case '=': {
          lexer_adv(self);
          return lexer_glyph(self, Token__MinusEq, next_token);
        } break;
        case '>': {
          lexer_adv(self);
          return lexer_glyph(self, Token__ArrowRight, next_token);
        } break;
        default: {
          return lexer_glyph(self, Token__Minus, next_token);
        } break;
      }
    } break;
    case Token__Plus: {
      c = lexer_adv_peekc(self);
      if (c == '=') {
        lexer_adv(self);
        return lexer_glyph(self, Token__PlusEq, next_token);
      } else {
        return lexer_glyph(self, Token__Plus, next_token);
      }
    } break;
    case Token__Pipe: {
      c = lexer_adv_peekc(self);
      switch (c) {
        case '|': {
          lexer_adv(self);
          return lexer_glyph(self, Token__DoublePipe, next_token);
        } break;
        case '=': {
          lexer_adv(self);
          return lexer_glyph(self, Token__PipeEq, next_token);
        } break;
        case '>': {
          lexer_adv(self);
          return lexer_glyph(self, Token__PipeRight, next_token);
        } break;
        default: {
          return lexer_glyph(self, Token__Pipe, next_token);
        } break;
      }
    } break;
    case Token__Comma: {
      lexer_adv(self);
      return lexer_glyph(self, Token__Comma, next_token);
    } break;
    case Token__Gt: {
      c = lexer_adv_peekc(self);
      switch (c) {
        case '>': {
          lexer_adv(self);
          return lexer_glyph(self, Token__ShiftRight, next_token);
        } break;
        case '=': {
          lexer_adv(self);
          return lexer_glyph(self, Token__GtEq, next_token);
        } break;
        default: {
          return lexer_glyph(self, Token__Gt, next_token);
        } break;
      }
    } break;
    case Token__Lt: {
      c = lexer_adv_peekc(self);
      switch (c) {
        case '<': {
          lexer_adv(self);
          return lexer_glyph(self, Token__ShiftLeft, next_token);
        } break;
        case '=': {
          lexer_adv(self);
          return lexer_glyph(self, Token__LtEq, next_token);
        } break;
        case '|': {
          lexer_adv(self);
          return lexer_glyph(self, Token__PipeLeft, next_token);
        } break;
        case '-': {
          lexer_adv(self);
          return lexer_glyph(self, Token__ArrowLeft, next_token);
        } break;
        default: {
          return lexer_glyph(self, Token__Lt, next_token);
        } break;
      }
    } break;
    case Token__Semicolon: {
      lexer_adv(self);
      return lexer_glyph(self, Token__Semicolon, next_token);
    } break;
    case Token__Colon: {
      c = lexer_adv_peekc(self);
      if (c == ':') {
        lexer_adv(self);
        return lexer_glyph(self, Token__DoubleColon, next_token);
      } else {
        return lexer_glyph(self, Token__Colon, next_token);
      }
    } break;
    case Token__BackSlash: {
      // TODO: allow backslashes for escaping characters in strings and maybe even
      // for lines like C, for now though, zeal does not recognize '\', and might be
      // completely ignored in the future if not included in core language
      lexer_adv(self);
      return LexError__UnexpectedEscapeCharacter;

    } break;
    case Token__ForwardSlash: {
      c = lexer_adv_peekc(self);
      if (c == '=') {
        lexer_adv(self);
        return lexer_glyph(self, Token__ForwardSlashEq, next_token);
      } else {
        return lexer_glyph(self, Token__ForwardSlash, next_token);
      }
    } break;
    case Token__QMark: {
      lexer_adv(self);
      return lexer_glyph(self, Token__QMark, next_token);
    } break;
  }

  if (lexer_is_rune(self)) {
    return lexer_rune(self, next_token);

  } else if (lexer_is_alpha_or_uscore(self)) {
    return lexer_identifier(self, next_token);

  } else if (isdigit(c)) {
    return lexer_integer(self, next_token);
  }

  return LexError__UnexpectedCharacter;
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
    "\\\\",
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
    "=>",
    "^=",
    "|=",
    "|>",
    "<|",
    ">>",
    "<<",
};

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

static inline const char* tokentype_uglyph_string(TokenType tt) {
  // this should catch any possible errors, but really this branch should never be taken, as if it is
  // that would be a logic error
  if (UNLIKELY(!ispunct(tt))) {
    return nullptr;
  }

  switch (tt) {
    case Token__OpenBrace: return "{";
    case Token__CloseBrace: return "}";
    case Token__OpenParen: return "(";
    case Token__CloseParen: return ")";
    case Token__OpenBracket: return "[";
    case Token__CloseBracket: return "]";
    case Token__DoubleQuot: return "\"";
    case Token__SingleQuot: return "\''";
    case Token__Bang: return "!";
    case Token__Percent: return "%";
    case Token__ChevronUp: return "^";
    case Token__Ampersand: return "&";
    case Token__Star: return "*";
    case Token__Minus: return "-";
    case Token__Plus: return "+";
    case Token__UnaryUnderscore: return "_";
    case Token__Eq: return "=";
    case Token__Pipe: return "|";
    case Token__Comma: return ",";
    case Token__Period: return ".";
    case Token__Gt: return ">";
    case Token__Lt: return "<";
    case Token__Semicolon: return ";";
    case Token__Colon: return ":";
    case Token__BackSlash: return "\\";
    case Token__ForwardSlash: return "/";
    case Token__QMark: return "?";
    default: {
      assert(false);
      return nullptr;
      // UNREACHABLE_RETURN(nullptr);
      // return nullptr;
    } break; 
  }
}

const char* tokentype_glyph_string(TokenType tt) {
  if (ispunct(tt)) {
    return tokentype_uglyph_string(tt);
  }
  if (tokentype_is_glyph(tt)) {
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

    const i32 end = self->cursor.i;
    tok->lexeme = lexer_slice(self, start, end);

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
  tok->loc = make(SourceLocation, start, self->cursor.row, start);
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
  tok->loc = make(SourceLocation, start, self->cursor.row, start);  // self->cursor;
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
  tok->loc = make(SourceLocation, start, self->cursor.row, start);  // self->cursor;

  return LexError__Ok;
}

PARAMS_NONNULL(1, 3)
static LexError lexer_glyph(LexState* self, TokenType tt, Token* tok) {
  // LOG_DBG("LEXER :: Creating glyph for token type: %s", tokentype_string(tt));
  if (LIKELY(tokentype_is_glyph(tt))) {
    tok->type = tt;
    tok->lexeme = tokentype_glyph_sslice(tt);
    const i32 start = self->cursor.i - tok->lexeme.len;
    tok->loc = make(SourceLocation, start, self->cursor.row, start);  // self->cursor;
    return LexError__Ok;
  }
  return LexError__TokenTypeOutOfRange;
}

static LexError lexer_string(LexState* self, Token* tok) {
  static constexpr const i32 DEPTH_MAX = INT32_MAX;

  const i32 start = self->cursor.i - 1;  // previous character will always be the first '"', denoting start of string

  char c = lexer_peekc(self);
  i32 i = 0;
  while (c != '"' && i < DEPTH_MAX) {
    c = lexer_adv_peekc(self);
    i++;
  }

  if (LIKELY(c == '"' && i < DEPTH_MAX)) {
    const i32 end = self->cursor.i;

    lexer_adv(self);
    tok->lexeme = lexer_slice(self, start, end);
    tok->type = Token__String;
    tok->loc = make(SourceLocation, start, self->cursor.row, start);
    return LexError__Ok;
  } else {
    return LexError__UnmatchedDoubleQuotString;
  }

  // for (i32 i = 0; c != '"' && i < DEPTH_MAX; i++, c = lexer_adv_peekc(self));
}

bool tokentype_is_keyword(TokenType self) { return self > Token__KeywordsStart && self < Token__KeywordsEnd; }

bool tokentype_is_glyph(TokenType self) { return (self > Token__GlyphStart && self < Token__GlyphsEnd) || (ispunct(self)); }

const char* tokentype_string(TokenType tt) {
  /// const string for TokenType variants that are used for bounds checking and counting the sizes of each sections
  /// variants, i.e. [Token__LiteralStart], [Token__LiteralEnd], [Token__GlyphsStart] [Token__GLyphsEnd],
  /// [Token__GlyphsCount], ect...
  static constexpr const char TT_CONST[] = "TOKENTYPE_CONST";

  switch (tt) {
    case Token__CannotGetStringOfNonGlyph:
      return STRINGIFY(Token__CannotGetStringOfNonGlyph);
    case Token__CannotGetStringOfNonKeyword:
      return STRINGIFY(Token__CannotGetStringOfNonKeyword);
    case Token__Eof:
      return STRINGIFY(Token__Eof);
    case Token__Identifier:
      return STRINGIFY(Token__Identifier);
    case Token__LiteralStart:
      return TT_CONST;
    case Token__Int:
      return STRINGIFY(Token__Int);
    case Token__Float:
      return STRINGIFY(Token__Float);
    case Token__String:
      return STRINGIFY(Token__String);
    case Token__Rune:
      return STRINGIFY(Token__Rune);
    case Token__LiteralEnd:
      return TT_CONST;
    case Token__OpenBrace:
      return STRINGIFY(Token__OpenBrace);
    case Token__CloseBrace:
      return STRINGIFY(Token__CloseBrace);
    case Token__OpenParen:
      return STRINGIFY(Token__OpenParen);
    case Token__CloseParen:
      return STRINGIFY(Token__CloseParen);
    case Token__OpenBracket:
      return STRINGIFY(Token__OpenBracket);
    case Token__CloseBracket:
      return STRINGIFY(Token__CloseBracket);
    case Token__DoubleQuot:
      return STRINGIFY(Token__DoubleQuot);
    case Token__SingleQuot:
      return STRINGIFY(Token__SingleQuot);
    case Token__Bang:
      return STRINGIFY(Token__Bang);
    case Token__Percent:
      return STRINGIFY(Token__Percent);
    case Token__ChevronUp:
      return STRINGIFY(Token__ChevronUp);
    case Token__Ampersand:
      return STRINGIFY(Token__Ampersand);
    case Token__Star:
      return STRINGIFY(Token__Star);
    case Token__Minus:
      return STRINGIFY(Token__Minus);
    case Token__Plus:
      return STRINGIFY(Token__Plus);
    case Token__UnaryUnderscore:
      return STRINGIFY(Token__UnaryUnderscore);
    case Token__Eq:
      return STRINGIFY(Token__Eq);
    case Token__Pipe:
      return STRINGIFY(Token__Pipe);
    case Token__Comma:
      return STRINGIFY(Token__Comma);
    case Token__Period:
      return STRINGIFY(Token__Period);
    case Token__Gt:
      return STRINGIFY(Token__Gt);
    case Token__Lt:
      return STRINGIFY(Token__Lt);
    case Token__Semicolon:
      return STRINGIFY(Token__Semicolon);
    case Token__Colon:
      return STRINGIFY(Token__Colon);
    case Token__BackSlash:
      return STRINGIFY(Token__BackSlash);
    case Token__ForwardSlash:
      return STRINGIFY(Token__ForwardSlash);
    case Token__QMark:
      return STRINGIFY(Token__QMark);
    case Token__GlyphStart:
      return STRINGIFY(Token__GlyphStart);
    case Token__StarEq:
      return STRINGIFY(Token__StarEq);
    case Token__AmpersandEq:
      return STRINGIFY(Token__AmpersandEq);
    case Token__PercentEq:
      return STRINGIFY(Token__PercentEq);
    case Token__BangEq:
      return STRINGIFY(Token__BangEq);
    case Token__DoubleEq:
      return STRINGIFY(Token__DoubleEq);
    case Token__PlusEq:
      return STRINGIFY(Token__PlusEq);
    case Token__MinusEq:
      return STRINGIFY(Token__MinusEq);
    case Token__DblQMark:
      return STRINGIFY(Token__DblQMark);
    case Token__DblForwardSlash:
      return STRINGIFY(Token__DblForwardSlash);
    case Token__ForwardSlashEq:
      return STRINGIFY(Token__ForwardSlashEq);
    case Token__DoubleBackSlash:
      return STRINGIFY(Token__DoubleBackSlash);
    case Token__DoubleColon:
      return STRINGIFY(Token__DoubleColon);
    case Token__LtEq:
      return STRINGIFY(Token__LtEq);
    case Token__GtEq:
      return STRINGIFY(Token__GtEq);
    case Token__Elipses:
      return STRINGIFY(Token__Elipses);
    case Token__DoublePipe:
      return STRINGIFY(Token__DoublePipe);
    case Token__EmptyParen:
      return STRINGIFY(Token__EmptyParen);
    case Token__EmptyBracket:
      return STRINGIFY(Token__EmptyBracket);
    case Token__EmptyBrace:
      return STRINGIFY(Token__EmptyBrace);
    case Token__ArrowRight:
      return STRINGIFY(Token__ArrowRight);
    case Token__ArrowLeft:
      return STRINGIFY(Token__ArrowLeft);
    case Token__FatArrow:
      return STRINGIFY(Token__FatArrow);
    case Token__ChevronEq:
      return STRINGIFY(Token__ChevronEq);
    case Token__PipeEq:
      return STRINGIFY(Token__PipeEq);
    case Token__PipeRight:
      return STRINGIFY(Token__PipeRight);
    case Token__PipeLeft:
      return STRINGIFY(Token__PipeLeft);
    case Token__ShiftRight:
      return STRINGIFY(Token__ShiftRight);
    case Token__ShiftLeft:
      return STRINGIFY(Token__ShiftLeft);
    case Token__GlyphsEnd:
      return TT_CONST;
    case Token__GlyphsCount:
      return TT_CONST;
    case Token__Comment:
      return STRINGIFY(Token__Comment);
    case Token__KeywordsStart:
      return TT_CONST;
    case Token__True:
      return STRINGIFY(Token__True);
    case Token__False:
      return STRINGIFY(Token__False);
    case Token__Let:
      return STRINGIFY(Token__Let);
    case Token__If:
      return STRINGIFY(Token__If);
    case Token__Else:
      return STRINGIFY(Token__Else);
    case Token__Mut:
      return STRINGIFY(Token__Mut);
    case Token__When:
      return STRINGIFY(Token__When);
    case Token__Fn:
      return STRINGIFY(Token__Fn);
    case Token__Struct:
      return STRINGIFY(Token__Struct);
    case Token__Trait:
      return STRINGIFY(Token__Trait);
    case Token__Impl:
      return STRINGIFY(Token__Impl);
    case Token__And:
      return STRINGIFY(Token__And);
    case Token__Or:
      return STRINGIFY(Token__Or);
    case Token__Return:
      return STRINGIFY(Token__Return);
    case Token__Self:
      return STRINGIFY(Token__Self);
    case Token__Const:
      return STRINGIFY(Token__Const);
    case Token__Loop:
      return STRINGIFY(Token__Loop);
    case Token__For:
      return STRINGIFY(Token__For);
    case Token__While:
      return STRINGIFY(Token__While);
    case Token__Break:
      return STRINGIFY(Token__Break);
    case Token__Match:
      return STRINGIFY(Token__Match);
    case Token__Continue:
      return STRINGIFY(Token__Continue);
    case Token__Pub:
      return STRINGIFY(Token__Pub);
    case Token__Ref:
      return STRINGIFY(Token__Ref);
    case Token__Error:
      return STRINGIFY(Token__Error);
    case Token__Enum:
      return STRINGIFY(Token__Enum);
    case Token__Type:
      return STRINGIFY(Token__Type);
    case Token__Await:
      return STRINGIFY(Token__Await);
    case Token__Comptime:
      return STRINGIFY(Token__Comptime);
    case Token__Static:
      return STRINGIFY(Token__Static);
    case Token__Mod:
      return STRINGIFY(Token__Mod);
    case Token__Macro:
      return STRINGIFY(Token__Macro);
    case Token__Derive:
      return STRINGIFY(Token__Derive);
    case Token__Dyn:
      return STRINGIFY(Token__Dyn);
    case Token__Default:
      return STRINGIFY(Token__Default);
    case Token__Sizeof:
      return STRINGIFY(Token__Sizeof);
    case Token__KeywordsEnd:
      [[fallthrough]];
    case Token__KeywordCount:
      return TT_CONST;
      break;
  }
}
