#include "core/runes.h"

#include <stdint.h>

#include "nv/core/algo.h"
#include "nv/core/attributes.h"
#include "nv/core/buffer.h"
#include "nv/core/constants.h"
#include "nv/core/intdefs.h"
#include "nv/core/log.h"
#include "nv/core/sslice.h"
#include "nv/core_types.h"
#include "nv/memory/alloc.h"
#include "nv/memory/arena.h"
#include "nv/memory/layout.h"

typedef enum RuneType { Rune__Empty = 0, Rune__Used, Rune__TypeCount } RuneType;
typedef struct RuneTable RuneTable;

static constexpr const f32 LOAD_FACTOR = 0.75;

struct RuneEntry {
  RuneType type;
  sslice name;

  u64 hash;
};
alias(RuneEntry);

static void runetab_rehash_entries(RuneTable* self, const Vec(RuneEntry) old_entries);

// NOTE: Yes this struct is tiny, and yes we are fine with allocating it as an opaque pointer. Most allocators own or
// allocate into contiguous virtual memory, so the cost of allocations are cheap, so allocating this tiny guy is ok. If
// we are really obsessed with this tiny struct residing in memory that isnt dynamicall allocated/managed, then use
// StaticAlloc to allocate the oqaque pointer plus Since we allocate into contiguous virtual memory, there is a great
// chance this RuneTable struct will live right next to the entries it contains in memory
struct RuneTable {
  Vec(RuneEntry) entries;
  i32 entries_count;

  Arena* alloc;
};

static RuneTable RT = {};
static Allocator RT_ALLOC = {};

void runetab_init(i32 entry_len, i32 name_storage_in_mb) {
  assert(entry_len > 0);
  assert(name_storage_in_mb > 2);

  if UNLIKELY (is_not_null(RT.entries) && is_not_null(RT.alloc)) {
    return;
  }

  Arena* arena = arena_new(name_storage_in_mb, KILOBYTES(8));
  if UNLIKELY (is_null(arena)) {
    LOG_DBG(FILE_FMT " :: Failed to create new Arena of size %dMB and init capacity: %d",
            FILE_FMT_ARGS(RuneTable, name_storage_in_mb, KILOBYTES(1)));
    return;
  }

  RT.alloc = arena;
  RT_ALLOC = arena_allocator(RT.alloc);
  RT.entries = vec_new(RuneEntry, entry_len, RT_ALLOC);
  RT.entries_count = 0;

  if UNLIKELY (is_null(RT.entries)) {
    LOG_DBG(FILE_FMT " :: Arena allocator failed to allocate RuneEntry array of size: %d bytes!",
            FILE_FMT_ARGS(RuneTable, (i32)sizeof(RuneEntry) * entry_len));
    arena_destroy(RT.alloc);
    memset(&RT, 0, sizeof(RuneTable));
    memset(&RT_ALLOC, 0, sizeof(Allocator));

    assert(RT.entries);

    return;
  }
}

i32 runetab_resize(i32 new_entry_len) {
  assert(allocator_is_ok(RT_ALLOC));
  assert(RT.entries);
  assert(new_entry_len > 0);

  const i32 curr_len = vec_len(RT.entries);
  // dont do anything for shrinking, as that would cause us to have to rehash everything, so shrinking is not
  // desireable
  if (new_entry_len <= curr_len) {
    return RT.entries_count;
  }

  Vec(RuneEntry) const old_entries = RT.entries;

  const f32 next_load_factor = (f32)new_entry_len / RT.entries_count;

  // make sure next load factor is large enough, otherwise this resize is pretty useless!
  if (next_load_factor > LOAD_FACTOR) {
    new_entry_len *= 4;
  }

  RT.entries = vec_resize(RT.entries, new_entry_len, RT_ALLOC);

  if UNLIKELY (is_null(RT.entries)) {
    LOG_DBG(FILE_FMT " :: Unable to resize RuneTable to %d entries!", FILE_FMT_ARGS(RuneTable, new_entry_len));

    return RT.entries_count;
  }

  vec_grow_to_cap(RT.entries);

  runetab_rehash_entries(&RT, old_entries);

  return new_entry_len;
}

f32 runetab_load_factor(void) {
  assert(RT.entries);

  const i32 len = vec_len(RT.entries);
  const f32 count = RT.entries_count == 0 ? 1. : (f32)RT.entries_count;

  return len / count;
}

Rune runetab_add(sslice name) {
  assert(allocator_is_ok(RT_ALLOC));
  assert(name.begin);
  assert(name.len > 0);

  if (is_null(RT.entries)) {
    RT.entries = vec_new(RuneEntry, KILOBYTES(1), RT_ALLOC);
  }

  if (vec_len(RT.entries) == 0) {
    vec_resize(RT.entries, KILOBYTES(1), RT_ALLOC);
  }

  const f32 load = runetab_load_factor();

  if (load >= LOAD_FACTOR) {
    runetab_resize(vec_len(RT.entries) * 4);
  }

  const Rune r = runetab_lookup(name);
  if (rune_is_ok(r)) {
    return r;
  }

  const u64 hash = fnv_hash64(name.begin, name.len);
  const i32 len = vec_len(RT.entries);
  const u64 mask = len - 1;

  const i32 index = cast(i32, hash & mask);

  RuneEntry* entry = &RT.entries[index];

  for (i32 i = clamp(index + 1, 0, len - 1); i < len; i++) {
    entry = &RT.entries[i];

    // we only have to worry about insertion here, since we do a full lookup above with the call to runetab_lookup_rune
    // so we know if we got here, we did not find an entry at this hashed index

    if (entry->type == Rune__Empty) {
      const char* s = arena_strndup(RT.alloc, name.begin, name.len);
      const sslice sl = sslice_new(s, name.len);
      entry->name = sl;
      entry->type = Rune__Used;
      entry->hash = hash;
      RT.entries_count += 1;
      return make(Rune, .hash = entry->hash, .name = entry->name);
    }

    if UNLIKELY (i == index) {
      return RUNE_NONE;
    }

    if (i + 1 >= len) {
      i = 0;
    }
  }

  LOG_DBG("Something went wrong when trying to add rune: %.*s to RuneTable!", name.len, name.begin);
  assert(false);

  UNREACHABLE();
}

