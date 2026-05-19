#pragma once

#include <stdint.h>
#include <string.h>

#include "nv/core/intdefs.h"
#include "nv/core/sslice.h"
#include "nv/core_types.h"

struct Rune {
  u64 hash;
  sslice name;
};
typedef struct Rune Rune;

static constexpr const Rune RUNE_NONE = make_zeroed(Rune);

PURE_FUNC
static inline bool rune_is_none(Rune self) { return memcmp(&self, &RUNE_NONE, sizeof(Rune)) == 0; }

PURE_FUNC
static inline bool rune_is_ok(Rune self) { return !rune_is_none(self); }

PURE_FUNC
static inline bool rune_eq(Rune left, Rune right) {
  return left.hash == right.hash && sslice_eq(left.name, right.name);
}

static constexpr const i32 RUNE_MAX_SIZE = INT16_MAX;

void runetab_init(i32 entry_len, i32 name_storage_in_mb);

PURE_FUNC
f32 runetab_load_factor(void);

/// Resizes entry array, returns new length of array. Does nothing if new_entry_len <= current entry count
i32 runetab_resize(i32 new_entry_len);

Rune runetab_add(sslice name);

PURE_FUNC
bool runetab_has(Rune rune);

PURE_FUNC
bool runetab_has_str(const char* string, i32 string_len);

PURE_FUNC
Rune runetab_lookup(sslice name);

void runetab_destroy(void);


void runetab_clear(void);

void runetab_print_entries(void);
