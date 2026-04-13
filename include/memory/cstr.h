#pragma once

#include "cast.h"
#include "intdefs.h"

static constexpr usize SMALL_BUF_SIZE = 22;

/// Type alias to make it more clear that
/// this is a pointer to a string that has an [i32] prefix length
/// which you can get by subtracting sizeof([i32]) from this [char]*
typedef char* prefix_str;

/// Type alias to make it more clear that
/// this is a pointer to a string that has an [i32] prefix length
/// which you can get by subtracting sizeof([i32]) from this [char]*
///
/// [const] version of [prefix_str]
typedef const char* cosnt_prefix_str;

struct cstr {
  union {
    struct Small {
      u8 len;
      char mem[SMALL_BUF_SIZE];
      /// padding byte, should always be set to 0 so
      /// that this char buffer always ends with a 0 byte to terminate the
      /// character buffer
      u8 null_pad;
    } buf;
    prefix_str heap;
  };

  bool is_large;
};
typedef struct cstr cstr;
typedef struct Small Small;

typedef enum CStrError {
  /// Ok! No Errors!
  CStrError__Ok = 0,

  /// String is too big to create a small [cstr]
  /// Returned from [cstr_init_small] when passed in null-terminated string is
  /// larger than [SMALL_BUF_SIZE]
  CStrError__StringTooBig,
  /// Error was caused in Underlying allocator and thus
  /// we are unable to create a new [cstr]
  CStrError__AllocatorError,
} CStrError;


static inline cstr cstr_empty(void) {
  return (cstr){  };
}

static inline bool cstr_is_small(const cstr* self) {
  return !self->is_large;
}

static inline usize cstr_len(const cstr* self) {
  if (self->is_large) {
     const i32* prefix_end = (const i32*)self->heap;
     const i32* prefix = prefix_end - 1;
     const usize len = cast(usize, *prefix);   
     return len;
  }

  const usize len = cast(usize, self->buf.len);
  return len;
  
}

static inline const char* cstr_as_str(const cstr* self) {
  if (self->is_large) {
    return self->heap;
  }

  return self->buf.mem;

}

/// Creates a new [cstr] from given null-terminated c-style string.
[[nodiscard("Must use returned cstr! While this type may not always allocate, its still best practice to treat it as if it did allocate")]]
cstr cstr_new(const char* string);

/// Creates a new [cstr] from given string, up to size @param(len)
[[nodiscard("Must use returned cstr! While this type may not always allocate, its still best practice to treat it as if it did allocate")]]
cstr cstr_from_slice(const char* string, usize len);


/// Creates a cstr on the heap wtih given capacity
[[nodiscard("Must use returned cstr! This function always allocates when creating new cstr's")]]
cstr cstr_large_with_capacity(usize capacity);

/// Tries to init a zeroed/uninit [cstr] without allocating on the heap at all.
/// Given null-terminated c-style string must be of length less than [SMALL_BUF_SIZE],
/// otherwise this function returns a [CStrError] and leaves given [cstr] in an invalid/zeroed state
CStrError cstr_init_small(cstr* self, const char* string);

/// Same as [cstr_init_small], but only
/// copies up to the first [SMALL_BUF_SIZE] of given string if
/// it is longer than that instead of returning an error
cstr cstr_small_new( const char* string);

/// Free a created [cstr].
///
/// Only makes a call to free if given cstr
/// is_large flag is set and owns a string allocated on the heap
/// 
/// This function does nothing if [cstr] is small and less than size [SMALL_BUF_SIZE]
///
void cstr_free(cstr self);
