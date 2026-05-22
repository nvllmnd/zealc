#include "ast/expr.h"

#include <stdarg.h>

#include "nv/memory/alloc.h"


Expr expr_vlist(Allocator alloc, i32 count, Expr first, ...) {
  assert(count >= 1);
  assert(allocator_is_ok(alloc));

  va_list args;
  va_start(args);

  Vec(Expr) list = punwrap(vec_new(Expr, count, alloc));
  vec_push(list, first);

  // reduce count since we already know we have at least 1 (first is part of the count)
  count--;

  for (i32 i = 0; i < count; i++) {
    const Expr e = va_arg(args, Expr);
    vec_push(list, e);
  }

  va_end(args);

  return expr_list(list);
}

Expr expr_vcall(Allocator alloc, Expr* callee, i32 count, Expr first, ...) {
  assert(callee);
  assert(count >= 1);
  assert(allocator_is_ok(alloc));

  va_list args;
  va_start(args);

  Vec(Expr) expr_args = punwrap(vec_new(Expr, count, alloc));
  vec_push(expr_args, first);

  // reduce count since we already know we have at least 1 (first is part of the count)
  count--;

  for (i32 i = 0; i < count; i++) {
    const Expr e = va_arg(args, Expr);
    vec_push(expr_args, e);
  }

  va_end(args);
  return expr_call(callee, expr_args);
}
