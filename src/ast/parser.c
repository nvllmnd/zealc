#include "ast/parser.h"

#include <assert.h>
#include <stdarg.h>

#include "ast/expr.h"
#include "ast/lex.h"
#include "ast/token.h"
#include "nv/core/attributes.h"
#include "nv/core/constants.h"
#include "nv/core/log.h"
#include "nv/core_types.h"
#include "nv/iter.h"
#include "nv/iter/vec.h"
#include "nv/memory/arena.h"
#include "runes.h"

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

PURE_FUNC
METHOD
static TokenType peektype(Parser* self) {
  assert(self);
  return self->current.type;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
METHOD
static inline Expr* pexpr_new(Parser* self) {
  Expr* e = arena_alloc(self->alloc, mlayout_new(Expr));
  return pexpect(e, "Parser Arena failed to allocate new Expr!");
}

RETURNS_ERROR
METHOD
static ParseError try_advance(Parser* self);

/// Returns false upon reaching Eof, otherwise true while parsing
METHOD
static bool advance(Parser* self);

METHOD
[[maybe_unused]]
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
[[maybe_unused]]
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
[[maybe_unused]]
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
static inline Expr* pexpr_binop_new(Parser* self, Expr* lhs, Expr* rhs, OperatorType type) {
  assert(self);
  assert(lhs);
  assert(rhs);
  Expr* e = pexpr_new(self);
  e->type = Expr__BinOp;
  e->val.binop = make(BinOpExpr, .lhs = lhs, .rhs = rhs, .optype = type);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
PARAMS_NONNULL(1, 2)
static inline Expr* pexpr_unaryop_new(Parser* self, Expr* rhs, OperatorType ot) {
  assert(self);
  assert(rhs);
  assert(ot == Operator__Not || ot == Operator__Negate);

  Expr* e = pexpr_new(self);
  e->type = Expr__UnaryOp;
  e->val.uop = make(UnaryOpExpr, .rhs = rhs, .optype = ot);
  return e;
}

RETURNS_NON_NULL
RETURNS_RESOURCE
[[maybe_unused]]
PARAMS_NONNULL(1) static inline Expr* pexpr_binop_add(Parser* self, Expr lhs, Expr rhs, OperatorType type) {
  assert(self);

  Expr* l = pexpr_new(self);
  assert(l);
  *l = lhs;

  Expr* r = pexpr_new(self);
  assert(r);
  *r = rhs;

  Expr* e = pexpr_new(self);
  e->type = Expr__BinOp;
  e->val.binop = make(BinOpExpr, .lhs = l, .rhs = r, .optype = type);
  return e;
}
RETURNS_NON_NULL
RETURNS_RESOURCE
PARAMS_NONNULL(1, 2)
[[maybe_unused]]
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
  return self->next.type == Token__Eof && self->current.type == Token__Eof;
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

ParseError try_advance(Parser* self) {
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

bool advance(Parser* self) {
  assert(self);

  if UNLIKELY (parser_is_eof(self)) {
    LOG_DBG("Parser reached EOF!");
    return false;
  }

  self->current = self->next;

  const LexError lerr = lexer_next(&self->lex, &self->next);
  if UNLIKELY (lerr != LexError__Ok) {
    log_fatal("!!LEX ERROR!! %s", lex_error_string(lerr));
  }
  return true;
}

#define expect_adv(_p)                                    \
  do {                                                    \
    if (!advance(_p)) {                                   \
      log_fatal("Unexpected End of File/Source Stream!"); \
    }                                                     \
  } while (0);

METHOD
static Expr print_stmt(Parser* self, PrintType type);

METHOD
RETURNS_NON_NULL
[[maybe_unused]]
static ExprStmt* expression_stmt(Parser* self);

METHOD
RETURNS_NON_NULL
static Expr* expression(Parser* lex);

METHOD
/// TODO: This should return a ExprStmt once we  have those ready to go
static Expr statement(Parser* self);

METHOD
RETURNS_NON_NULL
static Expr* assignment(Parser* lex);

METHOD
RETURNS_NON_NULL
static Expr* logical_bitwise(Parser* lex);

METHOD
RETURNS_NON_NULL
static Expr* comparison(Parser* lex);

RETURNS_NON_NULL
METHOD
static Expr* term(Parser* lex);

METHOD
RETURNS_NON_NULL
static Expr* factor(Parser* lex);

RETURNS_NON_NULL
METHOD
static Expr* unary(Parser* lex);

METHOD
RETURNS_NON_NULL
static Expr* primary(Parser* lex);

[[maybe_unused]]
METHOD RETURNS_NON_NULL static ExprStmt* block_expr_stmt(Parser* self);

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
[[maybe_unused]]
static inline bool match_any_(Parser* self, i32 count, ...) {
  assert(count >= 1);
  va_list args;
  va_start(args);

  const TokenType tt = peektype(self);

  for (i32 i = 0; i < count; i++) {
    const TokenType next = va_arg(args, TokenType);
    if (next == tt) {
      va_end(args);
      return true;
    }
  }

  va_end(args);
  return false;
}
#define match_any(_p, ...) (match_any_(_p, VA_ARGS_LEN(__VA_ARGS__), __VA_ARGS__))

// METHOD
// static inline void match_advance(Parser* self, TokenType tt) {
//   assert(self);
//   if (is_match(self, tt)) {
//     const ParseError err = advance(self);
//     if (!perror_is_ok(err)) {
//       log_fatal("Parser matched token: %s correctly, but encountered an error when trying to advance forward",
//                 tokentype_string(tt));
//     }
//     return;
//   } else {
//     log_fatal("Expected Token: %s, but got %s", tokentype_string(self->current.type), tokentype_string(tt));
//   }
// }

#define match(_p, ...)                                                                                          \
  do {                                                                                                          \
    if (!match_any(_p, __VA_ARGS__)) {                                                                          \
      log_fatal("Parser expected current token %s to match any of tokens: %s!", tokentype_string(peektype(_p)), \
                #__VA_ARGS__);                                                                                  \
    }                                                                                                           \
  } while (0);

#define expect_terminal(_p)                         \
  do {                                              \
    if (!match_any(_p, Token__Semicolon)) {         \
      log_fatal("Expected ';' to end statements!"); \
    }                                               \
  } while (0);

#define match_binop(_p, ...)        \
  ({                                \
    match(_p, __VA_ARGS__);         \
    tokentype_optype(peektype(_p)); \
  })

#define match_advance(_p, ...)                                                                                  \
  do {                                                                                                          \
    if (match_any(_p, __VA_ARGS__)) {                                                                           \
      const ParseError _err = advance(_p);                                                                      \
      if (_err < ParseErr__Ok) {                                                                                \
        log_fatal(                                                                                              \
            "Parser matched token %s correctly, but encountered an error: %s after "                            \
            "advancing past match!",                                                                            \
            tokentype_string(peektype(_p)), parse_error_string(_err));                                          \
      }                                                                                                         \
    } else {                                                                                                    \
      log_fatal("Parser expected current token %s to match any of tokens: %s!", tokentype_string(peektype(_p)), \
                #__VA_ARGS__);                                                                                  \
    }                                                                                                           \
  } while (0)

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

    Expr expr = statement(self);

    if (vec_is_full(exprs)) {
      exprs = vec_resize(exprs, vec_len(exprs) * 2, arena_allocator(self->alloc));
    }

    vec_push(exprs, expr);
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
    const ParseError err = try_advance(self);
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

METHOD
static Expr block_stmt(Parser* self);

Expr print_stmt(Parser* self, PrintType type) {
  Expr* e = expression(self);
  expect_terminal(self);
  advance(self);

  return expr_print_call(e, type);
}

Expr* parser_add_expr(Parser* self, Expr e) {
  Expr* mem = pexpr_new(self);
  *mem = e;
  return mem;
}
Expr* expression(Parser* self) {
  assert(self);
  return assignment(self);
}

Expr* assignment(Parser* self) {
  Expr* expr = logical_bitwise(self);

  if (is_match(self, Token__Eq)) {
    expect_adv(self);

    Expr* value = assignment(self);

    return pexpr_assign_new(self, expr, value);
  } else {
    return expr;
  }
}

Expr* logical_bitwise(Parser* self) {
  Expr* expr = comparison(self);
  bool done = false;
  while (!done) {
    OperatorType op = Operator__Invalid;
    switch (peektype(self)) {
      case Token__Or: {
        op = Operator__Or;
      } break;
      case Token__And: {
        op = Operator__And;
      } break;
      default: {
        done = true;
        continue;
      } break;
    }

    expect_adv(self);

    Expr* rhs = logical_bitwise(self);
    expr = pexpr_binop_new(self, expr, rhs, op);
  }
  return expr;
}

Expr* comparison(Parser* self) {
  assert(self);
  Expr* expr = term(self);
  bool done = false;
  while (!done) {
    OperatorType op = {};

    switch (peektype(self)) {
      case Token__Gt: {
        op = Operator__Gt;
      } break;
      case Token__GtEq: {
        op = Operator__Gte;
      } break;
      case Token__Lt: {
        op = Operator__Lt;
      } break;
      case Token__LtEq: {
        op = Operator__Lte;
      } break;
      case Token__DoubleEq: {
        op = Operator__Eq;
      } break;
      case Token__BangEq: {
        op = Operator__NotEq;
      } break;
      default: {
        done = true;
        continue;
      } break;
    }
    expect_adv(self);

    Expr* rhs = term(self);
    expr = pexpr_binop_new(self, expr, rhs, op);
  }
  return expr;
}

Expr* term(Parser* self) {
  Expr* expr = factor(self);
  for (;;) {
    const TokenType tt = peektype(self);
    OperatorType op = Operator__Invalid;
    if (tt == Token__Minus) {
      op = Operator__Minus;
    } else if (tt == Token__Plus) {
      op = Operator__Plus;
    } else {
      break;
    }
    expect_adv(self);
    Expr* rhs = factor(self);

    expr = pexpr_binop_new(self, expr, rhs, op);
  }
  return expr;
}

Expr* factor(Parser* self) {
  Expr* expr = unary(self);
  for (;;) {
    const TokenType tt = peektype(self);
    OperatorType op = Operator__Invalid;
    if (tt == Token__ForwardSlash) {
      op = Operator__Minus;
    } else if (tt == Token__Star) {
      op = Operator__Plus;
    } else {
      break;
    }

    expect_adv(self);

    Expr* rhs = unary(self);

    expr = pexpr_binop_new(self, expr, rhs, op);
  }
  return expr;
}

Expr* unary(Parser* self) {
  assert(self);
  const TokenType tt = peektype(self);
  OperatorType op = Operator__Invalid;

  if (tt == Token__Bang) {
    op = Operator__Not;

  } else if (tt == Token__Minus) {
    op = Operator__Negate;
  } else {
    return primary(self);
  }

  expect_adv(self);
  Expr* rhs = unary(self);

  return pexpr_unaryop_new(self, rhs, op);
}

Expr* primary(Parser* self) {
  const TokenType tt = peektype(self);
  switch (tt) {
    case Token__True: {
      expect_adv(self);
      return pexpr_bool_new(self, true);
    } break;

    case Token__False: {
      expect_adv(self);
      return pexpr_bool_new(self, false);

    } break;

    case Token__Int: {
      expect_adv(self);
      return pexpr_int_new(self, self->current.num_literal.integer);
    } break;

    case Token__Float: {
      expect_adv(self);
      return pexpr_float_new(self, self->current.num_literal.fp);
    } break;

    case Token__Rune: {
      expect_adv(self);
      const Rune r = runetab_add(self->current.lexeme);
      return pexpr_rune_new(self, r);
    } break;

    case Token__String: {
      expect_adv(self);
      const Rune r = runetab_add(self->current.lexeme);
      return pexpr_strlit_new(self, r);
    } break;

    case Token__OpenParen: {
      expect_adv(self);
      Expr* expr = expression(self);
      if (peektype(self) == Token__CloseParen) {
        expect_adv(self);
        return expr;
      } else {
        log_fatal("Unexpected Token: %s", tokentype_string(peektype(self)));
      }

    } break;

    case Token__Identifier: {
      expect_adv(self);
      const Rune r = runetab_add(self->current.lexeme);
      return pexpr_ident_new(self, r);

    } break;
    default: {
      log_fatal("Unexpected token: %s", tokentype_string(tt));
    } break;
  }
}

Expr statement(Parser* self) {
  // const Token* curr = &self->current;
  LOG("Parsing: %.*s(%s)", RSSPREAD(self->current.lexeme), tokentype_string(peektype(self)));

  Expr e = {};
  switch (peektype(self)) {
    // case Token__Eof:{} break;
    case Token__Let: {
      TODO();
    } break;
    case Token__Print: {
        LOG("MAtched print");
        expect_adv(self);
      e = print_stmt(self, Print__Format);
    } break;
    case Token__Println: {

        LOG("MAtched println");
        expect_adv(self);
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
      e = block_stmt(self);
    } break;
      break;
  }

  // expect_terminal(self);
  // advance(self);
  return e;
}

static Expr block_stmt(Parser* self) {
  assert(self);

  const bool is_block = peektype(self) == Token__OpenBrace;

  if (is_block) {
    expect_adv(self);
  }

  Vec(Expr) stmts = vec_new(Expr, 24, arena_allocator(self->alloc));
  while (!parser_is_eof(self)) {
    if UNLIKELY (is_block && peektype(self) == Token__CloseBrace) {
     expect_adv(self);
     break; 
    }

    if UNLIKELY (vec_is_full(stmts)) {
      stmts = vec_resize(stmts, vec_len(stmts) * 2, arena_allocator(self->alloc));
      assert(stmts);
    }

    Expr e = statement(self);
    vec_push(stmts, e);

    // expect_adv(self);
  }

  if (peektype(self) == Token__CloseBrace) {
    expect_adv(self);
  }

  // TODO: Make this an ExprStmt, or and Expr List depending on if this is a block statement or not
  return expr_list(stmts);

}
