#pragma once



#include "ast/expr.h"
#include "nv/core_types.h"
#include "nv/memory/alloc.h"


struct Ast {
  Expr* start;
  i32 len;
  i32 cap;
  Allocator alloc;
};
alias(Ast);


Ast ast_new(isize cap);

Ast ast_new_in(isize cap, Allocator alloc);

METHOD
struct Expr* ast_index(Ast* self, isize index);

METHOD
const struct Expr* ast_index_const(const Ast* self, isize index);


METHOD
ExprSlot ast_push(Ast* self, Expr expr);


METHOD
ExprSlot ast_exprlist_push(Ast* self, Expr* exprs, i32 len);


METHOD
ExprSlot ast_bool_push(Ast* self, bool val);

METHOD
ExprSlot ast_int_push(Ast* self, i64 n);

METHOD
ExprSlot ast_float_push(Ast* self, f64 n);


METHOD
ExprSlot ast_string_push(Ast* self, StringId string);

METHOD
ExprSlot ast_ident_push(Ast* self, StringId ident);

METHOD
ExprSlot ast_assign_push(Ast* self, ExprSlot lhs, ExprSlot rhs);


METHOD
ExprSlot ast_operator_push(Ast* self, ExprSlot lhs, ExprSlot rhs, OperatorType type);



#define ast_free(self) do { \
  __typeof__(self)* _s = &(self); \
  allocator_free(_s->alloc, _s->start); \
  memset(_s, 0, sizeof(__typeof__(self)));\
} while(0) 



