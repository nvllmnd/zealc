#include "ast/ast.h"
#include <stdlib.h>

#include "ast/expr.h"
#include "constants.h"
#include "core_types.h"
#include "log.h"
#include "memory/alloc.h"
#include "memory/arena.h"
#include "memory/block_alloc.h"

METHOD
static void ast_resize(Ast* self, isize new_cap);

Ast ast_new(isize cap) {
  BlockAllocator* ba = ba_owned_new(2);
  return ast_new_in(cap, ba_allocator(ba));
}

Ast ast_new_in(isize cap, Allocator alloc) {
  Expr* start = allocator_allocate(alloc, make(MemLayout, .size = sizeof(Expr) * cap, .align = alignof(Expr)));
  if (start) {
    return make(Ast, .start = start, .len = 0, .cap = cap, .alloc = alloc);
  }

  return make_zeroed(Ast);
}

METHOD
ExprSlot ast_push(Ast* self, Expr expr) {

  if (self->len >= self->cap) {
    ast_resize(self, self->cap * 2);
  }

  const ExprSlot slot = self->len;
    self->start[slot] = expr;
    self->len++;
    return slot;
  }

// METHOD
// ExprSlot ast_exprlist_push(Ast* self, Expr* exprs, i32 len) {
  
// }

METHOD
ExprSlot ast_bool_push(Ast* self, bool val) {
  const Expr e = make(Expr, .b = val, .type = Expr__Bool);
  return ast_push(self, e);
}

METHOD
ExprSlot ast_int_push(Ast* self, i64 n) {
  const Expr e = make(Expr, .i = n, .type = Expr__Int);
  return ast_push(self, e);
}

METHOD
ExprSlot ast_float_push(Ast* self, f64 n) {
  const Expr e = make(Expr, .f = n, .type = Expr__Float);
  return ast_push(self, e);
}

METHOD
ExprSlot ast_string_push(Ast* self, StringId string) {
  const Expr e = make(Expr, .s = string, .type = Expr__String);
  return ast_push(self, e);
}

METHOD
ExprSlot ast_ident_push(Ast* self, StringId ident) {
  const Expr e = make(Expr, .s = ident, .type = Expr__Ident);

  return ast_push(self, e);
}

METHOD
ExprSlot ast_assign_push(Ast* self, ExprSlot lhs, ExprSlot rhs) {
  const Expr e = make(Expr, .assign = make(ExprAssignment, .lhs = lhs, .rhs = rhs), .type = Expr__Assignment);

  return ast_push(self, e);
}

METHOD
ExprSlot ast_operator_push(Ast* self, ExprSlot lhs, ExprSlot rhs, OperatorType type) {

  const Expr e = make(Expr, .op = make(ExprOperator, .lhs = lhs, .rhs = rhs, .optype = type), .type = Expr__Operator);

  return ast_push(self, e);  
}

void ast_resize(Ast* self, isize new_len) {

  if (new_len <= self->len) {
    self->len = new_len;
    return;
  }
  // if new len is bigger than current, but less than cap
  if (new_len > self->len && new_len < self->cap) {
    self->len = new_len;
    return;
  }

  // need to realloc
  const MemLayout old_layout = make(MemLayout, .size = sizeof(Expr) * self->len, .align = alignof(Expr));
  const MemLayout new_layout = make(MemLayout, .size = sizeof(Expr) * new_len, .align = alignof(Expr));
  self->start = allocator_reallocate(self->alloc, self->start, old_layout, new_layout);
  if (is_null(self->start)) {
    log_fatal("Failed to reallocate Expr Array through Allocator interface struct!");
    return;
  }

  self->cap = new_len;

  
}

struct Expr* ast_index(Ast* self, isize index) {
  if (self->start && index < self->len) {
    return &self->start[index]; 
  }
  return nullptr;
}

METHOD
const struct Expr* ast_index_const(const Ast* self, isize index) {
  if (self->start && index < self->len) {
    return &self->start[index]; 
  }
  return nullptr;  
}
