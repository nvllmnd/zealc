#pragma once

#include "nv/core/attributes.h"
#include "nv/core/intdefs.h"

typedef enum ZError : u64 {

  ZError__OK = 0,
  ZOK = ZError__OK,
  ZError__Error = 1,
  ZERROR = ZError__Error,

  ZError__FailedDestroyRuneTable = 1 << 1,

  ZError__FailedVirtualAlloc = 1 << 2,
  ZError__FailedNewOrInitArenaAlloc = 1 << 3,
  ZError__FailedNewOrInitStaticAlloc = 1 << 4,
  ZError__FailedNewOrInitBlockAlloc = 1 << 5,

  ZError__FailedVirtualFree = 1 << 6,
  ZError__FailedDestroyOrFreeArenaAlloc = 1 << 7,
  ZError__FailedDestroyOrFreeStaticAlloc = 1 << 8,

  ZError__FailedInitRuneTable = 1 << 9,

  ZError__FailedDestroyOrFreeBlockAlloc = 1 << 10,
  /// An [Arena] Allocator returned nullptr from one of its arena_* allocation functions/methods
  ZError__ArenaAllocatorOOM = 1 << 11,
  ZError__InterpFailedToLoadFile = 1 << 12,
  ZError__ParseError = 1 << 13,
  ZERROR_COUNT = 14,
  /// Most functions (in libnv, libc, and here in zealc, for instance) return -1 to indicate failure, especially ones
  /// that return a meaningful non-negative value upon success, so 'AnyError' fits this

} HEDLEY_FLAGS ZError;
