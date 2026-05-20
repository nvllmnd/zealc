#include "core/runes.h"

#include <stdint.h>
#include <string.h>

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

i32 runetab_grow(i32 new_entry_len) {
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
    LOG_DBG(FILE_FMT "Unable to resize RuneTable to %d entries!", FILE_FMT_ARGS(RuneTable, new_entry_len));

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

  // look up Keywords first, as they live in a perfect hash table and lookups are very fast
  {
    const Keyword* kw = kw_lookup_str(name.begin, name.len);
    if (is_not_null(kw)) {
      const sslice kslice = kw_sslice(kw);
      // Keyword strings live in static storage in keywords.c, so
      // Keyword-type Runes dont have a hash value (we dont need it, if 2 Runes are both Keyword Runes, we can tell they
      // are equal simply by comparing thier enum type fields) and we store a slice to the string and set the keyword
      // type
      return make(Rune, .kwtype = kw->type, .hash = 0, .name = kslice);
    }
  }

  if UNLIKELY (is_null(RT.entries)) {
    RT.entries = vec_new(RuneEntry, KILOBYTES(1), RT_ALLOC);
    vec_grow_to_cap(RT.entries);
  }

  if UNLIKELY (vec_len(RT.entries) == 0) {
    vec_resize(RT.entries, KILOBYTES(1), RT_ALLOC);
  }

  if UNLIKELY (runetab_load_factor() >= LOAD_FACTOR) {
    runetab_grow(vec_len(RT.entries) * 4);
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
      return make(Rune, .hash = entry->hash, .name = entry->name, .kwtype = Keyword__None);
    }

    if UNLIKELY (i == index) {

      LOG_DBG(FILE_FMT
              "Did a full scan of RuneTable while looking up name: %.*s for insertion, but was not able to find it or an empty cell to insert into "
              "Something is big wrong!",
              FILE_FMT_ARGS(RuneTable, name.len, name.begin));
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
  assert(name.begin);
  assert(name.len > 0);

  {
    const Keyword* kw = kw_lookup(name);
    if (is_not_null(kw)) {
      const sslice kslice = kw_sslice(kw);
      return make(Rune, .kwtype = kw->type, .hash = 0, .name = kslice);
    }
  }

  const u64 hash = fnv_hash64(name.begin, name.len);
  const i32 elen = vec_len(RT.entries);
  const u64 mask = elen - 1;

  const i32 index = cast(i32, hash & mask);

  for (i32 i = clamp(index, 0, elen - 1); i < elen; i++) {
    const RuneEntry* entry = &RT.entries[i];

    if (entry->type == Rune__Empty) {
      return RUNE_NONE;
    }

    if (entry->type > Rune__Empty && entry->hash == hash) {
      if LIKELY (sslice_eq(entry->name, name)) {
        return make(Rune, .hash = hash, .name = entry->name, .kwtype = Keyword__None);
      }
      LOG_DBG("Looking up %.*s in RuneTable matches entry hash, but not the entry string value! %.*s", name.len,
              name.begin, entry->name.len, entry->name.begin);
      continue;
    }

    // we looped all the way around, but havent found this entry, this should only happen if given name does not exist
    // in this table (though realistically, not at all, we dont remove entries and only do lookups and additions when
    // load factor is < 0.75, so ending up here is VERY! unlikely)
    // TODO: As soon as im certain this branch will never be taken, we can put a UNREACHABLE here or something...
    if UNLIKELY (i == index) {
      LOG_DBG(FILE_FMT
              "Did a full scan of RuneTable while looking up name: %.*s but was not able to find it or an empty cell. "
              "Something is big wrong!",
              FILE_FMT_ARGS(RuneTable, name.len, name.begin));
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

bool is_keyword(const char* str, i32 len) {
  assert(str);
  assert(len > 0);
  const Keyword* kw = kw_lookup_str(str, len);
  return is_not_null(kw);
}

sslice kw_sslice(const Keyword* self) {
  const char* s = kw_string(self);
  const i32 len = stringlen(s);
  return sslice_new(s, len);
}

const Keyword* kw_lookup(sslice str) { return kw_lookup_str(str.begin, str.len); }
const char* kw_literal(Keyword kw) {
  assert(kw.type >= Keyword__True && kw.type < Keyword__Count);

  static const char* KEYWORD_LITERAL[] = {
      [Keyword__True] = "true",
      [Keyword__False] = "false",
      [Keyword__If] = "if",
      [Keyword__Else] = "else",
      [Keyword__Mut] = "mut",
      [Keyword__When] = "when",
      [Keyword__Fn] = "fn",
      [Keyword__Struct] = "struct",
      [Keyword__Trait] = "trait",
      [Keyword__Impl] = "impl",
      [Keyword__And] = "and",
      [Keyword__Or] = "or",
      [Keyword__Return] = "return",
      [Keyword__Self] = "self",
      [Keyword__Const] = "const",
      [Keyword__Loop] = "loop",
      [Keyword__For] = "for",
      [Keyword__While] = "while",
      [Keyword__Break] = "break",
      [Keyword__Match] = "match",
      [Keyword__Continue] = "continue",
      [Keyword__Pub] = "pub",
      [Keyword__Ref] = "ref",
      [Keyword__Error] = "error",
      [Keyword__Enum] = "enum",
      [Keyword__Type] = "type",
      [Keyword__Await] = "await",
      [Keyword__Comptime] = "comptime",
      [Keyword__Static] = "static",
      [Keyword__Mod] = "mod",
      [Keyword__Macro] = "macro",
      [Keyword__Derive] = "derive",
      [Keyword__Dyn] = "dyn",
      [Keyword__Default] = "default",
      [Keyword__Sizeof] = "sizeof",
  };
  return KEYWORD_LITERAL[kw.type];
}

CONST_FUNC
TokenType kw_tokentype(Keyword kw) {
  assert(kw.type >= Keyword__True && kw.type < Keyword__Count);

  static constexpr const TokenType KEYWORD_TOKENTYPE[] = {
      [Keyword__True] = Token__True,
      [Keyword__False] = Token__False,
      [Keyword__If] = Token__If,
      [Keyword__Else] = Token__Else,
      [Keyword__Mut] = Token__Mut,
      [Keyword__When] = Token__When,
      [Keyword__Fn] = Token__Fn,
      [Keyword__Struct] = Token__Struct,
      [Keyword__Trait] = Token__Trait,
      [Keyword__Impl] = Token__Impl,
      [Keyword__And] = Token__And,
      [Keyword__Or] = Token__Or,
      [Keyword__Return] = Token__Return,
      [Keyword__Self] = Token__Self,
      [Keyword__Const] = Token__Const,
      [Keyword__Loop] = Token__Loop,
      [Keyword__For] = Token__For,
      [Keyword__While] = Token__While,
      [Keyword__Break] = Token__Break,
      [Keyword__Match] = Token__Match,
      [Keyword__Continue] = Token__Continue,
      [Keyword__Pub] = Token__Pub,
      [Keyword__Ref] = Token__Ref,
      [Keyword__Error] = Token__Error,
      [Keyword__Enum] = Token__Enum,
      [Keyword__Type] = Token__Type,
      [Keyword__Await] = Token__Await,
      [Keyword__Comptime] = Token__Comptime,
      [Keyword__Static] = Token__Static,
      [Keyword__Mod] = Token__Mod,
      [Keyword__Macro] = Token__Macro,
      [Keyword__Derive] = Token__Derive,
      [Keyword__Dyn] = Token__Dyn,
      [Keyword__Default] = Token__Default,
      [Keyword__Sizeof] = Token__Sizeof,
  };
  return KEYWORD_TOKENTYPE[kw.type];
}
