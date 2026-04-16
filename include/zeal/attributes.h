// SPDX-FileCopyrightText: 2025 Matthew McDade <zedex805@protonmail.com>
//
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#ifndef __cplusplus
#define noexcept
#endif

#include "hedley.h"

#if defined(__clang__) && __clang__

// HEDLEY_PRAGMA(clang diagnostic push);

#define CLANG_NON_NULL_BEGIN           \
                                       \
  HEDLEY_PRAGMA(clang diagnostic push) \
                                       \
  HEDLEY_PRAGMA(clang diagnostic ignored "-Wnullability-extension")

#define CLANG_NON_NULL_END HEDLEY_PRAGMA(clang diagnostic pop)

// #pragma clang diagnostic pop

#define CLANG_NON_NULL _Nonnull
#define CLANG_NULLABLE _Nullable

#else
#define CLANG_NON_NULL
#define CLANG_NULLABLE
#define CLANG_NON_NULL_BEGIN
#define CLANG_NON_NULL_END

#endif

// #pragma clang assume_nonnull begin
// #if defined(__clang__) && __clang__
// #pragma clang assume_nonnull end
// #endif

// NOTE: Some Hedley aliases for A E S T H E T I C S and
// conveinience

///
#define PURE_FUNC                                                              \
  /*The function has no side-effects, and the return value depends only on the \
    parameters and/or global variables.*/                                      \
  HEDLEY_PURE

#define CONST_FUNC                                                              \
  /* The function has no side-effects, and the return value depends only on the \
     parameters. Note that pointer arguments are not allowed.*/                 \
  HEDLEY_CONST

/// Pointer does not alias (no other pointer points to same region of memory)
/// aka this pointer is 'unique'
#define NO_ALIAS HEDLEY_RESTRICT

///  Tell the compiler to always inline the function, even if it thinks doing so
///  would be a bad idea.
#define FORCE_INLINE HEDLEY_ALWAYS_INLINE

/// Tell the compiler that the function will never throw a C++ exception
///    .Note that this can improve performance even in C mode.Use only if you're
///    sure your function will never call a function which throws a C++
///    exception, even indirectly.
#define NOTHROW HEDLEY_NO_THROW

/// Tell the compiler that the pointer will not escape the function call. For
/// more information, see the documentation for clang's noescape attribute.
/// Can be used on functions that free memory to help clang's static analysis
#define CLEANUP_FUNC HEDLEY_NO_ESCAPE

/// Emit a non-fatal compile-time informational message.
/// Compile Time Message
#define CMESSAGE HEDLEY_MESSAGE

/// Alias for CMESSAGE (HEDLEY_MESSAGE)
#define CINFO CMESSAGE

/// Emit a compile-time warning. If compiling with fatal warnings (e.g., -Wall
/// on GCC, or /WX on MSVC), these may be treated as fatal.
#define CWARNING HEDLEY_WARNING

/// Alias for CWARNING (HEDLEY_WARNING)
#define CWARN CWARNING

/// Compile-time precondition.
///
/// expr
///    Expression to check.
///
///
/// Example:
/// ----------------------------------------
///                                        |
/// int foo(int a, int b) WHERE(a < b);    |
///                                        |
/// ----------------------------------------
///
///
#define PRECONDITION HEDLEY_REQUIRE
#define WHERE PRECONDITION

///  Function attribute which signals that the compiler should emit a diagnostic
///  if the return value is discarded without being checked.
///
///  Must provide a reason why. Otherwise just use [[nodiscard]] since that is
///  supported since C++17
#define NODISCARD(reason) HEDLEY_WARN_UNUSED_RESULT_MSG

/// Same as NODISCARD, but with a default message telling the user that the
/// returned value owns a dynamic resource and must be cleaned up!
#define MUST_CLEANUP                                                           \
  NODISCARD(                                                                   \
      "Returned pointer or wrapping struct owns a dynamic resource that must " \
      "be cleaned up after caller is done using! DO NOT IGNORE OR SUFFER "     \
      "MEMORY LEAKS!!!!!!!!!")

///  Convert value to a string. value can be a preprocessor macro.
#define STRINGIFY HEDLEY_STRINGIFY

/// Concatenate two macros.
#define MACRO_CONCAT HEDLEY_CONCAT

/// List parameters which must never be NULL (for example, because they are
/// unconditionally dereferenced in the function body).
///
/// ...
///     Variadic list of indexes (starting from 1, left-to-right) of parameters
///     which must never be NULL.
///
/// Example:
/// ----------------------------------------------------------------
///                                                                |
/// HEDLEY_NON_NULL(1,3)                                           |
/// void do_something(Context* ctx_cant_be_null, Foo* maybe_null,  |
///                  Bar* cant_be_null);                           |
///                                                                |
/// ----------------------------------------------------------------
///
#define PARAMS_NONNULL HEDLEY_NON_NULL

/// Inform the compiler/analyzer that the code should never be reached (even
/// with invalid input).
#define UNREACHABLE HEDLEY_UNREACHABLE

///  Inform the compiler/analyzer that the code should never be reached or, for
///  compilers which don't provide a way to provide such information, return a
///  value.
#define UNREACHABLE_RETURN HEDLEY_UNREACHABLE_RETURN

/// The function will alwoys return a non-null value.
///
/// This may allow the compiler to skip some null-pointer checks.
#define RETURNS_NON_NULL HEDLEY_RETURNS_NON_NULL

///  Explicitly tell the compiler to fall through a case in the switch
///  statement. Without this, some compilers may think you accidentally omitted
///  a "break;" and emit a diagnostic.
#define FALLTHROUGH HEDLEY_FALL_THROUGH

#define METHOD                                                                                                        \
  /* alias for PARAMS_NONNULL(1), which ensures that the first parameter (self, for a method function), is always     \
   * non-null */                                                                                                      \
  /*Use this for functions intended to be used like methods for types, where those functions take a pointer to struct \
   * as thier first parameter */                                                                                      \
  PARAMS_NONNULL(1)

#define RECEIVER                                                                                                   \
  /* same as [METHOD_FN]. Use this for functions intended to be used like methods for types, where those functions \
   * take a pointer to struct as thier first parameter  */                                                         \
  METHOD
