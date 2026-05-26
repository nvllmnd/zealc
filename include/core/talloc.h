#pragma once

#include "nv.h"
#include "nv/core/attributes.h"
#include "nv/core/constants.h"

// ##########################################################
// ################## Temp Allocator ########################
// ##########################################################

static constexpr const VirtMemOpts TALLOC_DEFAULT_OPTS =
    (VirtMemOpts){.size_in_mb = 1024 * 24, .initial_commit = KILOBYTES(4)};

/// @brief Allocates virtual memory for a static instance of a temporary Allocator
/// @warn Temp Allocator is NOT thread safe. only safe to call write from main thread
/// It is safe to skip calling this if you are okay with the temp allocator being initialized with [TALLOC_DEFAULT_OPTS]
/// @param() - [VirtMemOpts] Forwarded to initialize Temp Allocators backing [VirtMem]
void talloc_init(VirtMemOpts opts);

/// @brief Releases virtual memory used by temporary alloctor back to the operating system.
void talloc_destroy(void);

/// resets used counter back to 0. This operation is done in O(1)
void talloc_clear(void);

// void talloc_array_begin_(i32 elem_size);
// void talloc_array_end_(void);

// #define talloc_array_begin(T) (talloc_array_begin_(sizeof(T)))

// #define talloc_array_end() (talloc_array_end_())

// void talloc_string_begin(void);

// void* talloc_array_push_(MemLayout layout);

// #define talloc_array_push(v)                              \
//   ({                                                      \
//     void* _p = talloc_array_push_(sizeof(__typeof((v)))); \
//     memcpy(_p, &(v), sizeof(__typeof((v))))               \
//   })

// char talloc_putchar(char c);
// u8 talloc_putbyte(u8 b);

RETURNS_NON_NULL
void* talloc_allocate(MemLayout layout);

RETURNS_NON_NULL
void* talloc_zallocate(MemLayout layout);

// i32 talloc_array_len(void);

// PARAMS_NONNULL(1)
// const void* talloc_as_array_(i32* len_out);

// #define talloc_as_array(T, _len_out) ((__typeof(T)*)talloc_as_array((_len_out)))

#define talloc_allocate_tp(T) ((__typeof(T)*)talloc_allocate(mlayout_new(T)))

#define talloc_allocate_array(T, N) ((__typeof(T)*)talloc_allocate(mlayout_array(T, N)))

// bool talloc_is_init(void);

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
/// @returns null-terminated c-string pointing to duplicated  string in given allocator
const char* spad_dup_in(Allocator alloc);

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

// ##########################################################
// ##############  Array Builder (Type Pad) #################
// ##########################################################

void tpad_init(VirtMemOpts opts);
void tpad_destroy(void);
