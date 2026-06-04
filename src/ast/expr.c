#include "ast/expr.h"

#include <stdarg.h>

#include "ast/lex.h"
#include "ast/token.h"
#include "nv/core/log.h"
#include "nv/core/sslice.h"
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

OperatorType tokentype_optype(TokenType tt) {
  switch (tt) {
    case Token__Gt: {
      return Operator__Gt;
    } break;
    case Token__GtEq: {
      return Operator__Gte;
    } break;
    case Token__Lt: {
      return Operator__Lt;

    } break;
    case Token__LtEq: {
      return Operator__Lte;
    } break;
    case Token__DoubleEq: {
      return Operator__Eq;

    } break;
    case Token__BangEq: {
      return Operator__NotEq;

    } break;

    case Token__Plus: {
      return Operator__Plus;

    } break;
    case Token__Minus: {
      return Operator__Minus;

    } break;
    case Token__Star: {
      return Operator__Mul;

    } break;
    case Token__ForwardSlash: {
      return Operator__Div;

    } break;
    case Token__Percent: {
      return Operator__Modulo;

    } break;
    case Token__And: {
      return Operator__And;

    } break;
    case Token__Or: {
      return Operator__Or;

    } break;
    case Token__Pipe: {
      return Operator__BitOr;

    } break;

    case Token__Ampersand: {
      return Operator__BitAnd;

    } break;
    default: {
      LOG("TokenType: %s does not map to any OperatorType", tokentype_string(tt));
      return Operator__Invalid;
    } break;
  }
}

const char* optype_string(OperatorType op) {
  const sslice sl = optype_slice(op);
  return sl.begin;
}

CONST_FUNC
sslice optype_slice(OperatorType op) {
  static constexpr const sslice OPS[Operator__Count] = {
      [Operator__Plus] = sslice_static_new("+"),
      [Operator__Minus] = sslice_static_new("-"),   [Operator__Mul] = sslice_static_new("*"),
      [Operator__Div] = sslice_static_new("/"),     [Operator__Modulo] = sslice_static_new("%"),
      [Operator__Concat] = sslice_static_new("++"), [Operator__And] = sslice_static_new("and"),
      [Operator__Or] = sslice_static_new("or"),     [Operator__BitOr] = sslice_static_new("|"),
      [Operator__BitAnd] = sslice_static_new("&"),  [Operator__Gt] = sslice_static_new(">"),
      [Operator__Gte] = sslice_static_new(">="),    [Operator__Lt] = sslice_static_new("<"),
      [Operator__Lte] = sslice_static_new("<="),    [Operator__Eq] = sslice_static_new("=="),
      [Operator__NotEq] = sslice_static_new("!="),  [Operator__Not] = sslice_static_new("!"),
      [Operator__Negate] = sslice_static_new("-"),
  };
  if (op <= Operator__Invalid || op >= Operator__Count) {
    return sslice_static_new("Operator Invalid or Out of Bounds!");
  }

  return OPS[op];
}

sslice expr_stmt_type_slice(ExprStmtType t) {
  switch (t) {
    case ExprStmt__None: {
        return sslice_static_new("ExprStmt(__NONE__)");
      } break;
    case ExprStmt__Block: {
        return sslice_static_new("ExprStmt(Block)");
      } break;
    case ExprStmt__Loop: {
        return sslice_static_new("ExprStmt(Loop)");
      } break;
    case ExprStmt__While: {
        return sslice_static_new("ExprStmt(While)");
        
      } break;
    case ExprStmt__When: {

        return sslice_static_new("ExprStmt(When)");
        
      } break;
    case ExprStmt__LetDefine: {
        return sslice_static_new("ExprStmt(Let)");
        
      } break;
    case ExprStmt__ConstDefine: {
        return sslice_static_new("ExprStmt(Const)");
        
      } break;
    case ExprStmt__VarDefine: {
        return sslice_static_new("ExprStmt(Var)");
        
      } break;
    case ExprStmt__FuncDefine: {
        return sslice_static_new("ExprStmt(Function)");
        
      } break;
    case ExprStmt__Return: {
        return sslice_static_new("ExprStmt(Return)");
        
      } break;
    case ExprStmt__Continue: {
        return sslice_static_new("ExprStmt(Continue)");
        
      } break;
    case ExprStmt__Break: {

        return sslice_static_new("ExprStmt(Break)");
        
      } break;
    case ExprStmt__AtomExpr: {
        return sslice_static_new("ExprStmt(Expr)");
        
      } break;
    case ExprStmt__Statement: {
        return sslice_static_new("ExprStmt(Statement)");
      } break;
      break;
  }
}
