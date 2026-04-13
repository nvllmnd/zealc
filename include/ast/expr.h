#pragma once


#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
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



struct Expr {
  ExprType type;  
};
typedef struct Expr Expr;

