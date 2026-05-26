#include "core/talloc.h"

#include <stdio.h>
#include <string.h>

#include "nv/core/algo.h"
#include "nv/core/sslice.h"
#include "nv/core_types.h"
#include "nv/memory/error.h"
#include "nv/memory/layout.h"
#include "nv/memory/virt.h"

static u8* SCOPE_PTRS[TALLOC_MAX_SCOPE_DEPTH] = {};
static i32 SCOPE_LEN = 0;

static VirtMem* TALLOC = {};
static VirtMem* SPAD = {};

void talloc_init(i32 vmem_size_mb) {
  assert(vmem_size_mb > 0);  
  if LIKELY (is_null(TALLOC)) {
    if UNLIKELY (vmem_init(&TALLOC, vmem_size_mb) != MemError__Ok) {
      log_fatal("Failed to initialize backing VirtMem for Temporary Allocator!");
    }
  } else {
    LOG("Temporary allocator has already been initialized to size: %liMB. If you wish to set it to have size: %dMB, call vmem_destroy first!", vmem_size(TALLOC)/1024/1024, vmem_size_mb);
  }
}

void talloc_destroy(void) {
  if LIKELY (is_not_null(TALLOC)) {
    if UNLIKELY (vmem_destroy(TALLOC) != MemError__Ok) {
      log_fatal("Failed to release Temp Allocator back to operating system! aborting execution!");
    }
    TALLOC = nullptr;
  }
}

void talloc_clear(void) {
  assert(TALLOC);
  vmem_clear(TALLOC);
  SCOPE_LEN = 0;
}

RETURNS_NON_NULL
void* talloc_allocate(MemLayout layout) {
  assert(TALLOC);
  // abort if allocated pointer is null, this simplifies us satisfying the __returns_nonnull__ compiler attribute 
  return punwrap(vmem_allocate(TALLOC,  layout));
}

RETURNS_NON_NULL
void* talloc_zallocate(MemLayout layout) {
  assert(TALLOC);

  // abort if allocated pointer is null, this simplifies us satisfying the __returns_nonnull__ compiler attribute 
  return punwrap(vmem_zallocate(TALLOC,  layout));  
}

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


/// Same as [spad_fappend_delim], but passes '\0' as delim, making the delimiter effectively be an empty/no space!
HEDLEY_PRINTF_FORMAT(1, 2)
sslice spad_fappend(const char* fmt, ...);

/// Exactly the same as [spad_fappend_delim], but takes a va_list instead of
/// variadic arguments.
sslice spad_vfappend(const char* fmt, char delim, va_list args);



// typedef enum AllocMode {
//   AllocMode__Destroyed = -1,
//   AllocMode__Uninit = 0,
//   AllocMode__Default,
//   AllocMode__StringBuilder,
//   AllocMode__ArrayBuilder,
// } AllocMode;

// struct TempAllocator {
//   VirtMem* mem;
//   AllocMode mode;

//   /// used for when mode == AllocMode__ArrayBuilder
//   i32 elem_size;
//   /// used for when mode == AllocMode__ArrayBuilder
//   i32 elem_count;
// };
// alias(TempAllocator);

// static TempAllocator TALLOC = {};

// MemError talloc_init(VirtMemOpts opts) {
//   if (is_null(TALLOC.mem)) {
//     tryerr(vmem_init(&TALLOC.mem, opts.size_in_mb));
//     TALLOC.mode = AllocMode__Default;
//   } else {
//     LOG("Attempted to initialize Temporary Allocator after it has already been initialized! if this was intentional "
//         "call talloc_destroy before calling talloc_init again, or just call talloc_clear/talloc_clear_zeroed!");
//   }

//   return MemError__Ok;
// }

// void talloc_destroy(void) {
//   if (is_not_null(TALLOC.mem)) {
//     vmem_destroy(TALLOC.mem);
//     TALLOC.mem = nullptr;
//     TALLOC.mode = AllocMode__Destroyed;
//   } else {
//     LOG("Attempted to destroy Temporary Allocator after it has already been destroyed! This is safe, but probably a "
//         "logic error!");
//   }
// }

