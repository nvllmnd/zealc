#include "core/talloc.h"

#include <stdio.h>
#include <string.h>

#include "nv/core/algo.h"
#include "nv/core/sslice.h"
#include "nv/core_types.h"
#include "nv/memory/error.h"
#include "nv/memory/layout.h"
#include "nv/memory/virt.h"

typedef enum AllocMode {
  AllocMode__Destroyed = -1,
  AllocMode__Uninit = 0,
  AllocMode__Default,
  AllocMode__StringBuilder,
  AllocMode__ArrayBuilder,
} AllocMode;

struct TempAllocator {
  VirtMem* mem;
  AllocMode mode;

  /// used for when mode == AllocMode__ArrayBuilder
  i32 elem_size;
  /// used for when mode == AllocMode__ArrayBuilder
  i32 elem_count;
};
alias(TempAllocator);

static TempAllocator TALLOC = {};

MemError talloc_init(VirtMemOpts opts) {
  if (is_null(TALLOC.mem)) {
    tryerr(vmem_init(&TALLOC.mem, opts.size_in_mb));
    TALLOC.mode = AllocMode__Default;
  } else {
    LOG("Attempted to initialize Temporary Allocator after it has already been initialized! if this was intentional "
        "call talloc_destroy before calling talloc_init again, or just call talloc_clear/talloc_clear_zeroed!");
  }

  return MemError__Ok;
}

void talloc_destroy(void) {
  if (is_not_null(TALLOC.mem)) {
    vmem_destroy(TALLOC.mem);
    TALLOC.mem = nullptr;
    TALLOC.mode = AllocMode__Destroyed;
  } else {
    LOG("Attempted to destroy Temporary Allocator after it has already been destroyed! This is safe, but probably a "
        "logic error!");
  }
}

void talloc_clear(void) {
  assert(TALLOC.mem);
  vmem_clear(TALLOC.mem);
}

void talloc_clear_zeroed(void) {
  assert(TALLOC.mem);
  vmem_clear_zeroed(TALLOC.mem);
}

sslice talloc_as_string(void) {
  assert(TALLOC.mem);
  const VirtMemView view = vmem_view(TALLOC.mem);
  return sslice_new(.begin = view.start, .len = view.used_bytes);
}

const void* talloc_begin(void) {
  assert(TALLOC.mem);
  const VirtMemView view = vmem_view(TALLOC.mem);
  return view.start;
}
const void* talloc_end(void) {
  assert(TALLOC.mem);
  const VirtMemView view = vmem_view(TALLOC.mem);
  return view.end;
}

sslice talloc_nappend(const char* s, i32 len) {
  assert(TALLOC.mem);
  assert(TALLOC.mode == AllocMode__StringBuilder);
  assert(s);
  assert(len > 0);

  assert(len <= vmem_available(TALLOC.mem));
  char* str = vmem_allocate(TALLOC.mem, mlayout_bytes(len));
  if UNLIKELY (is_null(str)) {
    LOG("Temporary Allocator's backing virtual memory is too full to append: %.*s!", len, s);
    return sslice_empty();
  }
  strncpy(str, s, len);
  return sslice_new(.begin = str, .len = len);
}

sslice talloc_append(const char* s) {
  assert(s);

  assert(TALLOC.mode == AllocMode__StringBuilder);
  const i32 len = stringlen(s);
  assert(len < STRLEN_UPPER_BOUND);

  return talloc_nappend(s, len);
}

void talloc_string_begin(void) {
  assert(TALLOC.mem);
  TALLOC.mode = AllocMode__StringBuilder;
  TALLOC.elem_size = 1;
  TALLOC.elem_count = 0;
}

void* talloc_allocate(MemLayout layout) {
  assert(TALLOC.mem);

  void* mem = vmem_allocate(TALLOC.mem, layout);
  if UNLIKELY (is_null(mem)) {
    LOG("Temporary Allocator's backing virtual memory is too full to append object of size: %d!", layout.size);
    return nullptr;
  }
  return mem;
}

void* talloc_zallocate(MemLayout layout) {
  assert(TALLOC.mem);

  void* mem = vmem_zallocate(TALLOC.mem, layout);
  if (is_null(mem)) {
    LOG("Temporary Allocator's backing virtual memory is too full to append object of size: %d has only %li bytes "
        "available!!",
        layout.size, vmem_available(TALLOC.mem));
    return nullptr;
  }
  return mem;
}

sslice talloc_fappend_delim(const char* fmt, char delim, ...) {
  assert(TALLOC.mem);
  assert(fmt);

  va_list args;
  va_start(args);

  const sslice sl = spad_vfappend(fmt, delim, args);
  va_end(args);

  return sl;
}

