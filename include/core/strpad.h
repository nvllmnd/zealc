//! Wrapper for our static state around libnv's [StringPad]
#pragma once

// #include "nv/nv.h"

#include "nv/common.h"
#include "nv/memory/alloc.h"

/// @brief Default size of strpad if [strpad_init] is not called before any other strpad_* call
static constexpr const i32 STRPAD_DEFAULT_SIZE = GIGABYTES(1);

// /// @brief Default [VirtMemOpts] passed to either [talloc_init], [spad_init], or [tpad_init] if caller does not
// /// call it themselves before invoking any other functions associated with that temp allocator
// static constexpr const VirtMemOpts TALLOC_DEFAULT_OPTS =
//     (VirtMemOpts){.size_in_mb = 1024 * 24, .initial_commit = KILOBYTES(4)};
//
void strpad_init(i64 size_bytes);

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
i64 strpad_ends(char* buff, i32 buff_len);

void strpad_destroy(void);
