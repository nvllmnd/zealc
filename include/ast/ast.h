#pragma once



#include "nv/core_types.h"

struct Ast {
  /// Vec(Expr), so length is accessible through vec_len
  /// not writing it as Vec(Expr) to avoid having to include buffer.h in this header
  /// TODO: Change this from [Expr] to [ExprStmt] after we have verified we can parse expressions
  /// and simple print/println/let/const/var/ statements
  struct Expr* root;
  /// Arena that owns memory where our AST nodes live
  struct Arena* alloc;
};
alias(Ast);
