#pragma once

#include <unistd.h>

#include "nv/memory/cstr.h"



typedef enum ExprType {
  Expr__Unit,
  Expr__Bool,
  Expr__Int,
  Expr__Float,
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
  ExprStmtType__Block,
  ExprStmtType__Loop,
  ExprStmtType__While,
  ExprStmtType__When,
  ExprStmtType__DefFunc,
  ExprStmtType__Binding,
  ExprStmtType__Escape,
  ExprStmtType__AtomExpr,
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
    StringId s;
    RuneId rune; 

    struct ExprList {
      ExprSlot start; 
      i32 len;
    } list;
    // cstr s;
    // cstr rune;
    // struct ExprList {
    //   struct Expr* start;
    //   isize len;
    // } list;
    struct ExprAssignment {
      ExprSlot lhs;
      ExprSlot rhs;
      // struct Expr* lhs;
      // struct Expr* rhs;
    } assign;
    // struct ExprCall {
    //   struct Expr* callee;
    //   struct Expr* args;
    //   isize args_len;
    // } call;
    struct ExprOperator {
      ExprSlot lhs;
      ExprSlot rhs;
      // struct Expr* lhs;
      // struct Expr* rhs;
      OperatorType optype;
    } op;
  };

  ExprType type;
};
typedef struct Expr Expr;
typedef struct ExprList ExprList;
typedef struct ExprAssignment ExprAssignment;
typedef struct ExprCall ExprCall;
typedef struct ExprOperator ExprOperator;


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
