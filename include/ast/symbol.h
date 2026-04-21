#pragma once



#include "attributes.h"
#include "intdefs.h"
#include "memory/cstr.h"
typedef struct Keyword Keyword;

// typedef enum KeywordType {
  
// } KeywordType;

enum KeywordType {

 Keyword__True,
 Keyword__False,
 Keyword__Let,
 Keyword__If,
 Keyword__Else,
 Keyword__Mut,
 Keyword__When,
 Keyword__Fn,
 Keyword__Struct,
 Keyword__Trait,
 Keyword__And,
 Keyword__Or,
 Keyword__Impl,
 Keyword__Return,
 Keyword__Self,
 Keyword__Const,
 Keyword__Loop,
 Keyword__For,
 Keyword__While,
 Keyword__Continue,
 Keyword__Break,
 Keyword__Match,
 Keyword__Pub,
 Keyword__Ref,
 Keyword__Error,
 Keyword__Enum,
 Keyword__Type,
 Keyword__Await,
 Keyword__Comptime,
 Keyword__Static,
 Keyword__Mod,
 Keyword__Macro,
 Keyword__Derive,
 Keyword__Dyn,
 Keyword__Default,
 Keyword__Sizeof,
};
typedef enum KeywordType KeywordType;

PURE_FUNC
PARAMS_NONNULL(1)
const Keyword* lookup_keyword(register const char* str, register usize len);

PURE_FUNC
PARAMS_NONNULL(1)
bool is_keyword(const char* str,  usize len);


METHOD
PURE_FUNC
const char* kw_string(const Keyword* self);

METHOD
PURE_FUNC
sslice kw_sslice(const Keyword* self);