// void talloc_clear(void) {
//   assert(TALLOC.mem);
//   vmem_clear(TALLOC.mem);
// }

// void talloc_clear_zeroed(void) {
//   assert(TALLOC.mem);
//   vmem_clear_zeroed(TALLOC.mem);
// }

// sslice talloc_as_string(void) {
//   assert(TALLOC.mem);
//   const VirtMemView view = vmem_view(TALLOC.mem);
//   return sslice_new(.begin = view.start, .len = view.used_bytes);
// }

// const void* talloc_begin(void) {
//   assert(TALLOC.mem);
//   const VirtMemView view = vmem_view(TALLOC.mem);
//   return view.start;
// }
// const void* talloc_end(void) {
//   assert(TALLOC.mem);
//   const VirtMemView view = vmem_view(TALLOC.mem);
//   return view.end;
// }

// sslice talloc_nappend(const char* s, i32 len) {
//   assert(TALLOC.mem);
//   assert(TALLOC.mode == AllocMode__StringBuilder);
//   assert(s);
//   assert(len > 0);

//   assert(len <= vmem_available(TALLOC.mem));
//   char* str = vmem_allocate(TALLOC.mem, mlayout_bytes(len));
//   if UNLIKELY (is_null(str)) {
//     LOG("Temporary Allocator's backing virtual memory is too full to append: %.*s!", len, s);
//     return sslice_empty();
//   }
//   strncpy(str, s, len);
//   return sslice_new(.begin = str, .len = len);
// }

// sslice talloc_append(const char* s) {
//   assert(s);

//   assert(TALLOC.mode == AllocMode__StringBuilder);
//   const i32 len = stringlen(s);
//   assert(len < STRLEN_UPPER_BOUND);

//   return talloc_nappend(s, len);
// }

// void talloc_string_begin(void) {
//   assert(TALLOC.mem);
//   TALLOC.mode = AllocMode__StringBuilder;
//   TALLOC.elem_size = 1;
//   TALLOC.elem_count = 0;
// }

// void* talloc_allocate(MemLayout layout) {
//   assert(TALLOC.mem);

//   void* mem = vmem_allocate(TALLOC.mem, layout);
//   if UNLIKELY (is_null(mem)) {
//     LOG("Temporary Allocator's backing virtual memory is too full to append object of size: %d!", layout.size);
//     return nullptr;
//   }
//   return mem;
// }

// void* talloc_zallocate(MemLayout layout) {
//   assert(TALLOC.mem);

//   void* mem = vmem_zallocate(TALLOC.mem, layout);
//   if (is_null(mem)) {
//     LOG("Temporary Allocator's backing virtual memory is too full to append object of size: %d has only %li bytes "
//         "available!!",
//         layout.size, vmem_available(TALLOC.mem));
//     return nullptr;
//   }
//   return mem;
// }

// sslice talloc_fappend_delim(const char* fmt, char delim, ...) {
//   assert(TALLOC.mem);
//   assert(fmt);

//   va_list args;
//   va_start(args);

//   const sslice sl = spad_vfappend(fmt, delim, args);
//   va_end(args);

//   return sl;
// }

// sslice talloc_fspush_nl(const char* fmt, ...) {
//   assert(TALLOC.mem);
//   assert(fmt);

//   va_list args;
//   va_start(args);

//   const sslice sl = spad_vfappend(fmt, '\n', args);
//   va_end(args);

//   return sl;
// }

// sslice spad_fappend(const char* fmt, ...) {
//   assert(TALLOC.mem);
//   assert(fmt);

//   va_list args;
//   va_start(args);

//   const sslice sl = spad_vfappend(fmt, '\0', args);
//   va_end(args);

//   return sl;
// }

// sslice spad_vfappend(const char* fmt, char delim, va_list args) {
//   assert(TALLOC.mem);
//   assert(fmt);

//   assert(TALLOC.mode == AllocMode__StringBuilder);

//   va_list len_args = {};
//   va_copy(len_args, args);

