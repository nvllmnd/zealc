#include "ast/parser.h"

#include <assert.h>
#include <stdarg.h>

#include "ast/expr.h"
#include "ast/lex.h"
#include "ast/token.h"
#include "nv/core/attributes.h"
#include "nv/core/buffer.h"
#include "nv/core/constants.h"
#include "nv/core/log.h"
#include "nv/core_types.h"
#include "nv/memory/arena.h"

const char* parse_error_string(ParseError e) {
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
RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_new(Parser* self) {
  Expr* e = arena_alloc(self->alloc, mlayout_new(Expr));
  return pexpect(e, "Parser Arena failed to allocate new Expr!");
}

METHOD
static Expr* parser_add_expr(Parser* self, Expr e);

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_bool_new(Parser* self, bool val) {
  Expr* e = pexpr_new(self);
  *e = expr_bool(val);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_int_new(Parser* self, i64 val) {
  Expr* e = pexpr_new(self);
  *e = expr_int(val);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_float_new(Parser* self, f64 val) {
  Expr* e = pexpr_new(self);
  *e = expr_float(val);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_rune_new(Parser* self, Rune val) {
  assert(self);
  assert(val.name.begin);

  Expr* e = pexpr_new(self);
  *e = expr_rune(val);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_ident_new(Parser* self, Rune val) {
  assert(self);
  assert(val.name.begin);
  Expr* e = pexpr_new(self);
  *e = expr_ident(val);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_strlit_new(Parser* self, Rune val) {
  assert(self);
  assert(val.name.begin);
  Expr* e = pexpr_new(self);
  *e = expr_strlit(val);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_list_new(Parser* self, Vec(Expr) val) {
  assert(self);
  assert(val);
  Expr* e = pexpr_new(self);
  *e = expr_list(val);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_vlist_new(Parser* self, i32 count, ...) {
  assert(self);
  assert(count > 0);

  va_list args;
  va_start(args);

  Vec(Expr) list = vec_new(Expr, count, arena_allocator(self->alloc));
  assert(list);

  for (i32 i = 0; i < count; i++) {
    Expr e = va_arg(args, Expr);
    vec_push(list, e);
  }

  va_end(args);

  return pexpr_list_new(self, list);
}
RETURNS_NON_NULL
RETURNS_RESOURCE
PARAMS_NONNULL(1, 2, 3)
static inline Expr* pexpr_assign_new(Parser* self, Expr* lhs, Expr* rhs) {
  assert(self);
  assert(lhs);
  assert(rhs);
  Expr* e = pexpr_new(self);
  e->type = Expr__Assignment;
  e->val.assign = make(AssignmentExpr, .lhs = lhs, .rhs = rhs);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
PARAMS_NONNULL(1, 2, 3)
static inline Expr* pexpr_call_new(Parser* self, Expr* callee, Vec(Expr) args) {
  assert(self);
  assert(callee);
  assert(args);
  Expr* e = pexpr_new(self);
  e->type = Expr__Call;
  e->val.call = make(CallExpr, .callee = callee, .args = args);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
PARAMS_NONNULL(1, 2, 3)
static inline Expr* pexpr_operator_new(Parser* self, Expr* lhs, Expr* rhs, OperatorType type) {
  assert(self);
  assert(lhs);
  assert(rhs);
  Expr* e = pexpr_new(self);
  e->type = Expr__Operator;
  e->val.op = make(OperatorExpr, .lhs = lhs, .rhs = rhs, .optype = type);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
PARAMS_NONNULL(1, 2)
static inline Expr* pexpr_print_new(Parser* self, Expr* rhs, PrintType type) {
  assert(self);
  assert(rhs);
  Expr* e = pexpr_new(self);
  e->type = Expr__PrintCall;
  e->val.pc = make(PrintCall, .type = type, .rhs = rhs);
  return e;
}

static ParseError parser_preload(Parser* self);

bool parser_is_eof(const Parser* self) {
  assert(self);
  return self->current.type == Token__Eof;
}

METHOD
static inline void parser_push_error(Parser* self, ParseError perror, LexError lerror) {
  assert(self);

  if (self->errors.len >= MAX_PARSE_ERRORS) {
    self->errors.len = 0;
  }
  self->errors.errs[self->errors.len++] =
      make(ParseErrorInfo, .id = self->errors.total++, .etype = perror, .lerror = lerror, .loc = self->next.loc,
           .expr_string = self->next.lexeme, .prev = self->current, .curr = self->next);
}

RETURNS_ERROR
ParseError advance(Parser* self) {
  assert(self);

  if (parser_is_eof(self)) {
    return ParseErr__EndOfSourceStream;
  }

  self->current = self->next;

  const LexError err = lexer_next(&self->lex, &self->next);
  if UNLIKELY (err != LexError__Ok) {
    static constexpr const ParseError LERROR = ParseErr__InnerLexerError;
    parser_push_error(self, LERROR, err);

    return LERROR;

  } else {
    return ParseErr__Ok;
  }
}

METHOD
static Expr print_stmt(Parser* self, PrintType type);

[[maybe_unused]]
METHOD static ExprStmt* expression_stmt(Parser* self);

[[maybe_unused]]
METHOD static Expr expression(Parser* lex);

[[maybe_unused]]
METHOD static Expr assignment(Parser* lex);

[[maybe_unused]]
METHOD static Expr logical_bitwise(Parser* lex);

[[maybe_unused]]
METHOD static Expr comparison(Parser* lex);

[[maybe_unused]]
METHOD static Expr term(Parser* lex);

[[maybe_unused]]
METHOD static Expr factor(Parser* lex);

[[maybe_unused]]
METHOD static Expr unary(Parser* lex);

[[maybe_unused]]
METHOD static Expr equality(Parser* lex);

[[maybe_unused]]
METHOD static Expr primary(Parser* lex);

[[maybe_unused]]
METHOD static ExprStmt* block_expr_stmt(Parser* self);

Parser parser_new(Arena* alloc) {
  assert(alloc);

  return make(Parser, .alloc = alloc, .lex = {}, .current = {}, .errors = {});
}

METHOD
void parser_print_errors(const Parser* self) {
  assert(self);
  for (i32 i = 0; i < self->errors.len; i++) {
    const ParseErrorInfo* pei = &self->errors.errs[i];
    println("[%d:%d]#: %d =>  LexError: %s, ParseError: %s, Expression: %.*s,", pei->loc.row, pei->loc.col, pei->id,
            lex_error_string(pei->lerror), parse_error_string(pei->etype), RSSPREAD(pei->expr_string));
  }
}

#define adv_ok(_p) (perror_is_ok(advance((_p))))

#define adv_ok_expect(_p, _msg, ...)              \
  do {                                            \
    if UNLIKELY (!adv_ok(_p)) {                   \
      log_fatal(_msg __VA_OPT__(, ) __VA_ARGS__); \
      return make_zeroed(Ast);                    \
    }                                             \
  } while (0)

// METHOD
// static inline void advance_expect(Parser* self) {
//   assert(self);

//   if (!parser_is_eof(self)) {

//   }
//   const ParseError err = advance(self);
//   if (err < ParseErr__Ok && ) {
//   }

// }

PURE_FUNC
METHOD
static inline bool is_match(Parser* self, TokenType tt) {
  assert(self);
  return self->current.type == tt;
}

METHOD
static inline void match_advance(Parser* self, TokenType tt) {
  assert(self);
  if (is_match(self, tt)) {
    const ParseError err = advance(self);
    if (!perror_is_ok(err)) {
      log_fatal("Parser matched token: %s correctly, but encountered an error when trying to advance forward",
                tokentype_string(tt));
    }
    return;
  } else {
    log_fatal("Expected Token: %s, but got %s", tokentype_string(self->current.type), tokentype_string(tt));
  }
}

Ast parser_parse_ast(Parser* self, const char* str, i32 len) {
  assert(str);
  assert(len > 0);
  // clear lex state in case we just finished parsing something else
  self->lex = make_zeroed(LexState);
  // and errors!
  self->errors = make_zeroed(ParseErrorList);

  lexer_init_source(&self->lex, sslice_new(.begin = str, .len = len));

  // prime the parser
  parser_preload(self);

  Vec(Expr) exprs = ({
    const i32 cap = max(self->lex.source.len / 8, 24);
    const Allocator alloc = arena_allocator(self->alloc);
    vec_new(Expr, cap, alloc);
  });

  while (!parser_is_eof(self)) {
    const ParseError perr = advance(self);
    if (perr < ParseErr__Ok) {
      parser_push_error(self, perr, LexError__Ok);
      LOG("Parser encountered Error while advancing through source stream! %s", parse_error_string(perr));
      return make_zeroed(Ast);
    }

    const Token* curr = &self->current;

    Expr e = {};
    switch (curr->type) {
      // case Token__Eof:{} break;
      case Token__Let: {
        TODO();
      } break;
      case Token__Print: {
        e = print_stmt(self, Print__Format);
      } break;
      case Token__Println: {
        e = print_stmt(self, Print__Newline);
      } break;
      // case Token__If:
      // case Token__When:
      // case Token__Fn:
      // case Token__Struct:
      // case Token__Trait:
      // case Token__Impl:
      // case Token__Return:
      case Token__Const: {
        TODO();
      } break;
      // case Token__Loop:
      // case Token__For:
      // case Token__While:
      // case Token__Match:
      // case Token__Pub:
      // case Token__Error:
      // case Token__Enum:
      // case Token__Type:
      // case Token__Await:
      // case Token__Comptime:
      // case Token__Static:
      // case Token__Mod:
      // case Token__Macro:
      // case Token__Derive:
      // case Token__Sizeof:
      default: {
        TODO();
      } break;
        break;
    }

    vec_push(exprs, e);
  }

  return make(Ast, .root = exprs, .alloc = self->alloc);
}

ParseError parser_preload(Parser* self) {
  const char* str = self->lex.source.begin;
  const i32 len = self->lex.source.len;
  self->current = make_zeroed(Token);
  self->next = make_zeroed(Token);

  // We advance twice to preload, this nicely fills our 'current' and 'next' tokens
  // so we can simulate lookahead
  static constexpr const i32 PRELOAD_LEN = 2;

  for (i32 i = 0; i < PRELOAD_LEN; i++) {
    const ParseError err = advance(self);
    if (err < ParseErr__Ok) {
      return err;
    }
    // I dont think a valid expression is ever at all possible with just 2 tokens...
    // MAYBE something like a 'include std' or something, but we dont have to worry about that right now...
    if (parser_is_eof(self)) {
      LOG("Input parse stream:\n\n\"%.*s\"\n\nIs does not contain enough tokens to pre-load Parser!", len, str);
      return ParseErr__SourceStreamTooShort;
    }
  }

  return ParseErr__Ok;
}

static inline void check_expr(Expr e) {
  if UNLIKELY (!expr_is_valid(e)) {
    log_fatal("Parser encountered an invalid expression while trying to parse an expression or statement! Aboring!");
  }
}

Expr print_stmt(Parser* self, PrintType type) {
  const Expr e = expression(self);

  check_expr(e);

  Expr* pe = parser_add_expr(self, e);
  return expr_print_call(pe, type);
}

Expr* parser_add_expr(Parser* self, Expr e) {
  Expr* mem = pexpr_new(self);
  *mem = e;
  return mem;
}
Expr expression(Parser* self) {
  assert(self);
  return assignment(self);
}

Expr assignment(Parser* self) {
  const Expr expr = logical_bitwise(self);

  if (is_match(self, Token__Eq)) {
    const Expr value = assignment(self);

    Expr* lhs = parser_add_expr(self, expr);
    Expr* rhs = parser_add_expr(self, value);

    return expr_assign(lhs, rhs);
  } else {
    return expr;
  }
}
Expr logical_bitwise(Parser* self) {
  Expr expr = comparison(self);
  for (;;) {
    const TokenType tt = self->current.type;
    if (tt == Token__Or) {
      const ParseError err = advance(self);
      if (err < ParseErr__Ok) {
        log_fatal("Parser encounterd parse error: %s while parsing logical bitwise expression! Aborting!",
                  parse_error_string(err));
      }
      const OperatorType ot = Operator__Or;
      Expr right = logical_bitwise(self);
      Expr* lhs = parser_add_expr(self, expr);
      Expr* rhs = parser_add_expr(self, right);
      expr = expr_operator(lhs, rhs, ot);
    } else if (tt == Token__And) {
      const ParseError err = advance(self);
      if (err < ParseErr__Ok) {
        log_fatal("Parser encounterd parse error: %s while parsing logical bitwise expression! Aborting!",
                  parse_error_string(err));
      }
      const OperatorType ot = Operator__And;
      Expr right = logical_bitwise(self);
      Expr* lhs = parser_add_expr(self, expr);
      Expr* rhs = parser_add_expr(self, right);
      expr = expr_operator(lhs, rhs, ot);
    } else {
      break;
    }
  }
  return expr;
}
