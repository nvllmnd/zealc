#pragma once

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef int8_t i8;
typedef uint8_t byte;
typedef uint16_t u16;
typedef int16_t i16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

typedef size_t usize;
typedef ptrdiff_t isize;
/// AKA index :: same as `std::size_t` or `usize`
typedef usize index_t;
/// AKA: pointer offset :: same as `std::ptrdiff_t` or `isize`
typedef isize poffset_t;

typedef float f32;
typedef double f64;
typedef long double f128;
typedef typeof(void*) voidptr;
