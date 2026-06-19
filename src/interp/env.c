#include "interp/env.h"

bool env_lookup(const Env* self, Rune name, IValue* out) METHOD;

/// @brief only checks current scope
bool env_in_this_scope(const Env* self, Rune name) METHOD PURE_FUNC;

/// @brief Checks parent scopes recursively as well as the current scope
bool env_in_scope(const Env* self, Rune name) METHOD PURE_FUNC;

bool env_set(Env* self, Rune name, IValue val) METHOD;

/// @brief will abort execution if name is not currently in this scope
IValue env_get(const Env* self, Rune name) METHOD;
