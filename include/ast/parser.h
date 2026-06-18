#pragma once

#include "ast/ast.h"
#include "ast/lex.h"
#include "ast/token.h"
#include "error.h"
#include "nv/core/constants.h"
#include "nv/core/intdefs.h"

typedef enum ParseError : uerror {

  ParseErr__Ok = ZOK,

  PARSE_OK = ParseErr__Ok,

  PARSE_ERROR = 1,
  ParseErr__InvalidToken = 1 << 1,
  ParseErr__InnerLexerError = 1 << 2,
  ParseErr__UnmatchedCurlyBrace = 1 << 3,
  ParseErr__ExpectedBlockExpr = 1 << 4,
  ParseErr__UnexpectedDefineName = 1 << 5,
  ParseErr__InvalidAssignment = 1 << 6,
  ParseErr__UnexpectedEof = 1 << 7,
  ParseErr__SourceStreamTooShort = 1 << 8,
  ParseErr__ExpectedIdentifier = 1 << 9,
  ParseErr__EndOfSourceStream = 1 << 10,
  ParseErr__Unknown = 1 << 11,

  PARSE_ERROR_COUNT = 12,

} HEDLEY_FLAGS ParseError;

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

struct Vallocator;

/// Zeal language parser.
/// Parses a source string of Zeal source code into an Abstract Syntax Tree.
///
/// # Memory
///
///
/// - Parser does not own any memory, allocates AST nodes with given [Arena] allocator.
/// - Source string being parsed is expected to be managed by caller and be valid for
///     as long as it is being parsed.
/// - Any strings pointed to by AST nodes are expected to be valid for as long as the lifetime of the
///     [Arena] used to create this parser
struct Parser {
  /// Arena allocator used to allocate AST nodes.
  /// The AST returned by a successfully parsed source string must live for as long
  /// as the lifetime of this Arena allocator
  struct Vallocator* alloc;
  /// LexState used to parse Zeal source string. You can read the
  /// full source string through this [LexState]'s source field
  LexState lex;
  /// 'Current' token being parsed. Because of how we lex source code,
  /// we cant acutally peek next tokens, so technically this is actually the 'previous' token,
  /// but since we pre-load the parser, this usage should be opaque
  Token current;
  /// Next token being parsed. Because of how we lex source code, we cant actually peek next tokens,
  /// so technically this is actually the 'current' token, but these semantics should be opaque since we pre-load the
  /// parser
  Token next;
  /// List of ParseErrors, currently this is a small ringbuffer
  // ParseErrorList errors;
};
alias(Parser);

/// Parses Zeal source code in a UTF-8 String of @param (i32 len).
/// Returns an [Ast] struct, which contains a [Vec] of [Expr] pointers, where each element is a parsed expression.
/// [Ast] also contains a pointer to the Arena that owns the AST nodes
/// WARN: Keep in mind i plan to have this Arena be 'exclusively owned', so its lifetime is going
/// to be managed by some top-level state or struct, dont worry about destroying for now
METHOD
Ast parser_parse_ast(Parser* self, const char* str, i32 len);

METHOD
Expr* parser_parse_expr(Parser* self, const char* str, i32 len);

CONST_FUNC
const char* parse_error_string(ParseError e);

METHOD
PURE_FUNC
bool parser_is_eof(const Parser* self);

CONST_FUNC
static inline bool perror_is_ok(ParseError e) { return e >= ParseErr__Ok; }

PURE_FUNC
Parser parser_new(struct Vallocator* alloc);

METHOD
void parser_print_errors(const Parser* self);

/// @brief resets parser back to default state. keeping the allocator pointer it was created with
void parser_reset(Parser* self);

METHOD
sslice expression_string(const Expr* expr, Allocator alloc);
METHOD
sslice statement_string(const ExprStmt* expr, Allocator alloc);

METHOD
void print_expression(const Expr* e);

METHOD
void print_statement(const ExprStmt* e);

METHOD
ExprStmt parser_parse_expr_stmt(Parser* self, const char* str, i32 len);
