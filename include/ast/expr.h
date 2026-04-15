#pragma once


#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include "memory/cstr.h"
typedef enum ExprType {
  ExprType__Unit,
  ExprType__Bool,
  ExprType__Int,
  ExprType__Float,
  ExprType__String,
  ExprType__Ident,
  ExprType__Pair,
  ExprType__Triple,
  ExprType__List,
  ExprType__Assignment,
  ExprType__Call,
  ExprType__Operator,
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

struct Expr {
  union {
    bool b;
    i32 i;
    f32 f;
    cstr s;
    cstr ident;
    struct ExprList { struct Expr* start; isize len; } list;
    struct ExprAssignment {
      struct Expr* lhs;
      struct Expr* rhs;
    } assign;
    struct ExprCall {
      struct Expr* callee;
      struct Expr* args;
      isize args_len;
    } call;
    struct ExprOperator {
     struct Expr* lhs;
     struct Expr* rhs;
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

