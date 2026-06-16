#include "ast/ast.h"

#include <stdlib.h>

#include "ast/expr.h"
#include "ast/parser.h"
#include "nv/iter/vec.h"
#include "nv/memory/memory.h"
#include "nv/memory/arena.h"
#include "nv/memory/error.h"
#include "strpad.h"


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
      strpad_fappend("%.*s", RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__Rune: {
      strpad_fappend(":%.*s", RSSPREAD(expr->val.rune.name));
      return AstError__Ok;

    } break;
    case Expr__List: {
      const i32 llen = vec_len(expr->val.list);
      strpad_append("[");

      vec_for(expr->val.list) {
        tryerr(stringify_expr(&expr->val.list[i], nullptr));
        if LIKELY (i < llen - 1) {
          strpad_append(", ");
          continue;
        }
      }

      strpad_append("]");
      return AstError__Ok;

    } break;
    case Expr__Assignment: {
      strpad_append("(assign ");

      tryerr(stringify_expr(expr->val.assign.lhs, nullptr));

      strpad_append(" ");

      tryerr(stringify_expr(expr->val.assign.rhs, nullptr));

      strpad_append(")");

      return AstError__Ok;

    } break;
    case Expr__Call: {
      strpad_append("(call ");
      tryerr(stringify_expr(expr->val.call.callee, nullptr));
      strpad_append(" ");
      tryerr(stringify_expr(expr->val.call.args, nullptr));
      strpad_append(")");
      return AstError__Ok;

    } break;
    case Expr__BinOp: {
      strpad_fappend("(%s ", optype_string(expr->val.binop.optype));

      tryerr(stringify_expr(expr->val.binop.lhs, nullptr));
      strpad_append(" ");
      tryerr(stringify_expr(expr->val.binop.rhs, nullptr));
      strpad_append(")");
      return AstError__Ok;

    } break;
    case Expr__UnaryOp: {
      strpad_fappend("(%s ", optype_string(expr->val.binop.optype));
      tryerr(stringify_expr(expr->val.uop.rhs, nullptr));
      strpad_append(")");
      return AstError__Ok;

    } break;
    case Expr__PrintCall: {
      strpad_fappend("(%s ", expr->val.pc.type == Print__Newline ? "println" : "print");
      tryerr(stringify_expr(expr->val.pc.rhs, nullptr));
      strpad_append(")");

      return AstError__Ok;
    } break;
      break;
  }
}

PARAMS_NONNULL(1)
static AstWalkError stringify_expr_stmt(ExprStmt* es, void*) {
  assert(es);

  switch (es->type) {
    case ExprStmt__Block: {
      strpad_append("(begin \n");
      if (is_null(es->block)) {
        LOG_FATAL("WHAT");
      }
      vec_foreach(es->block) { tryerr(stringify_expr_stmt(iter, nullptr)); }
      strpad_append("\n end)\n");
    } break;
    case ExprStmt__Loop: {
    } break;
    case ExprStmt__While: {
    } break;
    case ExprStmt__When: {
    } break;
    case ExprStmt__LetDefine: {
      strpad_fappend("(let %.*s ", RSSPREAD(es->def.name.name));
      tryerr(stringify_expr(es->def.rhs, nullptr));
      strpad_append(")\n");
    } break;
    case ExprStmt__ConstDefine: {
    } break;
      strpad_fappend("(const %.*s ", RSSPREAD(es->def.name.name));
      tryerr(stringify_expr(es->def.rhs, nullptr));
      strpad_append(")\n");
    case ExprStmt__VarDefine: {
      strpad_fappend("(var %.*s ", RSSPREAD(es->def.name.name));
      tryerr(stringify_expr(es->def.rhs, nullptr));
      strpad_append(")\n");
    } break;
    case ExprStmt__FuncDefine: {
    } break;

    case ExprStmt__Return: {
    } break;
    case ExprStmt__Continue: {
    } break;
    case ExprStmt__Break: {
    } break;
    case ExprStmt__AtomExpr: {
      tryerr(stringify_expr(&es->expr, nullptr));
      strpad_append("\n");
    } break;
    case ExprStmt__Statement: {
      TODO();
    } break;
      break;
    case ExprStmt__None: {
      strpad_append("NONE");
    } break;
      break;
    case ExprStmt__AstChunkEnd:
      strpad_append("=== AST END ===");
      break;
  }
  return (AstWalkError)OK;
}

void ast_walk(Ast* self) {
  assert(self);
  assert(self->root);
  assert(self->walker.walk_expr);
  assert(self->walker.walk_expr_stmt);

  for(i32 i = 0; i < vec_len(self->root); i++) {
    ExprStmt* e = &self->root[i];
    self->walker.walk_expr_stmt(e, self->walker.userdata);
  }

  // vec_foreach(self->root) {
  //   LOG("ITERATING");
  //   if (iter) {
  //     self->walker.walk_expr_stmt(iter, self->walker.userdata);
  //   } else {
  //     LOG_DBG("NULL ITER:");
  //   }
  // }
}

sslice ast_stringify(Ast* self, Allocator alloc) {
  assert(self);
  assert(self->root);

  strpad_start();

  ast_walker_init(self, nullptr, stringify_expr, stringify_expr_stmt);
  ast_walk(self);

  const sslice ast = strpad_end(alloc);

  return ast;
}
