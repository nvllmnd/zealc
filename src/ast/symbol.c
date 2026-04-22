#include "ast/symbol.h"
#include "core_types.h"

bool is_keyword(const char* str, usize len) {
  const Keyword* kw = kw_lookup_str(str, len);
  return is_not_null(kw);
}

sslice kw_sslice(const Keyword* self) {
  const char* s = kw_string(self);
  const i32 len = stringlen(s);
  return sslice_new(s, len);
    
}

const Keyword* kw_lookup(sslice str) {
  return kw_lookup_str(str.begin, str.len);  
}

