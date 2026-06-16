#include "core/strpad.h"

#include "nv/core/algo.h"
#include "nv/core/log.h"
#include "nv/core/spad.h"
#include "nv/core/sslice.h"
#include "nv/memory/alloc.h"
#include "nv/memory/virt.h"

struct Strpad {
  VMem* vm;
  StringPad pad;
};
alias(Strpad);

static Strpad SPAD = {};

alias(SPad);

static inline bool isinit(const Strpad* self) { return is_not_null(self->vm); }

#define check_init                    \
  if UNLIKELY (!isinit(&SPAD)) {      \
    strpad_init(STRPAD_DEFAULT_SIZE); \
  }

#define check_init_bail(_retval)      \
  if UNLIKELY (!isinit(&SPAD)) {      \
    strpad_init(STRPAD_DEFAULT_SIZE); \
    return (_retval);                 \
  }

#define check_init_orelse(_else)      \
  if UNLIKELY (!isinit(&SPAD)) {      \
    strpad_init(STRPAD_DEFAULT_SIZE); \
  } else {                            \
    (_else);                          \
  }
#define check_init_return             \
  if UNLIKELY (!isinit(&SPAD)) {      \
    strpad_init(STRPAD_DEFAULT_SIZE); \
    return;                           \
  }
void strpad_init(const i64 size_bytes) {
  if LIKELY (is_null(SPAD.vm)) {
    VMem* vm = vmem_new(size_bytes);
    if UNLIKELY (is_null(vm)) {
      LOG_FATAL("Failed to initialize static instance of StringPad!");
    }

    auto begin = (char*)vmem_begin(SPAD.vm);
    auto end = (char*)vmem_end(SPAD.vm);

    SPAD = (Strpad){.vm = vm, .pad = spad_new(begin, end)};

    // spad_build_start(&SPAD.pad);
  }
}

i32 strpad_length(void) {
  check_init_bail(0);

  return spad_length(&SPAD.pad);
}

sslice strpad_fappend(const char* fmt, ...) {
  va_list args = {};
  va_start(args);

  const sslice sl = strpad_vfappend(fmt, args);

  va_end(args);
  return sl;
}

sslice strpad_vfappend(const char* fmt, va_list args) {
  check_init;

  return spad_vfappend(&SPAD.pad, fmt, args);
}

sslice strpad_nappend(const char* str, i32 len) {
  check_init;

  return spad_nappend(&SPAD.pad, str, len);
}

sslice strpad_append(const char* str) {
  check_init;

  return spad_append(&SPAD.pad, str);
}

void strpad_clear_zero(void) {
  check_init_return;

  spad_clear_zeroed(&SPAD.pad);
}

void strpad_clear(void) {
  check_init_return;

  spad_clear(&SPAD.pad);
}

void strpad_destroy(void) {
  if LIKELY (is_not_null(SPAD.vm)) {
    vmem_destroy(SPAD.vm);
    memset(&SPAD, 0, sizeof(Strpad));
  }
}

char strpad_putchar(char c) {
  check_init;

  return spad_putchar(&SPAD.pad, c);
}
void strpad_start(void) {
  check_init_orelse(spad_clear(&SPAD.pad));

  spad_build_start(&SPAD.pad);
}

sslice strpad_end(Allocator alloc) {
  check_init_bail(sslice_empty());

  return spad_build_end(&SPAD.pad, alloc);
}

i64 strpad_ends(char* buff, i32 buff_len) {
  if UNLIKELY (!isinit(&SPAD) || !SPAD.pad.inuse) {
    strpad_init(STRPAD_DEFAULT_SIZE);

    LOG_DBG("strpad_ends called without previous call to strpad_star");
    return -1;
  }

  return spad_build_end_into(&SPAD.pad, buff, buff_len);
}
