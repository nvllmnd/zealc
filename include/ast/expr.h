#pragma once

#include <assert.h>

#include "nv/core/attributes.h"
#include "nv/core_types.h"
#include "nv/iter.h"
#include "runes.h"

typedef enum ExprType {
  Expr__Invalid = -1,
  Expr__Unit,
  Expr__Bool,
  Expr__Int,
  Expr__Float,
  Expr__StringLiteral,
  Expr__String,
  Expr__Ident,
  Expr__Rune,
  Expr__List,
  Expr__Assignment,
  Expr__Call,
  Expr__BinOp,
  Expr__UnaryOp,
  Expr__PrintCall,
} ExprType;

typedef enum ExprStmtType {
  ExprStmt__None,
  /// Block expression statements, similar to Rust (let x = { 5 };)
  ExprStmt__Block,
  /// Infinite Loop expression statement, also simliar to Rust (but not currently)
  ExprStmt__Loop,
  /// While loop expression statement
  ExprStmt__While,
  /// When expression statement, similar to Elixir's cond keyword
  ExprStmt__When,
  /// Let binding/define expression statement
  ExprStmt__LetDefine,
  /// Const binding/define expression statement
  ExprStmt__ConstDefine,
  /// Variable binding/define expression statement
  ExprStmt__VarDefine,
  /// Function Definition
  ExprStmt__FuncDefine,
  /// Return statement
  ExprStmt__Return,
  /// Continue expression statement
  ExprStmt__Continue,
  /// Break expression statement
  ExprStmt__Break,
  /// Contains/reduces to a simple Expr
  ExprStmt__AtomExpr,
  /// A 'top-level' Statement ('top-level' in terms of language synchronization points, this is at the top) This is most
  /// likely some expression (like a function call) where the result of the expression is discarded. most likely a
  /// function that mutates state in some way. but not always! (for instance, calling a function that returns a value,
  /// but we discard the return value, most likely going to disallow users of Zeal to silently discard return values
  /// like rust, instead letting them do the _ = ... underscore meme)
  /// This is different from an [ExprStmt__AtomExpr] to differentiate between true top-level statements and any other
  /// situation where an ExprStmt could reduce to an Expr
  ExprStmt__Statement,
} ExprStmtType;

typedef enum OperatorType {
  Operator__Invalid = -1,
  Operator__Plus,
  Operator__Minus,
  Operator__Mul,
  Operator__Div,
  Operator__Modulo,
  Operator__Concat,
  Operator__And,
  Operator__Or,
  Operator__BitOr,
  Operator__BitAnd,
  Operator__Gt,
  Operator__Gte,
  Operator__Lt,
  Operator__Lte,
  Operator__Eq,
  Operator__NotEq,
  Operator__Not,
  Operator__Negate,
  Operator__Count,
} OperatorType;

CONST_FUNC
OperatorType tokentype_optype(TokenType tt);

CONST_FUNC
RETURNS_NON_NULL
const char* optype_string(OperatorType op);

CONST_FUNC
sslice optype_slice(OperatorType op);

CONST_FUNC
sslice expr_stmt_type_slice(ExprStmtType t);

typedef i64 StringId;
typedef i64 RuneId;
typedef i32 ExprSlot;

typedef enum PrintType {
  /// println
  Print__Newline,
  /// print
  Print__Format,
} PrintType;

/// Each root expression node lives on stack, which points to the rest of the AST
struct Expr {
  union {
    bool b;
    i64 i;
    f64 f;
    /// Used for either [Expr__Ident], [Expr__Rune], or [Expr__StringLiteral]
    /// TODO: Add a field for runtime strings that can be mutated when i implement (dynamic) arrays for Zeal
    Rune rune;

    Vec(struct Expr) list;

    struct AssignmentExpr {
      struct Expr* lhs;
      struct Expr* rhs;
    } assign;
    struct CallExpr {
      struct Expr* callee;
      Vec(struct Expr) args;
    } call;
    struct BinOpExpr {
      struct Expr* lhs;
      struct Expr* rhs;
      OperatorType optype;
    } binop;

    struct UnaryOpExpr {
      struct Expr* rhs;
      OperatorType optype;
    } uop;

    /// Temporary until we get native function calls working so I dont lose my sanity
    struct PrintCall {
      PrintType type;
      struct Expr* rhs;

    } pc;
  } val;

  ExprType type;
};

typedef struct Expr Expr;
typedef struct AssignmentExpr AssignmentExpr;
typedef struct CallExpr CallExpr;
typedef struct BinOpExpr BinOpExpr;
typedef struct UnaryOpExpr UnaryOpExpr;
typedef struct PrintCall PrintCall;

/// Type alias for Vec(Expr).
/// WARN: This is a pointer!!! treat it as such!
typedef Vec(Expr) VecExpr;

CONST_FUNC
static inline Expr expr_invalid(void) { return (Expr){.type = Expr__Invalid, .val = {}}; }

