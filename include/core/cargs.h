#pragma once

#include "nv/core/algo.h"
#include "nv/core/attributes.h"

typedef enum CArgFlags : u64 {
  CArg__None = 0,
  CArg__Script = 1 << 0,
  CArg__Compiler = 1 << 1,
  CArg__TargetRISCV = 1 << 2,
  CArg__Targetx86_64 = 1 << 3,
  CArg__TargetLLVM = 1 << 4,
  CArg__Interpreter = 1 << 5,
  CArg__OutputDir = 1 << 6,
  CArg__RootDir = 1 << 7,
  CArg__FilesInput = 1 << 8,
  CArg__Repl = 1 << 9,
  CArg__BuildFile = 1 << 10,

  CARG_COUNT = 11,
} HEDLEY_FLAGS CArgFlags;

typedef enum OptArgType {
  OptArg__None = 0,
  OptArg__FilePath,
  OptArg__Integer,
  OptArg__Boolean,
  OptArg__String,
} OptArgType;

struct OptArg {
  OptArgType type;
  union {
    const char* string;
    i64 num;
    bool boolean;
    usize index;
  };
};
alias(OptArg);

static constexpr const i64 MAX_CLI_ARGS = 32;

struct CArgs {
  OptArg args[MAX_CLI_ARGS];

  CArgFlags flags;

  const char** argv;
  i32 argc;
};
alias(CArgs);
