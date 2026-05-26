#include "ast/ast.h"

#include <stdlib.h>

#include "ast/expr.h"
#include "nv/core.h"
#include "nv/iter.h"
#include "nv/iter/string.h"
#include "nv/iter/vec.h"
#include "nv/memory.h"
#include "nv/memory/arena.h"
#include "nv/memory/error.h"
#include "nv/memory/virt.h"
#include "talloc.h"

struct StringifyState {
  String str;
};
alias(StringifyState);

PARAMS_NONNULL(1)
static AstWalkError stringify_expr(Expr* expr, void*) {
  switch (expr->type) {
    case Expr__Invalid: {
      LOG("Ast Stringifier encountered an Expression with type Invalid while walking the AST!");
      return AstError__EncounteredInvalidExpr;
    } break;
    case Expr__Unit: {
      talloc_append("()");
      return AstError__Ok;
    } break;
    case Expr__Bool: {
      spad_fappend("%s", expr->val.b ? "true" : "false");
      return AstError__Ok;
    } break;
    case Expr__Int: {
      spad_fappend("%li", expr->val.i);
      return AstError__Ok;

    } break;
    case Expr__Float: {
      spad_fappend("%f", expr->val.f);
      return AstError__Ok;

    } break;
    case Expr__StringLiteral: {
      talloc_fappend_delim("\"%.*s", '"', RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__String: {
      talloc_fappend_delim("\"%.*s", '"', RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__Ident: {
      spad_fappend(":%.*s", RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__Rune: {
      spad_fappend("%.*s", RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__List: {
      const i32 llen = vec_len(expr->val.list);
      talloc_putchar('[');

      vec_for(expr->val.list) {
        tryerr(stringify_expr(&expr->val.list[i], nullptr));
        if LIKELY (i < llen - 1) {
          talloc_append(", ");
          continue;
        }
      }

      talloc_putchar(']');
      return AstError__Ok;

    } break;
    case Expr__Assignment: {
      talloc_append("(assign ");

      tryerr(stringify_expr(expr->val.assign.lhs, nullptr));

      talloc_putchar(' ');

      tryerr(stringify_expr(expr->val.assign.rhs, nullptr));

      talloc_putchar(')');

      return AstError__Ok;

    } break;
    case Expr__Call: {
      talloc_append("(call ");
      tryerr(stringify_expr(expr->val.call.callee, nullptr));
      talloc_putchar(' ');
      tryerr(stringify_expr(expr->val.call.args, nullptr));
      talloc_putchar(')');
      return AstError__Ok;

    } break;
    case Expr__BinOp: {
      talloc_fspush_sp("(%s", optype_string(expr->val.binop.optype));
      tryerr(stringify_expr(expr->val.binop.lhs, nullptr));
      talloc_putchar(' ');
      tryerr(stringify_expr(expr->val.binop.rhs, nullptr));
      talloc_putchar(')');
      return AstError__Ok;

    } break;
    case Expr__UnaryOp: {
      talloc_fspush_sp("(%s", optype_string(expr->val.binop.optype));
      tryerr(stringify_expr(expr->val.uop.rhs, nullptr));
      talloc_putchar(')');
      return AstError__Ok;

    } break;
    case Expr__PrintCall: {
      talloc_fspush_sp("(%s", expr->val.pc.type == Print__Newline ? "println" : "print");
      tryerr(stringify_expr(expr->val.pc.rhs, nullptr));
      talloc_putchar(')');
      return AstError__Ok;

    } break;
      break;
  }
}

PARAMS_NONNULL(1)
static AstWalkError stringify_expr_stmt(ExprStmt* expr_stmt, void* ctx) {
  UNUSED(expr_stmt);
  UNUSED(ctx);
  TODO();
}

void ast_walk(Ast* self) {
  assert(self);
  assert(self->walker.walk_expr);
  assert(self->walker.walk_expr_stmt);

  vec_foreach(self->root) { self->walker.walk_expr(iter, self->walker.userdata); }
}

sslice ast_stringify(Ast* self) {
  assert(self);
  assert(self->root);
  // const i32 len = vec_len(self->root);

  if (!talloc_is_init()) {
    if (talloc_init(make(VirtMemOpts, .size_in_mb = 1024, .initial_commit = KILOBYTES(16))) != MemError__Ok) {
      LOG("Error initializing Temporary Allocator, needed for AST Stringifying!");
      abort();
    }
  } else {
    talloc_clear();
  }

  talloc_string_begin();

  ast_walker_init(self, nullptr, stringify_expr, stringify_expr_stmt);
  ast_walk(self);

  talloc_string_end();

  const sslice ast = talloc_as_string();

  const char* s = arena_strndup(self->alloc, SSPREAD(ast));
  assert(s);

  talloc_clear();

  return sslice_new(.begin = s, .len = ast.len);
}