CONST_FUNC
static inline Expr expr_unit(void) { return (Expr){.type = Expr__Unit, .val = {}}; }

CONST_FUNC
static inline Expr expr_bool(bool val) { return (Expr){.type = Expr__Bool, .val.b = val}; }

CONST_FUNC
static inline Expr expr_int(i64 val) { return (Expr){.type = Expr__Int, .val.i = val}; }

CONST_FUNC
static inline Expr expr_float(f64 val) { return (Expr){.type = Expr__Float, .val.f = val}; }

CONST_FUNC
static inline Expr expr_rune(Rune val) { return (Expr){.type = Expr__Rune, .val.rune = val}; }

CONST_FUNC
static inline Expr expr_strlit(Rune val) { return (Expr){.type = Expr__StringLiteral, .val.rune = val}; }

CONST_FUNC
static inline Expr expr_ident(Rune val) { return (Expr){.type = Expr__Ident, .val.rune = val}; }

CONST_FUNC
static inline Expr expr_list(Vec(Expr) val) { return (Expr){.type = Expr__List, .val.list = val}; }

/// Creates a new List Expr with a variadic number of [Expr]s.
/// Count parameter includes the first Expr param, as the @param (Expr first) is
/// solely for clarity that this function takes a variadic number of Exprs
/// Takes an [Allocator] param to create the Vec needed for this List Expr
CONST_FUNC
Expr expr_vlist(Allocator alloc, i32 count, Expr first, ...);

CONST_FUNC
PARAMS_NONNULL(1, 2)
static inline Expr expr_assign(Expr* lhs, Expr* rhs) {
  assert(lhs);
  assert(rhs);
  return (Expr){.type = Expr__Assignment, .val.assign = {.lhs = lhs, .rhs = rhs}};
}

CONST_FUNC
PARAMS_NONNULL(1, 2)
static inline Expr expr_call(Expr* callee, Vec(Expr) args) {
  assert(callee);
  assert(args);
  return (Expr){.type = Expr__Call, .val.call = {.callee = callee, .args = args}};
}

/// Creates a new Call Expr with a variadic number of [Expr]s as arguments.
/// Count parameter includes the first Expr param, as the @param (Expr first) is
/// solely for clarity that this function takes a variadic number of Exprs
/// Takes an [Allocator] param to create the Vec needed for this CallExpr
CONST_FUNC
PARAMS_NONNULL(2)
Expr expr_vcall(Allocator alloc, Expr* callee, i32 count, Expr first, ...);

CONST_FUNC
PARAMS_NONNULL(1, 2)
static inline Expr expr_operator(Expr* lhs, Expr* rhs, OperatorType type) {
  assert(lhs);
  assert(rhs);
  return (Expr){.type = Expr__BinOp, .val.binop = {.lhs = lhs, .rhs = rhs, .optype = type}};
}

typedef enum WhenExprStmtType { When__Branch, When__Else, When__End } WhenExprStmtType;

struct WhenExprStmt {
  WhenExprStmtType type;
  union {
    struct WhenBranch {
      Expr* condition;
      Vec(struct ExprStmt) body;
    } branch;

    struct ExprStmt* else_es;
  };
};

typedef struct WhenExprStmt WhenExprStmt;
typedef struct WhenBranch WhenBranch;

/// metadata for binding identifiers to function parameters, struct fields,
/// and let,const,var definitions
struct BindDefine {
  /// always false for now, plan to use later
  bool is_const;
  Rune name;
  // TODO: Add more fields, like Type
};

typedef struct BindDefine BindDefine;

struct DefFunc {
  Rune name;
  Vec(BindDefine) args;
  struct ExprStmt* body;
};

typedef struct DefFunc DefFunc;

typedef enum DefineStmtType {
  Define__Let,
  Define__Var,
  Define__Const,
} DefineStmtType;

struct DefineStmt {
  DefineStmtType type;
  Rune name;
  Expr* rhs;
};
typedef struct DefineStmt DefineStmt;

struct ExprStmt {
  ExprStmtType type;

  union {
    Vec(struct ExprStmt) block;
    struct WhileExprStmt {
      Expr* condition;
      Vec(struct ExprStmt) body;
    } while_es;

    DefineStmt def;

    WhenExprStmt when;
    /// Used for [ExprStmt__FuncDefine]
    DefFunc def_func;
    /// Used for [ExprStmt__Return], [ExprStmt__Break], and [ExprStmt__AtomExpr]
    Expr expr;
  };
};
typedef struct ExprStmt ExprStmt;

CONST_FUNC
PARAMS_NONNULL(1)
static inline ExprStmt expr_print_call(Expr* rhs, PrintType type) {
  assert(rhs);
  return (ExprStmt){.type = ExprStmt__AtomExpr,
                    .expr = (Expr){.type = Expr__PrintCall, .val.pc = {.type = type, .rhs = rhs}}};
}
