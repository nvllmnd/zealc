#include "interp/state.h"

#include <stdio.h>

#include "ast/parser.h"
#include "nv/core/constants.h"
#include "nv/iter/vec.h"
#include "nv/memory/vmem.h"

/// @brief state for a single parse cycle
/// @details a parse cycle (frame) starts anytime source code is desired to be parsed into an AST, and ends then next
/// time a new chunk of source code is called to be parsed into an AST as such all state in frame is cleared and/or
/// reset back to either some constant defined default state, or a configurable one (TBD)
struct ParseFrame {
  /// @brief reset to initial state and set to parse the next source chunk at the start of each parse frame
  Parser p;

  /// @brief filled during this parse frame
  /// @details at the end of this parse frame, this vec will not grow any longer (unless done so by external  code)
  Vec(Ast) trees;
};
alias(ParseFrame);



struct Interp {
  ParseFrame frame;
  IError err;
  struct {
    VArena ast;
    VArena env;
    VArena source;
  } allocs;
};
alias(Interp);

static Interp Self = {};

static constexpr const i64 DEFAULT_AST_CAPACITY = MB(128);
static constexpr const i64 DEFAULT_ENV_CAPACITY = GB(4);
static constexpr const i64 DEFAULT_FRAME_TREES_CAP = 24;

IError interp_get_error(void) { return Self.err; }

static inline const char* load_file_string(const char* path, i64* size_out) {
  FILE* file = fopen(path, "r");

  if (is_null(file)) {
    DERR("Failed to open file %s for interpretation!", path);
    return nullptr;
  }

  fseek(file, 0, SEEK_END);
  const i64 size = ftell(file);
  rewind(file);

  if (size_out) {
    *size_out = size;
  }

  char* dest = va_allocate(&Self.allocs.source, mlayout_bytes(size + 1));
  if UNLIKELY (is_null(dest)) {
    LOG_FATAL(
        "Ran out of space for evaluation of zeal source! file: %s has size of: %li bytes but Interpreter only has %li "
        "bytes "
        "available for reading source code files!",
        path, size, va_available(&Self.allocs.source));
  }
  const i64 count = fread(dest, 1, size, file);
  if UNLIKELY (count != size) {
    LOG_FATAL("Failed to read file %s into memory!", path);
  }
  dest[size] = 0;
  assert(fclose(file) != -1);
  return dest;
}

/// @details this function must be called first prior to any other interp_* function calls
/// failure to do so will trigger a runtime panic
void interp_init(const InterpOpts opts) {
  if UNLIKELY (!is_zeroed(&Self)) {
    DERROR("Interpreter already initialized!");
    return;
  }

  const i64 ast_cap = opts.ast_capacity <= 0 ? DEFAULT_AST_CAPACITY : opts.ast_capacity;
  const i64 env_cap = opts.env_capacity <= 0 ? DEFAULT_ENV_CAPACITY : opts.env_capacity;

  VArena ast = va_new(ast_cap);
  if UNLIKELY (is_zeroed(&ast)) {
    LOG_FATAL("Failed to Initialize Arena for AST of size %li bytes! (%li)MB", ast_cap, of_megabytes(ast_cap));
  }
  VArena env = va_new(env_cap);

  if UNLIKELY (is_zeroed(&env)) {
    LOG_FATAL("Failed to Initialize Arena of size %li bytes! (%li)MB for Interpreter Environment", env_cap,
              of_megabytes(env_cap));
  }

  const i64 source_cap = env_cap + ast_cap;
  VArena source = va_new(source_cap);

  if UNLIKELY (is_zeroed(&env)) {
    LOG_FATAL("Failed to Initialize Arena of size %li bytes! (%li)MB for Interpreter Environment", source_cap,
              of_megabytes(source_cap));
  }

  Vec(Ast) trees = vec_new(Ast, DEFAULT_FRAME_TREES_CAP, va_allocator(&ast));
  if UNLIKELY (is_null(trees)) {
    LOG_FATAL(
        "Successfully initialized Memory Arenas for Zeal Interpreter, but allocating an initial Vec to hold ASTs "
        "failed!");
  }

  Self = (Interp){.frame = {.trees = trees}, .allocs = {.ast = ast, .env = env, .source = source}};
  Self.frame.p = parser_new(&Self.allocs.ast);
}

/// @brief Returns an array of dynamic size, containing all parsed AST trees in this parse frame
/// @details All nodes in AST are still valid after this function returns, and shall remain alive and valid
/// up util the next call to ][interp_pframe_start]/[interp_pframe_reset]
Vec(Ast) interp_pframe_end(void) { return Self.frame.trees; }

