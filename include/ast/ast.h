#pragma once



#include "ast/expr.h"
#include "core_types.h"


struct Ast {
  Expr* start;
  i32 len;
  i32 cap;
};
alias(Ast);


struct AstEntry {
  ExprSlot id; 
  Expr* e;
};
alias(AstEntry);

Ast ast_new(isize cap);


METHOD
AstEntry ast_push(Ast* self, Expr expr);


METHOD
AstEntry ast_exprlist_push(Ast* self, Expr* exprs, i32 len);


METHOD
AstEntry ast_bool_push(Ast* self, bool val);

METHOD
AstEntry ast_int_push(Ast* self, i64 n);

METHOD
AstEntry ast_float_push(Ast* self, f64 n);


METHOD
AstEntry ast_string_push(Ast* self, sslice string);

METHOD
AstEntry ast_ident_push(Ast* self, sslice ident);

METHOD
AstEntry ast_assign_push(Ast* self, ExprSlot lhs, ExprSlot rhs);


METHOD
AstEntry ast_operator_push(Ast* self, ExprSlot lhs, ExprSlot rhs, OperatorType type);

