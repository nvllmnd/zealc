#pragma once



#include "ast/expr.h"
#include "core_types.h"


/// A flat list of [Expr]s (or [ExprStmt]s once those are implemented).
/// behaves like a vec of Expr, although you cannot index a pointer to this type
/// and expect to get an [Expr], see: [ast_index] for that
typedef struct Ast Ast;

// struct Ast {
//   Expr* start;
//   i32 len;
//   i32 cap;
// };
// alias(Ast);


Ast* ast_new(isize cap);


METHOD
void ast_push(Ast* self, Expr expr);


