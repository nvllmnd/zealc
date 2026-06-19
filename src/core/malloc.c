#include "core/malloc.h"

#include <stdlib.h>

#include "nv/core/algo.h"
#include "nv/memory/vmem.h"

static VArena Self = {};
static VArena Strings = {};

ZError memory_init(isize size) {
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

void* alloc(isize size, isize align) {
  void* ptr = va_allocate(&Self, (Layout){.size = size, .align = align});
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Cannot allocate object of size %li and alignment: %li from Global System Arena! Ran out of Memory!",
              size, align);
  }
  return ptr;
}

void* zalloc(isize size, isize align) {
  void* ptr = va_zallocate(&Self, (Layout){.size = size, .align = align});
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Cannot allocate object of size %li and alignment: %li from Global System Arena! Ran out of Memory!",
              size, align);
  }
  return ptr;
}

bool mresize(void* ptr, Layout old, Layout new) { return va_resize(&Self, ptr, old, new); }

void* reallocate(void* ptr, Layout old, Layout new) {
  void* res = va_reallocate(&Self, ptr, old, new);
  if UNLIKELY (is_null(res)) {
    LOG_FATAL(
        "Failed to reallocate object of size: %li and align: %li to size: %li and align: %li! va_reallocate returned "
        "nullptr!",
        old.size, old.align, new.size, new.align);
  }

  return res;
}

char* dupnstring(const char* str, isize len) {
  char* ptr = va_strndup(&Strings, str, len);
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Failed to duplicate string: %.*s of length: %li", (i32)len, str, len);
  }
  return ptr;
}

char* dupstring(const char* str) {
  char* ptr = va_strdup(&Strings, str);
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Failed to duplicate string: %s!", str);
  }
  return ptr;
}

char* fstring(isize* size_out, const char* fmt, ...) {
  va_list args = {};
  va_start(args);

  char* ptr = vfstring(size_out, fmt, args);

  va_end(args);
  return ptr;
}

char* vfstring(isize* size_out, const char* fmt, va_list args) {
  char* ptr = va_vfstring(&Strings, size_out, fmt, args);
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("Failed to format string: %s! only %li bytes available!", fmt, va_available(&Strings));
  }
  return ptr;
}

void memory_destroy(void) {
  if (!is_none(&Self)) {
    va_destroy(&Self);
    Self = zeroed(VArena);
  }
  if (!is_none(&Strings)) {
    va_destroy(&Strings);
    Strings = zeroed(VArena);
  }
}

void cfree(void* ptr) { free(ptr); }

void* cmalloc(isize size, isize align) {
  void* ptr = aligned_alloc(align, size);
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("libc allocator has ran out of memory! aligned_alloc(align: %li, size: %li) returned nullptr!", align,
              size);
  }
  return ptr;
}

void* ccalloc(isize size, isize count) {
  void* ptr = calloc(count, size);
  if UNLIKELY (is_null(ptr)) {
    LOG_FATAL("libc allocator has ran out of memory! calloc(count: %li, size: %li) returned nullptr!", count, size);
  }
  return ptr;
}

void* global_allocate(void*, Layout layout) { return alloc(layout.size, layout.align); }
void* global_zallocate(void*, Layout layout) { return zalloc(layout.size, layout.align); }
bool global_resize(void*, void* ptr, Layout old, Layout new) { return mresize(ptr, old, new); }
void* global_reallocate(void*, void* ptr, Layout old, Layout new) { return reallocate(ptr, old, new); }
void global_free(void*, void*) {}

void* libc_allocate(void*, Layout layout) { return aligned_alloc(layout.align, layout.size); }
void* libc_zallocate(void*, Layout layout) { return calloc(1, layout.size); }
bool libc_resize(void*, void*, Layout, Layout) { return false; }
void* libc_reallocate(void*, void* ptr, Layout, Layout new) { return realloc(ptr, new.size); }
void libc_free(void*, void* ptr) { free(ptr); }

static const AllocVTable LIBC_VT = {.allocate = libc_allocate,
                                    .zallocate = libc_zallocate,
                                    .resize = libc_resize,
                                    .reallocate = libc_reallocate,
                                    .free = libc_free,
                                    .mask = VT__All};

static const AllocVTable GLOBAL_VT = {.allocate = global_allocate,
                                      .zallocate = global_zallocate,
                                      .resize = global_resize,
                                      .reallocate = global_reallocate,
                                      .free = global_free,
                                      .mask = VT__All};

Allocator global_allocator(void) { return (Allocator){.ctx = nullptr, .vtable = &GLOBAL_VT}; }
Allocator libc_allocator(void) { return (Allocator){.ctx = nullptr, .vtable = &LIBC_VT}; }
