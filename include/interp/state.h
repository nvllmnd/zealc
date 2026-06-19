#pragma once

#include "ast/ast.h"
#include "ast/parser.h"
#include "error.h"
#include "nv/core/algo.h"
#include "nv/core/attributes.h"
#include "nv/iter/vec.h"
#include "runes.h"

typedef enum IValueType {
  IValue__InvalidUnknown = -1,
  IValue__Unit = 0,
  IValue__Bool,
  IValue__Int64,
  IValue__Float64,
  IValue__StringLiteral,
  IValue__String,
  IValue__Bytes,
  IValue__Struct,
  IValue__Map,
  IValue__Set,
  IValue__Array,
  IValue__List,
  IValue__TypeValue,
  IValue__Ident,
  IValue__Rune,
} IValueType;

typedef union IValueData IValueData;

struct IValue {
  IValueType type;
  union IValueData {
    nullptr_t unit;
    bool boolean;
    i64 integer;
    f64 fp;
    /// @brief also used for StringLiteral and ident
    Rune string;

    Vec(struct IValue) list;

    byte bytes[32];
  } data;
};
alias(IValue);

CONST_FUNC
static inline IValue ival_unit(void) { return (IValue){.type = IValue__Unit, .data = {.unit = nullptr}}; }

CONST_FUNC
static inline IValue ival_bool(bool val) { return (IValue){.type = IValue__Bool, .data = {.boolean = val}}; }

CONST_FUNC
static inline IValue ival_int(i64 val) { return (IValue){.type = IValue__Int64, .data = {.integer = val}}; }

CONST_FUNC
static inline IValue ival_float(f64 val) { return (IValue){.type = IValue__Float64, .data = {.fp = val}}; }

PURE_FUNC
static inline IValue ival_stringlit(Rune r) { return (IValue){.type = IValue__StringLiteral, .data = {.string = r}}; }

PURE_FUNC
static inline IValue ival_string(Rune r) { return (IValue){.type = IValue__String, .data = {.string = r}}; }

PURE_FUNC
static inline IValue ival_ident(Rune r) { return (IValue){.type = IValue__Ident, .data = {.string = r}}; }

PURE_FUNC
static inline IValue ival_rune(Rune r) { return (IValue){.type = IValue__Rune, .data = {.string = r}}; }

#define ival_list(_alloc, ...)                               \
  ({                                                         \
    auto _vals = (IValue[]){__VA_ARGS__};                    \
    const i64 _count = sizeof(_vals) / sizeof(_vals[0]);     \
    Vec(IValue) _list = vec_new(IValue, _count, (_alloc));   \
    memcpy(_list, &_vals, sizeof(IValue) * _count);          \
    vec_set_len(_list, _count - 1);                          \
    (IValue){.type = IValue__List, .data = {.list = _list}}; \
  })

struct InterpOpts {
  /// @brief memory region used for storing Abstract Syntax Tree Nodes
  /// @details AST gets cleared at the start of each parse chunk, as such any pointers to AST
  /// notes that remain from last parse should be considered invalid, so make sure you get everything you need from
  /// the AST prior to starting another parse frame
  i64 ast_capacity;
  /// @brief memory region used by interpreter (runtime) environment
  i64 env_capacity;
};
alias(InterpOpts);

typedef enum IError : u64 {
  IError__Ok = 0,
  IError__Error = 1,
  IOK = IError__Ok,
  IERROR = IError__Error,
  IError__Parser = 1 << 1,
  IError__FileLoad = 1 << 2,
  IError__OutOfMemory = 1 << 3,
} HEDLEY_FLAGS IError;

/// @brief init interpreter with default options.
/// @details this function must be called first prior to any other interp_* function calls
/// failure to do so will trigger a runtime panic
void interp_init(InterpOpts opts);

/// @brief Returns an array of dynamic size, containing all parsed AST trees in this parse frame
/// @details All nodes in AST are still valid after this function returns, and shall remain alive and valid
/// up util the next call to ][interp_pframe_start]/[interp_pframe_reset]
Vec(Ast) interp_pframe_end(void);

/// @brief resets parse frame state and clears AST allocator(s) as well
/// @details Pointers to AST nodes still alive to this point should be considered invalid after this function returns
void interp_pframe_start(void);

/// @brief Prase a file and return a parsed AST.
/// @details interpreter does not load this into the ASTs it owns, to do that, pass the returned (valid) AST to
/// [interp_load_ast], or just call [ionterp_load_file] directly
//
/// @returns zeroed AST structure in case of error, otherwise a valid AST
Ast interp_parse_file(const char* path) PARAMS_NONNULL(1);

/// @brief Parse a zeal source string and return a parsed AST
/// @details interpreter does not load this into the ASTs it owns, to do that, pass the returned (valid) AST to
/// [interp_load_ast], or just call [ionterp_load_string] directly
/// @returns zeroed AST structure in case of error, otherwise a valid AST
Ast interp_parse_string(const char* str, i32 len) PARAMS_NONNULL(1);

void interp_load_ast(Ast ast);

/// @brief Parse a zeal file and load the AST into the interpreters inteeral vec of parsed AST trees
IError interp_load_file(const char* path) PARAMS_NONNULL(1);

/// @brief Parse a zeal source string and load the AST into the interpreters inteeral vec of parsed AST trees
IError interp_load_string(const char* str, i32 len) PARAMS_NONNULL(1);

/// @brief Tells zeal interpreter instance to evalate (execute) all the parsed ASTs it currently has loaded
IError interp_evaluate(void);

IValue interp_eval_expr(const Expr* expr) PARAMS_NONNULL(1);

IValue interp_eval_stmt(const ExprStmt* stmt) PARAMS_NONNULL(1);

IValue interp_eval_ast(Ast ast);

PURE_FUNC
IError interp_get_error(void);
