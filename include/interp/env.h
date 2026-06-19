#pragma once

#include "interp/state.h"
#include "nv/core/attributes.h"
#include "runes.h"

struct Env {
  struct Env* parent;
  Vec(struct EnvEntry) names;
  isize count;
};
alias(Env);

Env env_root_new(void);

Env env_child_new(Env* parent) PARAMS_NONNULL(1);

isize env_capacity(const Env* self) PURE_FUNC METHOD;

PURE_FUNC METHOD static inline isize env_len(const Env* self) { return self->count; }

isize env_load_factor(const Env* self) PURE_FUNC METHOD;

bool env_lookup(const Env* self, Rune name, IValue* out) METHOD;

/// @brief same as [env_lookup] but recursively walks up parent nodes if given name is not
/// in this scope
bool env_find(const Env* self, Rune name, IValue* out) METHOD;

/// @brief only checks current scope
bool env_in_this_scope(const Env* self, Rune name) METHOD PURE_FUNC;

/// @brief Checks parent scopes recursively as well as the current scope
bool env_in_scope(const Env* self, Rune name) METHOD PURE_FUNC;

void env_set(Env* self, Rune name, IValue val) METHOD;

/// @brief will abort execution if name is not currently in this scope
IValue env_get(const Env* self, Rune name) METHOD;

bool env_delete(Env* self, Rune name) METHOD;

void env_rset(Env* self, Rune name, IValue val) METHOD;

/// @brief will abort execution if name is not currently in this scope
IValue env_rget(const Env* self, Rune name) METHOD;

bool env_rdelete(Env* self, Rune name) METHOD;
