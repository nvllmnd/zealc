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

// ##########################################################
// ################## Temp Allocator ########################
// ##########################################################

/// @brief Default [VirtMemOpts] passed to either [talloc_init], [spad_init], or [tpad_init] if caller does not
/// call it themselves before invoking any other functions associated with that temp allocator
static constexpr const VirtMemOpts TALLOC_DEFAULT_OPTS =
    (VirtMemOpts){.size_in_mb = 1024 * 24, .initial_commit = KILOBYTES(4)};


/// @brief Number of calls to talloc_scope_begin/talloc_scope_end call pairs allowed
static constexpr const i32 TALLOC_MAX_SCOPE_DEPTH = 128;

/// @brief Allocates virtual memory for a static instance of a temporary Allocator
/// @warn Temp Allocator is NOT thread safe. only safe to call write from main thread
/// It is safe to skip calling this if you are okay with the temp allocator being initialized with [TALLOC_DEFAULT_OPTS]
/// @param(i32 vmem_size_mb) - Number of megabytes to initialize Temp Allocators backing [VirtMem]. 
void talloc_init(i32 vmem_size_mb);

/// @brief Releases virtual memory used by temporary alloctor back to the operating system.
void talloc_destroy(void);

/// resets used counter back to 0. This operation is done in O(1)
void talloc_clear(void);

RETURNS_NON_NULL
void* talloc_allocate(MemLayout layout);

RETURNS_NON_NULL
void* talloc_zallocate(MemLayout layout);

#define talloc_allocate_tp(T) ((__typeof(T)*)talloc_allocate(sizeof(__typeof(T))))
#define talloc_zallocate_tp(T) ((__typeof(T)*)talloc_zallocate(sizeof(__typeof(T))))

#define talloc_allocate_array(T, N) ((__typeof(T)*)talloc_allocate(mlayout_array(T, N)))
#define talloc_allocate_vec(T, _n) ((__typeof(T)*)talloc_allocate(mlayout_vec(T, (_n))))

#define talloc_zallocate_array(T, N) ((__typeof(T)*)talloc_zallocate(mlayout_array(T, N)))
#define talloc_zallocate_vec(T, _n) ((__typeof(T)*)talloc_zallocate(mlayout_vec(T, (_n))))

/// @brief Begins a temporary allocations scope
/// @details Pushes a new scope onto TempAlloc's inner scope stack. There should be a matching call to [talloc_scope_end] for every call to this function
/// These 2 functions can allow you to easily reuse sections of memory through scopes, instead of blindly clearing and reusing
/// @warning If this function is called more than [TALLOC_MAX_SCOPE_DEPTH] at a time, (without closing any scopes), [abort] is called and execution terminates
void talloc_scope_begin(void);


/// @brief Ends a temporary allocations scope
/// @details Pops scope from the back of TempAlloc's inner scope stack. There should be a matching call to [talloc_scope_begin] for every call to this function
/// These 2 functions can allow you to easily reuse sections of memory through scopes, instead of blindly clearing and reusing
void talloc_scope_end(void);

// ##########################################################
// ##############  String Builder (String Pad) ##############
// ##########################################################
void spad_init(VirtMemOpts opts);
void spad_destroy(void);

/// @brief Returns a slice view into this string pad's inner string buffer.
/// Returned [sslice] is not gauranteed to be null-terminated.
PURE_FUNC
sslice spad_string(void);

PURE_FUNC
i32 spad_string_len(void);

/// @brief copies buff_len bytes from SPad's inner string buffer into given memory buffer (buff_out)
/// @param (char* buff_out) - writes `buff_len` - 1 characters into memory at this address. appends null-character to
/// end of `buff_out`. Must not be null and at least `buff_len` bytes long.
/// @param (i32 buff_len) - number of bytes to write form Spad to `buff_out`
/// @returns Number of bytes written to `buff_out`. May be less than `buff_len` if Spad's inner string buffer is of
/// shorter length than `buff_len`
PARAMS_NONNULL(1)
i32 spad_clone_into(char* buff_out, i32 buff_len);

/// @brief duplicates currently built string using given allocator
/// @param(Allocator alloc) - Used to duplicate Spad's inner string buffer
/// @returns null-terminated string slice pointing to where the cloned string lives in given allocator. slice length
/// does not include null terminating character
sslice spad_clone_string(Allocator alloc);

sslice spad_nappend(const char* s, i32 len);
sslice spad_append(const char* s);

/// @brief Formats given format string and appends it to the back of the StringPad's inner buffer. Takes a format string
/// literal and printf-style variadic format value args
///
/// @param (const char* fmt) - printf-style format string literal
/// @param (char delim) - character delimiter to write to the end of appended formatted string.
///   Passing null character does NOT delimit strings with null character, instead if null character is passed for
///   delim, then this function acts as a way to append strings together, while always keeping a null character at the
///   end, otherwise strings are delimited with given delimiter character,
///
///  - ex:
///     spad_fappend_delim("%s", ',', "ayo"); // "ayo,"
///     spad_fappend_delim("%s", '\0', "yoo"); // "ayo,yoo"
///     spad_fappend_delim("%s", '!' " no null"); // "ayo,yoo nonull!"
HEDLEY_PRINTF_FORMAT(1, 3)
sslice spad_fappend_delim(const char* fmt, char delim, ...);

#define spad_fappend_nl(fmt, ...) (spad_fappend_delim(fmt, '\n' __VA_OPT__(, ) __VA_ARGS__))
#define spad_fappend_sp(fmt, ...) (spad_fappend_delim(fmt, ' ' __VA_OPT__(, ) __VA_ARGS__))

/// Same as [spad_fappend_delim], but passes '\0' as delim, making the delimiter effectively be an empty/no space!
HEDLEY_PRINTF_FORMAT(1, 2)
sslice spad_fappend(const char* fmt, ...);

/// Exactly the same as [spad_fappend_delim], but takes a va_list instead of
/// variadic arguments.
sslice spad_vfappend(const char* fmt, char delim, va_list args);

