#include "ast/symbol.h"
#include "ast/token.h"
#include "core_types.h"

struct Keyword {
  i32 id;
  TokenType type;
};

bool is_keyword(const char* str, usize len) {
  const Keyword* kw = lookup_keyword(str, len);
  return is_not_null(kw);
}


const char* kw_string(const Keyword* self) {
  
}

sslice kw_sslice(const Keyword* self) {
  
}



