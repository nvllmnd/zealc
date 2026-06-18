#pragma once


#include "interp/state.h"
#include "runes.h"

struct EnvEntry {
  Rune ident;
  IValue val;
};
alias(EnvEntry);

struct Env {
  struct Env* parent;
  Vec(EnvEntry) names;
};
alias(Env);

bool env_lookup(Rune name, IValue* out);

/// @brief only checks current scope
bool env_in_this_scope(Rune name);

/// @brief Checks parent scopes recursively as well as the current scope
bool env_in_scope(Rune name);

bool env_set(Rune name, IValue val);

/// @brief will abort execution if name is not currently in this scope
IValue env_get(Rune name);



