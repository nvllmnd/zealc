#pragma once

#include "intdefs.h"
#define array(T, N)                                                    \
  /* conveinence for declaring static array of type (T) of size (N) */ \
  __typeof__(T[N])

#define ptr(T)                                                 \
  /* conveinence for declaring pointer types. bye, bye '*'! */ \
  __typeof__(__typeof_unqual__(T)*)

#define const_ptr(T)                                                           \
  /* same as [ptr] macro, is more clear that this pointer is a constant one */ \
  __typeof__(const __typeof_unqual__(T)*)

#define type_eq(a, b)                                                         \
  /* compares an expression (a) with given type (b) to see if their types are \
   * the same */                                                              \
  _Generic((a), __typeof__(b): true, default: false)

#define assert_type_eq(a, b)                                               \
  /* same as [type_eq] macro, but fails a static assertion if types do not \
   * match */                                                              \
  static_assert(type_eq(a, b))

#define is_string(s)                                                      \
  /* checks to see if s is of type [const char*] */                       \
  /* Do not that this will return false if given string is a mutable char \
   * buffer (char*) */                                                    \
  type_eq((s), char*)

#define cmp_min(a, b) ((a < b ? a : b))
#define cmp_max(a, b) ((a > b ? a : b))

#define cast(T, _src)                                                          \
  /* casts expression _src to be of type T */                                  \
  /* for a version of this macro that is specialized for casting pointers, see \
   * [pcast]*/                                                                 \
  ((__typeof__(T))(_src))

#define pcast(T, _ptr)                                                    \
  /* casts a given pointer (_ptr) to be of type T */                      \
  /* for a version of this macro that just does general casts between any \
   * given type and an expresion; see: [cast]*/                           \
  (cast(__typeof__(T*), (_ptr)))

#define is_null(p)                                    \
  /* checks if given pointer (p) is equal to null. */ \
  (nullptr == (p))

#define is_not_null(p)                                   \
  /* checks if given pointer (p) is not equal to null */ \
  (!(is_null((p))))

#define is_ptr_ok(p)                                                                                                   \
  /* check if given pointer (p) is good. (converts to a non-negative, non-zero integer) */                             \
  /* shorthand for [alloc_result] for checking pointers returned by [Allocator] interface struct [AllocVTable] methods \
   */                                                                                                                  \
  (((signed long long)(p)) > 0)

#define clamp(x, min, max)                                             \
  /* clamps a value values (x) to be between (min) and (max) */        \
  /* i.e.: 'clamp(-1, 0, 5) == 0;', or 'clamp(650, 0, 100) == 100;' */ \
  (cmp_max((min), cmp_min((x), (max))))

// NOTE: I kind of like these 2 macros in a guilty pleasure kind of way lmao...
//  i might one day use them, but idk its kinda ugg and seems too distant to C for
//  it to make any sense to other developers coming across it while reading
//  this codebase
//  #define deref(p) (*(p))
//  #define ref &