bool runetab_has(Rune rune) { return runetab_has_str(rune.name.begin, rune.name.len); }

bool runetab_has_str(const char* string, i32 string_len) {
  assert(string);
  assert(string_len > 0);
  const sslice name = sslice_new(.begin = string, .len = string_len);
  const Rune rune = runetab_lookup(name);
  return !rune_is_none(rune);
}

Rune runetab_lookup(sslice name) {
  assert(RT.entries);
  const u64 hash = fnv_hash64(name.begin, name.len);
  const i32 elen = vec_len(RT.entries);
  const u64 mask = elen - 1;

  const i32 index = cast(i32, hash & mask);

  for (i32 i = clamp(index, 0, elen - 1); i < elen; i++) {
    const RuneEntry* entry = &RT.entries[i];

    entry = &RT.entries[i];

    if (entry->type == Rune__Empty) {
      return RUNE_NONE;
    }

    if (entry->type > Rune__Empty && entry->hash == hash) {
      if LIKELY (sslice_eq(entry->name, name)) {
        return make(Rune, .hash = hash, .name = entry->name);
      }
      LOG_DBG("Looking up %.*s in RuneTable matches entry hash, but not the entry string value! %.*s", name.len,
              name.begin, entry->name.len, entry->name.begin);
      continue;
    }

    // we looped all the way around, but havent found this entry, this should only happen if given name does not exist
    // in this table
    if (i == index) {
      return RUNE_NONE;
    }

    if (i + 1 >= elen) {
      i = 0;
    }
  }

  return RUNE_NONE;
}

// METHOD
// PURE_FUNC
// sslice runetab_lookup(const RuneTable* self, Rune rune) {
//   assert(self);
//   assert(self->entries);
//   assert(rune.id != RUNE_NONE.id || self->entries_count == 0);
//   if (rune.parent != self) {
//     return sslice_empty();
//   }
//   assert(rune.id < vec_len(self->entries));
//   const RuneEntry entry = self->entries[rune.id];
//   if (entry.type == Rune__Empty) {
//     return sslice_empty();
//   }
//   return entry.name;
// }

void runetab_destroy(void) {
  if (is_not_null(RT.alloc)) {
    arena_destroy(RT.alloc);
    memset(&RT, 0, sizeof(RuneTable));
    memset(&RT_ALLOC, 0, sizeof(Allocator));
  }
}

void runetab_rehash_entries(RuneTable* self, const Vec(RuneEntry) old_entries) {
  Vec(RuneEntry) entries = self->entries;

  const i32 entries_len = vec_len(entries);
  const u64 mask = entries_len - 1;

  vec_for(old_entries) {
    const RuneEntry entry = old_entries[i];

    const u64 hash = entry.hash;

    const i32 index = (hash & mask);
    assert(index < entries_len);

    {
      RuneEntry* next = &entries[index];
      if LIKELY (next->type == Rune__Empty) {
        *next = entry;
        continue;
      }

      LOG_DBG(FILE_FMT
              "Hash collision for entry: %.*s. Starting linear probe! If this is happening often (or really any more "
              "than very infrequently) then you may want to increase the capacity of that RuneTable's RuneEntry Vec!",
              FILE_FMT_ARGS(RuneTable, entry.name.len, entry.name.begin));

      // linear probe through Entries to find an empty cell.
      // this should only happen rarely (if at all) in case of hash collision
      const i32 elen = vec_len(entries);
      for (i32 j = clamp(i + 1, 0, elen - 1); j < elen; j++) {
        if (j + 1 >= elen) {
          j = 0;
        }

        next = &entries[j];
        if (next->type == Rune__Empty) {
          LOG_DBG(FILE_FMT "Found Hash Collision Entry using linear probing at index: %d", FILE_FMT_ARGS(RuneTable, j));
          *next = entry;
          break;
        }

        // we have wrapped full around and not found an entry. This should never happen!
        if UNLIKELY (j == i) {
          log_fatal(FILE_FMT "Could not find a free entry in RuneTable while trying to rehash entries during resize!",
                    FILE_FMT_ARGS());
        }
      }
    }
  }
}

void runetab_clear(void) {
  assert(allocator_is_ok(RT_ALLOC));
  arena_clear(RT.alloc);
  
  RT.entries = nullptr;
}

void runetab_print_entries(void) {
  println("====== Printing RuneTable Entries: ======");
  vec_for(RT.entries) {
    const RuneEntry entry = RT.entries[i];
    println("#%d: %*.s", i + 1, entry.name.len, entry.name.begin); 
  }  
  println("====== End RuneTable Entries ======");
}
