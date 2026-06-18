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

ZError arena_init(isize size);

[[gnu::alloc_size(1)]] [[gnu::alloc_align(2)]]
void* arena_allocate(isize size, isize align) RETURNS_NON_NULL;

[[gnu::alloc_size(1)]] [[gnu::alloc_align(2)]]
void* arena_zallocate(isize size, isize align) RETURNS_NON_NULL;

bool arena_resize(void* ptr, Layout old, Layout new) PARAMS_NONNULL(1);

void* arena_reallocate(void* ptr, Layout old, Layout new) PARAMS_NONNULL(1) RETURNS_NON_NULL;

[[gnu::alloc_size(2)]]
char* arena_strndup(const char* str, isize len) PARAMS_NONNULL(1) RETURNS_NON_NULL;

RETURNS_NON_NULL
char* arena_strdup(const char* str) PARAMS_NONNULL(1);

HEDLEY_PRINTF_FORMAT(2, 3)
char* arena_fstring(isize* size_out, const char* fmt, ...) PARAMS_NONNULL(2) RETURNS_NON_NULL;

char* arena_vfstring(isize* size_out, const char* fmt, va_list args) RETURNS_NON_NULL;

void arena_destroy(void);

void cfree(void* ptr);

[[gnu::alloc_size(1)]] [[gnu::alloc_align(2)]]
void* cmalloc(isize size, isize align) RETURNS_NON_NULL;

[[gnu::alloc_size(1, 2)]] [[gnu::alloc_align(3)]]
void* ccalloc(isize size, isize count, isize align) RETURNS_NON_NULL;
