#pragma once

#include "ast/expr.h"
#include "nv/core.h"
#include "nv/core/attributes.h"

typedef enum AstWalkError : error {

  AST_WALK_ERROR_MAX_VALUE,
  AstError__EncounteredInvalidExpr = -2,
  AstError__InnerSystemError = -1,
  AstError__Ok,
  /// anything >= 1 is a user generated value or error
  AstError__UserData,
} AstWalkError;

typedef AstWalkError (*AstWalkExprFn)(Expr* expr, void* userdata);
typedef AstWalkError (*AstWalkExprStmtFn)(ExprStmt* expr_stmt, void* userdata);

/// Zeal Abstract Syntax Tree. There is no init/new functions for this type (as of now 05/23/2026)
/// instead, this type is usually created by the parser in parser.c/.h
struct Ast {
  /// Vec(Expr), so length is accessible through vec_len
  /// not writing it as Vec(Expr) to avoid having to include buffer.h in this header
  /// TODO: Change this from [Expr] to [ExprStmt] after we have verified we can parse expressions
  /// and simple print/println/let/const/var/ statements
  struct Expr* root;
  /// Arena that owns memory where our AST nodes live.
  /// we track this in this struct in case we need to merge with other ASTs(when i have that built out) or allocate
  /// strings and other such mess
  struct Arena* alloc;

  struct AstWalker {
    void* userdata;
    AstWalkExprFn walk_expr;
    AstWalkExprStmtFn walk_expr_stmt;
  } walker;
};
alias(Ast);
alias(AstWalker);

PARAMS_NONNULL(1, 3, 4)
static inline void ast_walker_init(Ast* self, void* userdata, AstWalkExprFn walk_expr,
                                   AstWalkExprStmtFn walk_expr_stmt) {
  assert(self);
  assert(walk_expr);
  assert(walk_expr_stmt);
  self->walker = make(AstWalker, .userdata = userdata, .walk_expr = walk_expr, .walk_expr_stmt = walk_expr_stmt);
}

METHOD
void ast_walk(Ast* self);

PURE_FUNC
METHOD
sslice ast_stringify(Ast* self);
