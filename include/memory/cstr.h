#pragma once

#include "cast.h"
#include "core_types.h"
#include "intdefs.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
typedef const char* const_prefix_str;

struct cstr {
  union {
    struct Small {
      u8 len;
      char mem[SMALL_BUF_SIZE + 1]; // +1 for null terminator
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

static inline bool cstr_is_heap_allocated(const cstr* self) {
  return self->is_large;
}

static inline usize cstr_len(const cstr* self) {
  if (self->is_large) {
     const i32* str_begin = pcast(i32, self->heap);
     const usize len = cast(usize, str_begin[-1]);   
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
/// Keep in mind the cstr returned by this function is completely zeroed out,
/// but contains a prefix length of given capacity.
/// So inspecting this resulting cstr, you will observe that it is reporting it has a length,
/// despite being completely zeroed
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


/// Shrinks [cstr] to length of given @param(smaller_size) in bytes.
/// if [cstr] has a length already smaller than given @param(smaller_size),
/// then this function returns false and bails out early, changing nothing.
///
/// This function will not, for instance, move a heap allocated string of size larger than [SMALL_BUF_SIZE],
/// onto the stack, even if @param(smaller_size) < [SMALL_BUF_SIZE]. Allocations do not move from where they currently
/// are as of invoking this function. This is a checked operation around a simple assignment of this cstr's current
/// length field to @param(smaller_size) 
///
/// This function will return true upon successful shrinking to @param(smaller_size), otherwise false
///
bool cstr_shrink_to(cstr* self, usize smaller_size);

/// Free a created [cstr].
///
/// Only makes a call to free if given cstr
/// is_large flag is set and owns a string allocated on the heap
/// 
/// This function does nothing if [cstr] is small and less than size [SMALL_BUF_SIZE]
///
void cstr_free(cstr* self);




cstr cstr_concat(const cstr* left, const cstr*right);

/// same as [cstr_concat], but frees the concatenated [cstr]s after copying their data to
/// the new [cstr] that is contains a string that is left + right
cstr cstr_move_concat(cstr* left,  cstr*right);
void cstr_append_string(cstr* self, const char* s, usize len);

/// Helper function for [cstr_token] macro. Not for any other use otherwise
/// PLS DONT USE KTHNX
static inline cstr priv_cstr_token_impl(const char* s, usize len) {
  cstr self = {}; 
  strncpy(self.buf.mem, s, len);
  self.buf.len = cast(u8, len);
  return self;
}



#define cstr_token(string_literal)({ \
 static_assert(type_eq(string_literal, const char*)) \
 /* Creats a new cstr allocated on the stack. Contains a static assurtion to check that*/ \
 /* the provided string_literal argument is of length < [SMALL_BUF_SIZE]*/ \
 constexpr const usize _LEN = sizeof((string_literal)); \
 static_assert(_LEN < SMALL_BUF_SIZE); \
 priv_cstr_token_impl(string_literal, _LEN); \
})