sslice talloc_fspush_nl(const char* fmt, ...) {
  assert(TALLOC.mem);
  assert(fmt);

  va_list args;
  va_start(args);

  const sslice sl = spad_vfappend(fmt, '\n', args);
  va_end(args);

  return sl;
}

sslice spad_fappend(const char* fmt, ...) {
  assert(TALLOC.mem);
  assert(fmt);

  va_list args;
  va_start(args);

  const sslice sl = spad_vfappend(fmt, '\0', args);
  va_end(args);

  return sl;
}

sslice spad_vfappend(const char* fmt, char delim, va_list args) {
  assert(TALLOC.mem);
  assert(fmt);

  assert(TALLOC.mode == AllocMode__StringBuilder);

  va_list len_args = {};
  va_copy(len_args, args);

  const i32 avail = vmem_available(TALLOC.mem) - 1;
  i32 len = vsnprintf(nullptr, 0, fmt, len_args) + 1;
  assert(len > 0);
  if (len > avail) {
    len = avail;
  }

  va_end(len_args);

  // check if delim is a null character, if its not then we can append the full length, as the null character that
  // vsnprintf appends to the end of the formatted will be overwritten by our delim character
  char* str = vmem_allocate(TALLOC.mem, mlayout_bytes(delim == '\0' ? len - 1 : len));
  if UNLIKELY (is_null(str)) {
    LOG("Temporary Allocator's backing virtual memory is too full to append format string: %s! has only %li bytes "
        "available!",
        fmt, vmem_available(TALLOC.mem));
    return sslice_empty();
  }

  vsnprintf(str, len, fmt, args);

  if (delim != '\0') {
    str[len] = delim;
  } else {
    WARN(
        "pushing a formatted string with terminator character set to a null character. If this is intentional, you can "
        "ignore this message. Otherwise this null character may make the string being built appear to be shorter than "
        "it actually is!");
  }

  return sslice_new(.begin = str, .len = len);
}

void talloc_string_end(void) {
  talloc_putchar('\0');
  TALLOC.mode = AllocMode__Default;
}

char talloc_putchar(char c) {
  assert(TALLOC.mem);

  assert(TALLOC.mode == AllocMode__StringBuilder);

  char* ch = vmem_allocate(TALLOC.mem, mlayout_bytes(1));

  if UNLIKELY (is_null(ch)) {
    LOG("Temporary Allocator's backing virtual memory is too full to append character: %c", c);
    return INT8_MIN;
  }

  *ch = c;
  return c;
}

u8 talloc_putbyte(u8 b) {
  assert(TALLOC.mem);

  u8* by = vmem_allocate(TALLOC.mem, mlayout_bytes(1));

  if UNLIKELY (is_null(by)) {
    LOG("Temporary Allocator's backing virtual memory is too full to append byte: %c", b);
    return UINT8_MAX;
  }

  *by = b;
  return b;
}
void talloc_array_begin_(i32 elem_size) {
  assert(TALLOC.mem);
  TALLOC.mode = AllocMode__ArrayBuilder;
  TALLOC.elem_size = elem_size;
  TALLOC.elem_count = 0;
}
void talloc_array_end_() {
  assert(TALLOC.mem);
  TALLOC.mode = AllocMode__Default;
}

const void* talloc_as_array_(i32* len_out) {
  assert(TALLOC.mem);
  *len_out = TALLOC.elem_count;
  const VirtMemView view = vmem_view(TALLOC.mem);
  return view.start;
}

void* talloc_array_push_(MemLayout layout) {
  assert(TALLOC.mem);

  assert(TALLOC.mode == AllocMode__ArrayBuilder);
  assert(layout.size == TALLOC.elem_size);
  void* mem = vmem_allocate(TALLOC.mem, layout);
  if UNLIKELY (is_null(mem)) {
    LOG("Temporary Allocator's backing virtual memory is too full to append array element of size: %d", layout.size);
    return nullptr;
  }

  TALLOC.elem_count += 1;
  return mem;
}

bool talloc_is_init(void) { return is_not_null(TALLOC.mem) && TALLOC.mode > AllocMode__Uninit; }

i32 talloc_array_len(void) {
  assert(TALLOC.mem);
  assert(TALLOC.mode == AllocMode__ArrayBuilder);
  return TALLOC.elem_count;
}

sslice talloc_fspush_sp(const char* fmt, ...) {
  assert(TALLOC.mem);
  assert(fmt);

  va_list args;
  va_start(args);

  const sslice sl = spad_vfappend(fmt, ' ', args);
  va_end(args);

  return sl;
}
