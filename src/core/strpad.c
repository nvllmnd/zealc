#include "core/strpad.h"

#include "nv/core/log.h"
#include "nv/core/spad.h"
#include "nv/core/sslice.h"
#include "nv/core_types.h"
#include "nv/memory/alloc.h"
#include "nv/memory/virt.h"

static struct SPad {
  VirtMem* vm;
  StringPad sp;
  bool active;
} SPAD;

alias(SPad);

static inline bool isinit(const typeof(SPAD)* self) { return is_not_null(self->vm); }

void strpad_init(i32 size_bytes) {
  if LIKELY (is_null(SPAD.vm)) {
    if UNLIKELY (vmem_init(&SPAD.vm, size_bytes)) {
      LOG_FATAL("Failed to initialize backing VirtMem for Static StringPad Singleton!");
    }

    SPAD.sp = spad_new(SPAD.vm);
  }
}

i32 strpad_length(void) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
    return 0;
  }

  return SPAD.sp.size;
}

sslice strpad_fappend(const char* fmt, ...) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
  }

  va_list args = {};
  va_start(args);

  const sslice sl = strpad_vfappend(fmt, args);

  va_end(args);
  return sl;
}

sslice strpad_vfappend(const char* fmt, va_list args) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
  }
  return spad_vfappend(&SPAD.sp, fmt, args);
}

sslice strpad_nappend(const char* str, i32 len) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
  }

  return spad_nappend(&SPAD.sp, str, len);
}

sslice strpad_append(const char* str) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
  }

  return spad_append(&SPAD.sp, str);
}

void strpad_clear_zero(void) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
  }

  spad_clear_zeroed(&SPAD.sp);
}

void strpad_clear(void) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
  }

  spad_clear(&SPAD.sp);
}

void strpad_destroy(void) {
  if LIKELY (is_not_null(SPAD.vm)) {
    vmem_destroy(SPAD.vm);
    memset(&SPAD, 0, sizeof(typeof(SPAD)));
  }
}

char strpad_putchar(char c) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
  }

  return spad_putchar(&SPAD.sp, c);
}
void strpad_start(void) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);
  } else {
    spad_clear(&SPAD.sp);
  }

  SPAD.active = true;
}

sslice strpad_end(Allocator alloc) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);

    LOG_DBG("strpad_end called without previous call to strpad_star");
    return sslice_empty();
  }

  if UNLIKELY (!SPAD.active) {
    LOG_DBG("strpad_end called without previous call to strpad_star");
    return sslice_empty();
  }

  const sslice sl = spad_clone_string(&SPAD.sp, alloc);

  SPAD.active = false;

  strpad_clear();
  return sl;
}

void strpad_ends(char* buff, i32 buff_len) {
  if UNLIKELY (!isinit(&SPAD)) {
    strpad_init(STRPAD_DEFAULT_SIZE);

    LOG_DBG("strpad_ends called without previous call to strpad_star");
    return;
  }

  if UNLIKELY (!SPAD.active) {
    LOG_DBG("strpad_ends called without previous call to strpad_star");
    return;
  }

  spad_clone_into(&SPAD.sp, buff, buff_len);

  spad_clear(&SPAD.sp);
  SPAD.active = false;
}