/// @brief resets parse frame state and clears AST allocator(s) as well
/// @details Pointers to AST nodes still alive to this point should be considered invalid after this function returns
/// (with no error)
void interp_pframe_reset(void) {
  va_clear(&Self.allocs.ast);
  va_clear(&Self.allocs.env);
  va_clear(&Self.allocs.source);
  parser_reset(&Self.frame.p);
}

/// @brief Prase a file and return a parsed AST.
/// @details interpreter does not load this into the ASTs it owns, to do that, pass the returned (valid) AST to
/// [interp_load_ast], or just call [ionterp_load_file] directly
//
/// @returns zeroed AST structure in case of error, otherwise a valid AST
Ast interp_parse_file(const char* path) {
  i64 file_size = 0;
  const char* source = load_file_string(path, &file_size);
  if (is_null(source)) {
    return zeroed(Ast);
  }

  return interp_parse_string(source, file_size);
}

/// @brief Parse a zeal source string and return a parsed AST
/// @details interpreter does not load this into the ASTs it owns, to do that, pass the returned (valid) AST to
/// [interp_load_ast], or just call [ionterp_load_string] directly
/// @returns zeroed AST structure in case of error, otherwise a valid AST
Ast interp_parse_string(const char* str, i32 len) {
  parser_reset(&Self.frame.p);
  return parser_parse_ast(&Self.frame.p, str, len);
}

void interp_load_ast(Ast ast) {
  Vec(Ast) trees = Self.frame.trees;
  const i64 len = vec_len(trees);
  const i64 cap = vec_capacity(trees);
  if UNLIKELY (len >= cap) {
    Self.frame.trees = vec_resize(trees, cap * 2, va_allocator(&Self.allocs.ast));
    trees = Self.frame.trees;
  }
  vec_push(trees, ast);
}

/// @brief Parse a zeal file and load the AST into the interpreters inteeral vec of parsed AST trees
IError interp_load_file(const char* path) {}

/// @brief Parse a zeal source string and load the AST into the interpreters inteeral vec of parsed AST trees
IError interp_load_string(const char* str, i32 len) {}

/// @brief Tells zeal interpreter instance to evalate (execute) all the parsed ASTs it currently has loaded
IError interp_evaluate(void) {
  IError err = IOK;
  vec_for(&Self.frame.trees) {
    (void)interp_eval_ast(Self.frame.trees[i]);
    err |= Self.err;
  }
  vec_clear(&Self.frame.trees);
  return err;
}

PARAMS_NONNULL(1)
static IValue eval_expression(const Expr* expr) {
  switch (expr->type) {
    case Expr__Invalid: {
      LOG_FATAL("Encountered Invalid Expression!");
    } break;
    case Expr__Unit: {
      return ival_unit();
    } break;
    case Expr__Bool: {
      return ival_bool(expr->val.b);
    } break;
    case Expr__Int: {
      return ival_int(expr->val.i);
    } break;
    case Expr__Float: {
      return ival_float(expr->val.f);
    } break;
    case Expr__StringLiteral: {
      return ival_stringlit(expr->val.rune);
    } break;
    case Expr__String: {
      return ival_string(expr->val.rune);
    } break;
    case Expr__Ident: {
      return ival_ident(expr->val.rune);
    } break;
    case Expr__Rune: {
      return ival_rune(expr->val.rune);
    } break;
    case Expr__List: {
      const i64 cap = vec_capacity(expr->val.list);
      const i64 len = vec_len(expr->val.list);
      Vec(IValue) vals = vec_new(IValue, cap, va_allocator(&Self.allocs.ast));
      for (i32 i = 0; i < len; i++) {
        const IValue v = eval_expression(&expr->val.list[i]);
        vec_push(vals, v);
      }
      return (IValue){.type = IValue__List, .data = {.list = vals}};

    } break;
    case Expr__Assignment: {
      const IValue lhs = eval_expression(expr->val.assign.lhs);
      assert(lhs.type == IValue__Ident);
      const IValue rhs = eval_expression(expr->val.assign.rhs);

      const Rune ident = lhs.data.string;

    } break;
    case Expr__Call: {
    } break;
    case Expr__BinOp: {
    } break;
    case Expr__UnaryOp: {
    } break;
    case Expr__PrintCall: {
    } break;
      break;
  }
}

static IValue eval_statement(const ExprStmt* stmt) {}

IValue interp_eval_expr(const Expr* expr) {}

IValue interp_eval_stmt(const ExprStmt* stmt) {}

IValue interp_eval_ast(Ast ast) {
  vec_foreach(ast.root) { IValue val = eval_statement(iter); }
}