//   const i32 avail = vmem_available(TALLOC.mem) - 1;
//   i32 len = vsnprintf(nullptr, 0, fmt, len_args) + 1;
//   assert(len > 0);
//   if (len > avail) {
//     len = avail;
//   }

//   va_end(len_args);

//   // check if delim is a null character, if its not then we can append the full length, as the null character that
//   // vsnprintf appends to the end of the formatted will be overwritten by our delim character
//   char* str = vmem_allocate(TALLOC.mem, mlayout_bytes(delim == '\0' ? len - 1 : len));
//   if UNLIKELY (is_null(str)) {
//     LOG("Temporary Allocator's backing virtual memory is too full to append format string: %s! has only %li bytes "
//         "available!",
//         fmt, vmem_available(TALLOC.mem));
//     return sslice_empty();
//   }

//   vsnprintf(str, len, fmt, args);

//   if (delim != '\0') {
//     str[len] = delim;
//   } else {
//     WARN(
//         "pushing a formatted string with terminator character set to a null character. If this is intentional, you can "
//         "ignore this message. Otherwise this null character may make the string being built appear to be shorter than "
//         "it actually is!");
//   }

//   return sslice_new(.begin = str, .len = len);
// }

// void talloc_string_end(void) {
//   talloc_putchar('\0');
//   TALLOC.mode = AllocMode__Default;
// }

// char talloc_putchar(char c) {
//   assert(TALLOC.mem);

//   assert(TALLOC.mode == AllocMode__StringBuilder);

//   char* ch = vmem_allocate(TALLOC.mem, mlayout_bytes(1));

//   if UNLIKELY (is_null(ch)) {
//     LOG("Temporary Allocator's backing virtual memory is too full to append character: %c", c);
//     return INT8_MIN;
//   }

//   *ch = c;
//   return c;
// }

// u8 talloc_putbyte(u8 b) {
//   assert(TALLOC.mem);

//   u8* by = vmem_allocate(TALLOC.mem, mlayout_bytes(1));

//   if UNLIKELY (is_null(by)) {
//     LOG("Temporary Allocator's backing virtual memory is too full to append byte: %c", b);
//     return UINT8_MAX;
//   }

//   *by = b;
//   return b;
// }
// void talloc_array_begin_(i32 elem_size) {
//   assert(TALLOC.mem);
//   TALLOC.mode = AllocMode__ArrayBuilder;
//   TALLOC.elem_size = elem_size;
//   TALLOC.elem_count = 0;
// }
// void talloc_array_end_() {
//   assert(TALLOC.mem);
//   TALLOC.mode = AllocMode__Default;
// }

// const void* talloc_as_array_(i32* len_out) {
//   assert(TALLOC.mem);
//   *len_out = TALLOC.elem_count;
//   const VirtMemView view = vmem_view(TALLOC.mem);
//   return view.start;
// }

// void* tpad_push_(MemLayout layout) {
//   assert(TALLOC.mem);

//   assert(TALLOC.mode == AllocMode__ArrayBuilder);
//   assert(layout.size == TALLOC.elem_size);
//   void* mem = vmem_allocate(TALLOC.mem, layout);
//   if UNLIKELY (is_null(mem)) {
//     LOG("Temporary Allocator's backing virtual memory is too full to append array element of size: %d", layout.size);
//     return nullptr;
//   }

//   TALLOC.elem_count += 1;
//   return mem;
// }

// bool talloc_is_init(void) { return is_not_null(TALLOC.mem) && TALLOC.mode > AllocMode__Uninit; }

// i32 talloc_array_len(void) {
//   assert(TALLOC.mem);
//   assert(TALLOC.mode == AllocMode__ArrayBuilder);
//   return TALLOC.elem_count;
// }

// sslice spad_fspush_sp(const char* fmt, ...) {
//   assert(TALLOC.mem);
//   assert(fmt);

//   va_list args;
//   va_start(args);

//   const sslice sl = spad_vfappend(fmt, ' ', args);
//   va_end(args);

//   return sl;
// }
