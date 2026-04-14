#pragma once

#include "intdefs.h"

#define array(T, N) __typeof__(T[N])
#define ptr(T) __typeof__(__typeof_unqual__(T)*)
#define const_ptr(T) __typeof__(const __typeof_unqual__(T)*)

#define type_eq(a, b) \
  _Generic((a), __typeof__(b): true, default: false)


#define assert_type_eq(a, b) static_assert(type_eq(a, b))


#define cmp_max(a, b) ({ \
  __typeof__(a) _a = (a); \
  __typeof__(b) _b = (b); \
  _a > _b ? _a : _b; \
 }) \

#define cmp_min(a, b) ({ \
  __typeof__(a) _a = (a); \
  __typeof__(b) _b = (b); \
  _a < _b ? _a : _b; \
}) \

#define cast(T, _src) ((__typeof__(T))(_src))

#define pcast(T, _ptr) (cast(__typeof__(T*), (_ptr)))






