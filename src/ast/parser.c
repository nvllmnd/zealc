#include "ast/parser.h"

#include <assert.h>

#include "ast/expr.h"
#include "ast/lex.h"
#include "ast/token.h"
#include "nv/core/log.h"
#include "nv/core_types.h"
#include "nv/memory/arena.h"
#include "nv/memory/virt.h"

typedef enum ParseError : error {
  PARSE_ERROR_MIN_LIMIT = -0x100,

  ParseErr__InvalidToken = PARSE_ERROR_MIN_LIMIT,
  ParseErr__InnerLexerError,
  ParseErr__UnmatchedCurlyBrace,
  ParseErr__ExpectedBlockExpr,
  ParseErr__UnexpectedDefineName,
  ParseErr__InvalidAssignment,
  ParseErr__UnexpectedEof,
  ParseErr__Unknown,
  PARSE_ERROR_MAX_LIMIT,
  ParseErr__Ok = ZOK,
  ParseErr__EndOfSourceStream,

} ParseError;

CONST_FUNC
static inline const char* parse_error_string(ParseError e) {
  switch (e) {
    case ParseErr__InnerLexerError: {
      return STRINGIFY(ParseErr__InnerLexerError) " :: Parser encountered a Lexer Error";
    } break;
    case ParseErr__UnmatchedCurlyBrace: {
      return STRINGIFY(ParseErr__UnmatchedCurlyBrace) " :: Unmatched '{'!";
    } break;
    case ParseErr__ExpectedBlockExpr: {
      return STRINGIFY(ParseErr__ExpectedBlockExpr) " :: Expected Block Expression but encountered something else!";
    } break;
    case ParseErr__UnexpectedDefineName: {
      return STRINGIFY(ParseErr__UnexpectedDefineName) " :: Unexpected Define Name!";
    } break;
    case ParseErr__InvalidAssignment: {
      return STRINGIFY(ParseErr__InvalidAssignment) " :: Invalid/Illegal Assignment!";
    } break;
    case ParseErr__UnexpectedEof: {
      return STRINGIFY(
          ParseErr__UnexpectedEof) " :: Parser encountered an unexpected End of File/Stream in middle of parsing!";
    } break;
    case ParseErr__Unknown: {
      return STRINGIFY(ParseErr__Unknown) " :: Parser encountered an unknown error!";
    } break;
    case ParseErr__EndOfSourceStream: {
      return STRINGIFY(ParseErr__EndOfSourceStream) " :: Parser has reach the end of a source stream without any (unrecoverable) errors!";
    } break;
    case ParseErr__Ok: {
      return STRINGIFY(ParseErr__Ok) " :: Parser has completed its execution without any (unrecoverable) errors!";
    } break;
    default: {
      return "Unknown ParseError enum variant!";
    } break;
  }
}

struct ParseErrorInfo {
  i32 id;
  ParseError etype;
  LexError lerror;
  SourceLocation loc;
  /// expression(statement) as a string, that is causing the parse error
  sslice expr_string;
  Token prev;
  Token curr;
};
alias(ParseErrorInfo);

static constexpr const i32 MAX_PARSE_ERRORS = 8;

struct ParseErrorList {
  u8 len;
  i32 total;
  ParseErrorInfo errs[MAX_PARSE_ERRORS];
};
alias(ParseErrorList);

/// allocate 1GB of Virtual Memory for our AST nodes
static constexpr const i32 AST_ARENA_CAP_MB = 1024;
/// use a ~1 page of virtual memory as initial size
static constexpr const i32 AST_ARENA_INITIAL_SIZE = KILOBYTES(4);

struct Parser {
  Arena* alloc;
  LexState lex;
  LexIter prev;
  LexIter current;
  ParseErrorList errors;
};
alias(Parser);

METHOD
PURE_FUNC
static inline bool parse_is_eof(const Parser* self) {
  assert(self);
  return self->current.tok.type == Token__Eof || self->prev.tok.type == Token__Eof;
}

CONST_FUNC
static inline bool error_is_ok(ParseError e) { return e >= ParseErr__Ok; }

METHOD
static inline ParseError advance(Parser* self) {
  assert(self);

  if (parse_is_eof(self)) {
    return ParseErr__EndOfSourceStream;
  }

  self->prev = self->current;

  const LexError err = lexer_next(&self->lex, &self->current.tok);
  if UNLIKELY (err != LexError__Ok) {
    self->current.err = err;

    static constexpr const ParseError LERROR = ParseErr__InnerLexerError;
    if (self->errors.len >= MAX_PARSE_ERRORS) {
      self->errors.len = 0;
    }
    self->errors.errs[self->errors.len++] =
        make(ParseErrorInfo, .id = self->errors.total++, .etype = LERROR, .lerror = err, .loc = self->current.tok.loc,
             .expr_string = self->current.tok.lexeme, .prev = self->prev.tok, .curr = self->current.tok);

    return LERROR;

  } else {
    self->current.err = LexError__Ok;
    return ParseErr__Ok;
  }
}

[[maybe_unused]]
static Expr* expression(Parser* lex);

[[maybe_unused]]
static Expr* assignment(Parser* lex);

[[maybe_unused]]
static Expr* logical_bitwise(Parser* lex);

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

static inline Parser parser_new(const char* str, i32 len) {
  LexState lex = {};

  const sslice src = sslice_new(str, len);

  lexer_init_source(&lex, src);

  Arena* arena = arena_new(AST_ARENA_CAP_MB, AST_ARENA_INITIAL_SIZE);

  return make(Parser, .alloc = pexpect(arena, "Failed to create Arena Allocator for Parser!"), .lex = lex,
              .current = {}, .prev = {}, .errors = {});
}

METHOD
static inline void parser_print_errors(const Parser* self) {
  assert(self);
  for (i32 i = 0; i < self->errors.len; i++) {
    const ParseErrorInfo* pei = &self->errors.errs[i];
    println("[%d:%d]#: %d =>  LexError: %s, ParseError: %s, Expression: %.*s,", pei->loc.row, pei->loc.col, pei->id,
            lex_error_string(pei->lerror), parse_error_string(pei->etype), RSSPREAD(pei->expr_string));
  }
}

METHOD
static inline void parser_destroy(Parser* self) {
  assert(self);
  if (self->alloc) {
    arena_destroy(self->alloc);
    self->alloc = nullptr;
    self->current = make_zeroed(LexIter);
    self->prev = make_zeroed(LexIter);
    self->lex = make_zeroed(LexState);
    if (self->errors.len > 0) {
      println("Destroying Parser with %d active errors:", (i32)self->errors.len);
      parser_print_errors(self);
    }
    self->errors = make_zeroed(ParseErrorList);
  }
}

#define adv_ok(_p) (error_is_ok(advance((_p))))

#define adv_ok_expect(_p, _msg, ...)              \
  do {                                            \
    if UNLIKELY (!adv_ok(_p)) {                   \
      parser_destroy(_p);                         \
      log_fatal(_msg __VA_OPT__(, ) __VA_ARGS__); \
      return nullptr;                             \
    }                                             \
  } while (0)

Expr* parse_source_str(const char* str, isize len) {
  assert(str);
  assert(len > 0);

  Parser p = parser_new(str, len);

  // prime the parser
  adv_ok_expect(&p, "Input parse stream:\n\n\"%.*s\"\n\nIs does not contain enough tokens to pre-load Parser!",
                (i32)len, str);

  return nullptr;
}
