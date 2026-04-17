#pragma once
#include "intdefs.h"

#define SLICE_NAME(T) Slice_##T 

#define CONCAT(A, B) A##B

#define CONCAT3(A, B, C) A##B##C
#define SLICE_FN(T, _func_name) CONCAT3(Slice_, T##_, _func_name)

#define DEFINE_SLICE(T) typedef struct SLICE_NAME(T) SLICE_NAME(T); \
 struct SLICE_NAME(T) { \
   __typeof__(T*) start;\
   int len; \
 }; \
 SLICE_NAME(T) SLICE_FN(T, new)(__typeof__(T*) ptr, int len) { return (SLICE_NAME(T)){ .start = ptr, .len = len }; }


 #define DEFINE_SLICE_AS(T, U) \
 typedef T U; \
 DEFINE_SLICE(U)

 DEFINE_SLICE(u8)

 DEFINE_SLICE_AS(u8, Str)
 

 #define SLICE_NEW(T, ...) ({ (SLICE_NAME(T)) { __VA_ARGS__ }; })



