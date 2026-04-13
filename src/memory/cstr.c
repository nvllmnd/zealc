#include "memory/cstr.h"
#include "mimalloc.h"
#include "mimalloc/internal.h"
#include <assert.h>
#include <string.h>

static void init_small_cstr(cstr *self, const char *string, usize len) {
  assert(len <= SMALL_BUF_SIZE);
  self->buf.len = cast(u8, len);
  strncpy(self->buf.mem, string, len);
  self->buf.mem[SMALL_BUF_SIZE] = 0;
}


static u8* alloc_prefix_memory(usize size) {

  const usize  alloc_size  = size + sizeof(i32) + 1;  


  u8* s = mi_calloc(sizeof(u8), alloc_size);
  assert(nullptr != s);

  return s;

}
static prefix_str prefix_str_new(const char* string, usize len) {


  u8* s = alloc_prefix_memory(len);


  i32* prefix_ptr = pcast(i32, s);

  *prefix_ptr = cast(i32, len);

  prefix_str string_begin = pcast(char, prefix_ptr + 1);
  strncpy(string_begin, string, len);
  return string_begin;

}


static void init_large_cstr(cstr* self, const char* string, usize len) {
  assert(len > SMALL_BUF_SIZE);

  self->is_large = true;

  prefix_str string_begin = prefix_str_new(string, len);
  
  self->heap = string_begin;
}
cstr cstr_new(const char *string) {
  const usize len = strlen(string);
  return cstr_from_slice(string, len);
}

cstr cstr_large_with_capacity(usize capacity) {
  u8* mem = alloc_prefix_memory(capacity);

  i32* prefix_ptr = pcast(i32, mem);
  *prefix_ptr = cast(i32, capacity);

  
  prefix_str str_begin = pcast(char, prefix_ptr + 1);

  return (cstr){ .is_large = true, .heap = str_begin };


}

cstr cstr_from_slice(const char *string, usize len) {
  cstr self = {};
  if (len <= SMALL_BUF_SIZE) {
    init_small_cstr(&self, string,  len);
  } else {
   init_large_cstr(&self, string, len);    
  }
   return self;
}

CStrError cstr_init_small(cstr *self, const char *string) {
  const usize string_len = strlen(string);

  if (string_len <= SMALL_BUF_SIZE) {
    self->buf.len = cast(u8, string_len);
    strncpy(self->buf.mem, string, string_len);
    self->buf.mem[SMALL_BUF_SIZE] = 0;
    return CStrError__Ok;
  }

  return CStrError__StringTooBig;
}

cstr cstr_small_new(const char *string) {
  const usize slen = strlen(string);

  const usize len = cmp_min(slen, SMALL_BUF_SIZE);
  cstr self = {};
  init_small_cstr(&self, string, len);
  return self;

}
void cstr_free(cstr self) {
  // We only have to call free if this is a large
  // cstr that has been allocated on the heap
  if (self.is_large) {
    i32 *prefix_end = pcast(i32, self.heap);
    i32 *prefix = prefix_end - 1;
    void *data = pcast(void, prefix);
    mi_free(data);
  }
}
