//!
//! Arena for general, long-lived allocations. and an arena for long-lived strings
//! In general these Arenas are for Long-term storage, but you can use markers to free scoped sections,
//! or free all memory with [arena_clear], or free just the string arena with [arena_clear_strings], or just general
//! arena with [arena_clear_buffer]
//!
#pragma once

#include "error.h"
#include "nv/core/attributes.h"
#include "nv/core/intdefs.h"
#include "nv/memory/alloc.h"

ZError memory_init(isize size);

[[gnu::alloc_size(1)]] [[gnu::alloc_align(2)]]
void* alloc(isize size, isize align) RETURNS_NON_NULL;

[[gnu::alloc_size(1)]] [[gnu::alloc_align(2)]]
void* zalloc(isize size, isize align) RETURNS_NON_NULL;

bool mresize(void* ptr, Layout old, Layout new) PARAMS_NONNULL(1);

void* reallocate(void* ptr, Layout old, Layout new) PARAMS_NONNULL(1) RETURNS_NON_NULL;

[[gnu::alloc_size(2)]]
char* dupnstring(const char* str, isize len) PARAMS_NONNULL(1) RETURNS_NON_NULL;

RETURNS_NON_NULL
char* dupstring(const char* str) PARAMS_NONNULL(1);

HEDLEY_PRINTF_FORMAT(2, 3)
char* fstring(isize* size_out, const char* fmt, ...) PARAMS_NONNULL(2) RETURNS_NON_NULL;

char* vfstring(isize* size_out, const char* fmt, va_list args) RETURNS_NON_NULL;

Allocator global_allocator(void);

void memory_destroy(void);

#ifndef ZEAL_OVERRIDE_MALLOC
#define ZEAL_OVERRIDE_MALLOC 1
#endif

// #if ZEAL_OVERRIDE_MALLOC == 1
//
// #define aligned_alloc(_align, _size) (alloc((_size), (_align)))
// #define malloc(_size) (alloc((_size), 1))
// #define calloc(_size, _count) (zalloc((_size) * (_count)))
// #define realloc(_ptr, _new_size) (reallocate((_ptr), mlayout_bytes(1), mlayout_bytes((_new_size))))
// #define free(_ptr)
//
// #endif

void cfree(void* ptr);

[[gnu::alloc_size(1)]] [[gnu::alloc_align(2)]]
void* cmalloc(isize size, isize align) RETURNS_NON_NULL;

[[gnu::alloc_size(1, 2)]]
void* ccalloc(isize size, isize count) RETURNS_NON_NULL;

PURE_FUNC
Allocator libc_allocator(void);

#define new(T) ((typeof(T)*)alloc(sizeof(T), alignof(T)))
#define array_new(T, _n) ((typeof(T)*)alloc(sizeof(T) * (_n), alignof(T)))

#define alloc_vec(T, _cap) (vec_new(T, (_cap), global_allocator()))
#define alloc_dvec(T, _cap) (dvec_new(T, (_cap), global_allocator()))
