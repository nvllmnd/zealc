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
#include "strpad.h"

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
      strpad_append("()");
      return AstError__Ok;
    } break;
    case Expr__Bool: {
      strpad_fappend("%s", expr->val.b ? "true" : "false");
      return AstError__Ok;
    } break;
    case Expr__Int: {
      strpad_fappend("%li", expr->val.i);
      return AstError__Ok;

    } break;
    case Expr__Float: {
      strpad_fappend("%f", expr->val.f);
      return AstError__Ok;

    } break;
    case Expr__StringLiteral: {
      strpad_fappend("\"%.*s\"", RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__String: {
      strpad_fappend("\"%.*s\"", RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__Ident: {
      strpad_fappend(":%.*s", RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__Rune: {
      strpad_fappend("%.*s", RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__List: {
      const i32 llen = vec_len(expr->val.list);
      strpad_putchar('[');

      vec_for(expr->val.list) {
        tryerr(stringify_expr(&expr->val.list[i], nullptr));
        if LIKELY (i < llen - 1) {
          strpad_append(", ");
          continue;
        }
      }

      strpad_putchar(']');
      return AstError__Ok;

    } break;
    case Expr__Assignment: {
      strpad_append("(assign ");

      tryerr(stringify_expr(expr->val.assign.lhs, nullptr));

      strpad_putchar(' ');

      tryerr(stringify_expr(expr->val.assign.rhs, nullptr));

      strpad_putchar(')');

      return AstError__Ok;

    } break;
    case Expr__Call: {
      strpad_append("(call ");
      tryerr(stringify_expr(expr->val.call.callee, nullptr));
      strpad_putchar(' ');
      tryerr(stringify_expr(expr->val.call.args, nullptr));
      strpad_putchar(')');
      return AstError__Ok;

    } break;
    case Expr__BinOp: {
      strpad_fappend("(%s", optype_string(expr->val.binop.optype));
      tryerr(stringify_expr(expr->val.binop.lhs, nullptr));
      strpad_putchar(' ');
      tryerr(stringify_expr(expr->val.binop.rhs, nullptr));
      strpad_putchar(')');
      return AstError__Ok;

    } break;
    case Expr__UnaryOp: {
      strpad_fappend("(%s", optype_string(expr->val.binop.optype));
      tryerr(stringify_expr(expr->val.uop.rhs, nullptr));
      strpad_putchar(')');
      return AstError__Ok;

    } break;
    case Expr__PrintCall: {
      strpad_fappend("(%s", expr->val.pc.type == Print__Newline ? "println" : "print");
      tryerr(stringify_expr(expr->val.pc.rhs, nullptr));
      strpad_putchar(')');
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

  vec_foreach(self->root) { self->walker.walk_expr(*iter, self->walker.userdata); }
}

sslice ast_stringify(Ast* self) {
  assert(self);
  assert(self->root);
  // const i32 len = vec_len(self->root);
  //

  strpad_start();

  ast_walker_init(self, nullptr, stringify_expr, stringify_expr_stmt);
  ast_walk(self);

  const sslice ast = strpad_end(arena_allocator(self->alloc));

  LOG(":: AST STRINGIFY RESULT ::\n%*.s", RSSPREAD(ast));
  return ast;
}
