#include "interp/env.h"

#include "core/malloc.h"
#include "interp/state.h"
#include "nv/iter/vec.h"
typedef enum EntryType {
  EnvEntry__Tombstone = -1,
  EnvEntry__Empty = 0,
  EnvEntry__Alive,
} EntryType;

struct EnvEntry {
  EntryType type;
  Rune ident;
  IValue val;
};
alias(EnvEntry);

static constexpr const isize ENV_INIT_CAPACITY = 32;

// TODO: Finish this implementation. For right now im just going to linearly scan each Env Scope to lookup
// definitions at runtime, This is not ideal AT ALL, but fine for getting things working
// TODO: Add growing when load factor is >= 0.75, as well as rehashing entries when resizing

Env env_root_new(void) {
  Vec(EnvEntry) entries = alloc_vec(EnvEntry, ENV_INIT_CAPACITY);
  vec_grow_to_cap(entries);
  return (Env){.parent = nullptr, .names = entries};
}

Env env_child_new(Env* parent) {
  assert(parent);
  Vec(EnvEntry) entries = alloc_vec(EnvEntry, ENV_INIT_CAPACITY);

  vec_grow_to_cap(entries);
  return (Env){.parent = parent, .names = entries};
}

isize env_load_factor(const Env* self) {
  assert(self);
  const isize vec_cap = vec_len(self->names);
  const isize cap = vec_cap == 0 ? 1 : vec_cap;
  return self->count / cap;
}

isize env_capacity(const Env* self) {
  assert(self);
  return vec_len(self->names);
}

static inline EnvEntry* add_entry(Env* self, Rune name) {
  assert(name.hash >= 0);
  const u64 hash = name.hash;
  const i32 elen = vec_len(self->names);
  const u64 mask = elen - 1;

  const i32 index = cast(i32, hash & mask);

  for (i32 i = index, count = 0; count < elen; i++, count++) {
    EnvEntry* entry = &self->names[i];

    if (entry->type == EnvEntry__Alive) {
      continue;
    }

    if (entry->type == EnvEntry__Tombstone || entry->type == EnvEntry__Empty) {
      return entry;
    }

    if (i + 1 >= elen) {
      i = 0;
    }
  }

  return nullptr;
}

static inline EnvEntry* mut_entry(Env* self, Rune name) {
  assert(name.hash >= 0);
  const u64 hash = name.hash;
  const i32 elen = vec_len(self->names);
  const u64 mask = elen - 1;

  const i32 index = cast(i32, hash & mask);

  for (i32 i = index, count = 0; count < elen; i++, count++) {
    EnvEntry* entry = &self->names[i];

    if (entry->type == EnvEntry__Tombstone) {
      continue;
    }
    if (entry->type == EnvEntry__Empty) {
      return nullptr;
    }

    if (entry->type == EnvEntry__Alive && entry->ident.hash == hash) {
      if (sslice_eq(entry->ident.name, name.name)) {
        return entry;
        // return make(Rune, .hash = hash, .name = entry->name, .kwtype = Keyword__None);
      }
      LOG_DBG("Looking up %.*s in Interpreter Env matches entry hash, but not the entry string value! %.*s",
              RSSPREAD(name.name), RSSPREAD(entry->ident.name));
      continue;
    }

    if (i + 1 >= elen) {
      i = 0;
    }
  }

  return nullptr;
}

static inline const EnvEntry* get_entry(const Env* self, Rune name) {
  assert(name.hash >= 0);
  const u64 hash = name.hash;
  const i32 elen = vec_len(self->names);
  const u64 mask = elen - 1;

  const i32 index = cast(i32, hash & mask);

  for (i32 i = index, count = 0; count < elen; i++, count++) {
    const EnvEntry* entry = &self->names[i];

    if (entry->type == EnvEntry__Tombstone) {
      continue;
    }
    if (entry->type == EnvEntry__Empty) {
      return nullptr;
    }

    if (entry->type == EnvEntry__Alive && entry->ident.hash == hash) {
      if (sslice_eq(entry->ident.name, name.name)) {
        return entry;
        // return make(Rune, .hash = hash, .name = entry->name, .kwtype = Keyword__None);
      }
      LOG_DBG("Looking up %.*s in Interpreter Env matches entry hash, but not the entry string value! %.*s",
              RSSPREAD(name.name), RSSPREAD(entry->ident.name));
      continue;
    }

    if (i + 1 >= elen) {
      i = 0;
    }
  }

  return nullptr;
}

bool env_find(const Env* self, Rune name, IValue* out) {
  const Env* curr = self;
  while (is_not_null(curr)) {
    if (env_lookup(self, name, out)) {
      return true;
    }
    curr = self->parent;
  }
}

bool env_lookup(const Env* self, Rune name, IValue* out) {
  const EnvEntry* entry = get_entry(self, name);
  if (is_not_null(entry)) {
    if (out) {
      *out = entry->val;
      return true;
    }
  }
  return false;
}

/// @brief only checks current scope
bool env_in_this_scope(const Env* self, Rune name) { return env_lookup(self, name, nullptr); }

/// @brief Checks parent scopes recursively as well as the current scope
bool env_in_scope(const Env* self, Rune name) {
  const Env* curr = self;
  while (is_not_null(curr)) {
    if (env_in_this_scope(curr, name)) {
      return true;
    }
    curr = curr->parent;
  }
  return false;
}

void env_set(Env* self, Rune name, IValue val) {
  EnvEntry* entry = mut_entry(self, name);
  if (is_null(entry)) {
    entry = add_entry(self, name);
    entry->type = EnvEntry__Alive;
    entry->ident = name;
    self->count += 1;
  }

  entry->val = val;
}

/// @brief will abort execution if name is not currently in this scope
IValue env_get(const Env* self, Rune name) {
  IValue out = {.type = IValue__InvalidUnknown, .data = {.unit = nullptr}};
  env_lookup(self, name, &out);
  return out;
}

bool env_delete(Env* self, Rune name) {
  EnvEntry* entry = mut_entry(self, name);
  if (is_not_null(entry)) {
    entry->type = EnvEntry__Tombstone;
    entry->val = (IValue){};
    entry->ident = (Rune){};
    return true;
  }
  return false;
}

void env_rset(Env* self, Rune name, IValue val) {
  Env* curr = self;
  // try to find name in this scope or parent scopes before inserting
  while (is_not_null(curr)) {
    if (!env_in_this_scope(curr, name)) {
      curr = curr->parent;
      continue;
    }

    env_set(curr, name, val);
    return;
  }
  env_set(self, name, val);
}

IValue env_rget(const Env* self, Rune name) {
  const Env* curr = self;
  while (is_not_null(curr)) {
    const EnvEntry* entry = get_entry(self, name);
    if (is_null(entry)) {
      curr = curr->parent;
      continue;
    }
    return entry->val;
  }
}

bool env_rdelete(Env* self, Rune name) {
  Env* curr = self;
  while (is_not_null(curr)) {
    EnvEntry* entry = mut_entry(curr, name);
    if (is_null(entry)) {
      curr = curr->parent;
      continue;
    }
    entry->type = EnvEntry__Tombstone;
    entry->val = (IValue){};
    entry->ident = (Rune){};
    self->count -= 1;
  }
}
