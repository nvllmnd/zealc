//!
//! This header contains functions for interacting with a few static instances of temporary allocators,
//! each used for a different purpose. The String and Array pads are great for building arrays and strings with dynamic
//! length at runtime, before copying them to a permanent location, for instance an [Arena]. in contrast to this,
//! pushing elements into a vec allocated by an [Arena] beyond their capacity, requires copying the entire vec into a
//! bigger allocation, still allocated by [Arena], causing spurrious copies that leak memory, the [Arena]
//! allocator handles reallocation by leaking the old contents, as it has no mechanism to free memory beyond resetting
//! the entire [Arena]
//!
//! Temp allocators used:
//!     A) TempAllocator (talloc_*) - Used for general temporary allocations, frequently cleared, pointered returned by
//!     this allocator should not live for very long each generation, also contains functions to scope allocations
//!
//!     B) StringPad (spad_*) - Used for building strings of dynamic length, has functions to format strings in a printf
//!     style, as well as copy the built string into a buffer or [Allocator].
#pragma once

#include "nv.h"
#include "nv/memory/alloc.h"

// ##########################################################
// ################## Temp Allocator ########################
// ##########################################################
//
//

/// @brief Default size of strpad if [strpad_init] is not called before any other strpad_* call
static constexpr const i32 STRPAD_DEFAULT_SIZE = GIGABYTES(1);

// /// @brief Default [VirtMemOpts] passed to either [talloc_init], [spad_init], or [tpad_init] if caller does not
// /// call it themselves before invoking any other functions associated with that temp allocator
// static constexpr const VirtMemOpts TALLOC_DEFAULT_OPTS =
//     (VirtMemOpts){.size_in_mb = 1024 * 24, .initial_commit = KILOBYTES(4)};
//
void strpad_init(i32 size_bytes);

HEDLEY_PRINTF_FORMAT(1, 2)
PARAMS_NONNULL(1)
sslice strpad_fappend(const char* fmt, ...);

PARAMS_NONNULL(1)
sslice strpad_vfappend(const char* fmt, va_list args);

PARAMS_NONNULL(1)
sslice strpad_nappend(const char* str, i32 len);

PARAMS_NONNULL(1)
sslice strpad_append(const char* str);

void strpad_clear_zero(void);
void strpad_clear(void);

char strpad_putchar(char c);

PURE_FUNC
i32 strpad_length(void);

void strpad_start(void);

sslice strpad_end(Allocator alloc);
void strpad_ends(char* buff, i32 buff_len);

void strpad_destroy(void);
