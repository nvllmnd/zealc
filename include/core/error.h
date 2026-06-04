#pragma once

#include "nv/core/intdefs.h"

typedef enum ZError : error {
  ZERROR_MIN_ERROR_LIMIT = -0xDEADBAD,


  ZError__FailedDestroyRuneTable,

  ZError__FailedVirtualAlloc,
  ZError__FailedNewOrInitArenaAlloc,
  ZError__FailedNewOrInitStaticAlloc,
  ZError__FailedNewOrInitBlockAlloc,

  ZError__FailedVirtualFree,
  ZError__FailedDestroyOrFreeArenaAlloc,
  ZError__FailedDestroyOrFreeStaticAlloc,

  ZError__FailedInitRuneTable,

  ZError__FailedDestroyOrFreeBlockAlloc,
  /// An [Arena] Allocator returned nullptr from one of its arena_* allocation functions/methods
  ZError__ArenaAllocatorOOM,
  /// Most functions (in libnv, libc, and here in zealc, for instance) return -1 to indicate failure, especially ones
  /// that return a meaningful non-negative value upon success, so 'AnyError' fits this
  ZError__AnyError = -1,
  ZError__OK = 0,

} ZError;

/// shortcut for [ZError__OK]
static constexpr const ZError ZOK = ZError__OK;
/// shortcut for [ZError__AnyError]
static constexpr const ZError ZERROR = ZError__AnyError;

