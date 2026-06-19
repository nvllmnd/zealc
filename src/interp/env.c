#include "interp/env.h"

#include "core/malloc.h"
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

Env env_root_new(void) {
  Vec(EnvEntry) entries = alloc_vec(EnvEntry, ENV_INIT_CAPACITY);
  return (Env){.parent = nullptr, .names = entries};
}

Env env_child_new(Env* parent) {
  assert(parent);
  Vec(EnvEntry) entries = alloc_vec(EnvEntry, 32);
  return (Env){.parent = parent, .names = entries};
}

isize env_load_factor(const Env* self) {
  assert(self);
  const isize vec_cap = vec_len(self->names);
  const isize cap = vec_cap == ? 1 : vec_cap;
  return self->count / cap;
}

isize env_capacity(const Env* self) {
  assert(self);
  return vec_len(self->names);
}

bool env_lookup(const Env* self, Rune name, IValue* out) { const isize mask = env_capacity(self) - 1; }

/// @brief only checks current scope
bool env_in_this_scope(const Env* self, Rune name) METHOD PURE_FUNC;

/// @brief Checks parent scopes recursively as well as the current scope
bool env_in_scope(const Env* self, Rune name) METHOD PURE_FUNC;

bool env_set(Env* self, Rune name, IValue val) METHOD;

/// @brief will abort execution if name is not currently in this scope
IValue env_get(const Env* self, Rune name) METHOD;

bool env_delete(Env* self, Rune name) {}
