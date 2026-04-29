#include "ast/symbol.h"

#include "core_types.h"


// struct StringPool {
  
// };
// alias(StringPool);


bool is_keyword(const char* str, usize len) {
  const Keyword* kw = kw_lookup_str(str, len);
  return is_not_null(kw);
}

sslice kw_sslice(const Keyword* self) {
  const char* s = kw_string(self);
  const i32 len = stringlen(s);
  return sslice_new(s, len);
}

const Keyword* kw_lookup(sslice str) { return kw_lookup_str(str.begin, str.len); }

static constexpr const u32 PRIME32 = 0x010001930;
static constexpr const u32 OFFSET32 = 0x811c9dc5;
static constexpr const u64 PRIME64 = 0x00000100000001b3;
static constexpr const u64 OFFSET64 = 0xcbf29ce484222325;

u32 fnv_hash32(const char* string, isize len) {
  if (is_null(string) || len <= 0) {
    return 0;
  }
  u32 hash = OFFSET32;
  for (i32 i = 0; i < len; i++) {
    const u32 c = string[i];
    hash = (hash ^ c) * PRIME32;
  }
  return hash;
}

u64 fnv_hash64(const char* string, isize len) {
  if (is_null(string) || len <= 0) {
    return 0;
  }
  u64 hash = OFFSET64;

  for (i32 i = 0; i < len; i++) {
    const u64 c = string[i];
    hash = (hash ^ c) * PRIME64;
  }
  return hash;
}


// StringPool* stringpool(void) {
  
// }
