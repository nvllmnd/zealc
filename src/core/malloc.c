#include "core/malloc.h"

#include "nv/memory/vmem.h"

static VArena Self = {};
static VArena Strings = {};

ZError arena_init(isize size) {
  if (is_zeroed(&Self)) {
    Self = va_new(size);
    assert(!is_zeroed(&Self));
  }
  if (is_zeroed(&Strings)) {
    Strings = va_new(size * 2);
    assert(!is_zeroed(&Strings));
  }
  return ZOK;
}

void* arena_allocate(isize size, isize align) {
  void* ptr = va_allocate(&Self, (Layout){.size = size, .align = align});
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Cannot allocate object of size %li and alignment: %li from Global System Arena! Ran out of Memory!",
              size, align);
  }
  return ptr;
}

void* arena_zallocate(isize size, isize align) {
  void* ptr = va_zallocate(&Self, (Layout){.size = size, .align = align});
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Cannot allocate object of size %li and alignment: %li from Global System Arena! Ran out of Memory!",
              size, align);
  }
  return ptr;
}

bool arena_resize(void* ptr, Layout old, Layout new) { return va_resize(&Self, ptr, old, new); }

void* arena_reallocate(void* ptr, Layout old, Layout new) {
  void* res = va_reallocate(&Self, ptr, old, new);
  if UNLIKELY (is_null(res)) {
    LOG_FATAL(
        "Failed to reallocate object of size: %li and align: %li to size: %li and align: %li! va_reallocate returned "
        "nullptr!",
        old.size, old.align, new.size, new.align);
  }

  return res;
}

char* arena_strndup(const char* str, isize len) {
  char* ptr = va_strndup(&Strings, str, len);
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Failed to duplicate string: %.*s of length: %li", (i32)len, str, len);
  }
  return ptr;
}

RETURNS_NON_NULL
char* arena_strdup(const char* str) {
  char* ptr = va_strdup(&Strings, str);
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Failed to duplicate string: %s!", str);
  }
  return ptr;
}

HEDLEY_PRINTF_FORMAT(2, 3)
char* arena_fstring(isize* size_out, const char* fmt, ...) {}

char* arena_vfstring(isize* size_out, const char* fmt, va_list args) RETURNS_NON_NULL;

void arena_destroy(void);

void cfree(void* ptr);

[[gnu::alloc_size(1)]] [[gnu::alloc_align(2)]]
void* cmalloc(isize size, isize align) RETURNS_NON_NULL;

[[gnu::alloc_size(1, 2)]] [[gnu::alloc_align(3)]]
void* ccalloc(isize size, isize count, isize align) RETURNS_NON_NULL;
