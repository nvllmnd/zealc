#include "ast/symbol.h"

#include "nv/core/constants.h"
#include "nv/core_types.h"

static constexpr const isize PS_LOCAL_SIZE = 30;
/// Similar to [cstr], but bigger sized (as this will live on heap) and non-owning
/// Used internally by [StringPool]
struct PoolString {
  bool inl;
  union {
    /// lives inline with [StringPoolEntry]
    struct {
      char len;
      char s[PS_LOCAL_SIZE];
    } local;
    /// lives in [StringPool] string pool memory
    sslice pooled;
  };
};
alias(PoolString);

struct StringPoolEntry {
  /// index byte offset to [InterrnedString] buffer for this entry
  i32 offset;
  /// entry type. if any variant other than [SPoolEntry__Keyword], then
  /// string data lives in pstring field, otherwise kw
  SPoolEntryType type;
  /// full (non-moduloed/masked) hash of entry
  /// this should be as close to a globally unique identifer for any string as we can get
  u64 hash;


  union {
    PoolString pstring;
    Keyword kw;
  };
};
alias(StringPoolEntry);




struct InterrnedString {
  i32 len;
  char string[];
};

alias(InterrnedString);

struct StringPool {

  /// Buffer of entries. Each entry
  struct {
   StringPoolEntry* es;
   i32 len;
   i32 cap;  
  } entries;

  
  /// owned string buffer used for internned strings
  struct {
    /// bytes used
    i32 used;
    /// capacity in bytes
    i32 cap;
    /// length prefixed, null delimited interrned string storage buffer
    InterrnedString* s;
  } buf;

};
alias(StringPool);


static StringPool SP = make_zeroed(StringPool);

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


// StringPool* stringpool(void) {
  
// }



// SPoolSlot stringpool_insert(const char* s, isize len) {
//   const u64 hash  = fnv_hash64(s,  len);
//   // const isize index = hash %
// }

// sslice stringpool_lookup(SPoolSlot slot) {
  
// }

// bool stringpool_exists(const char* s, isize len) {
  
// }


// void stringpool_init(void) {
//   static constexpr const isize INIT_SIZE = MEGABYTES(1) / sizeof(StringPoolEntry);
//   static constexpr const isize PINIT_SIZE = INIT_SIZE * 2;
//   SP.entries.es = arena_heap_zalloc(gpa_main(), INIT_SIZE, 1);
//   SP.entries.cap = INIT_SIZE;
//   SP.entries.len = 0;
//   SP.buf.s = arena_heap_zalloc(gpa_main(), PINIT_SIZE , 1);
//   SP.buf.cap = PINIT_SIZE;
//   SP.buf.used = 0;
// }

void stringpool_free(void) {
  // mi_free(SP.entries.es);
  // mi_free(SP.buf.s);

  memset(&SP, 0, sizeof(SP));

}


