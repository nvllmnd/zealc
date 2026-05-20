#pragma once

#include <unistd.h>

#include "nv/core/buffer.h"
#include "nv/memory/cstr.h"
#include "runes.h"

typedef enum ExprType {
  Expr__Unit,
  Expr__Bool,
  Expr__Int,
  Expr__Float,
  Expr__StringLiteral,
  Expr__String,
  Expr__Ident,
  Expr__Rune,
  Expr__Pair,
  Expr__Triple,
  Expr__List,
  Expr__Assignment,
  Expr__Call,
  Expr__Operator,
} ExprType;

typedef enum ExprStmtType {
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
  /// Variable assignment
  ExprStmt__Assign,
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
  OperatorType__Plus,
  OperatorType__Minus,
  OperatorType__Mul,
  OperatorType__Div,
  OperatorType__Modulo,
  OperatorType__Concat,
} OperatorType;

typedef i64 StringId;
typedef i64 RuneId;
typedef i32 ExprSlot;

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
    struct OperatorExpr {
      struct Expr* lhs;
      struct Expr* rhs;
      OperatorType optype;
    } op;
  };

  ExprType type;
};
typedef struct Expr Expr;
typedef struct AssignmentExpr AssignmentExpr;
typedef struct CallExpr CallExpr;
typedef struct OperatorExpr OperatorExpr;

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
  Expr rhs;
};
typedef struct DefineStmt DefineStmt;

struct ExprStmt {
  ExprStmtType type;

  union {
    Vec(struct ExprStmt) list;
    struct WhileExprStmt {
      Expr condition;
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

// struct WhenFormList {
//   struct WhenForm* start;
//   i32 len;
// };
// typedef struct WhenFormList WhenFormList;

// typedef enum WhenFormType {
//   WhenForm__Branch,
//   WhenForm__Else,
//   WhenForm__End,
// } WhenFormType;

// struct WhenForm {
//   union {
//     struct WhenFormBranch {
//       Expr* cond;
//       struct ExprStmt* body;
//     } branch;
//     struct ExprStmt* else_branch;
//     // end variant has no data, but can be accessed through [WhenFormType] type field on in this struct
//   };

//   WhenFormType type;
// };
// typedef struct WhenForm WhenForm;
// typedef struct WhenFormBranch WhenFormBranch;

// struct ExprStmtList {
//   struct ExprStmt* start;
//   i32 len;
// };
// typedef struct ExprStmtList ExprStmtList;

// struct FuncDecl {
//   cstr
// };
// typedef struct FuncDecl FuncDecl;

// struct ExprStmt {
//   union {
//     ExprStmtList block;
//     ExprStmtList loop;
//     struct WhileLoopStmt {
//       Expr* cond;
//       ExprStmtList body;
//     } wloop;
//   } when;
// };

// ExprStmtType type;

// typedef struct ExprStmt ExprStmt;
// struct WhenForm {
//  union {
//     struct WhenFormBranch {
//       Expr* cond;
//       struct ExprStmt* body;
//     } branch;
//   }* start;
// };
