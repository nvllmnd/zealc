#pragma once
#include "intdefs.h"


/// Function pointer typedef for [Allocator] [AllocVTable] allocate method.
///
/// void* self  - Pointer to self (may be null if Allocator has no state!)
///              Typically you will cast this to your derived Allocator type in this method implementation
/// usize size  - Size of allocation requested in bytes
/// usize align - Alignment of requested allocation 
typedef void*(*VTableAllocate)(void* self, usize size, usize align);
/// Function pointer typedef for [Allocator] [AllocVTable] reallocate method
///
/// void* self     - Pointer to self (may be null if Allocator has no state!)
///                  Typically you will cast this to your derived Allocator type in this method implementation
/// void* ptr      - Pointer to begging of block of memory to be reallocated
/// usize new_size - Size of requested reallocation in bytes
/// usize align    - Alignment of requested allocation
typedef void*(*VTableReallocate)(void* self, void* ptr, usize new_size, usize align);

/// Function pointer typedef for [Allocator] [AllocVTable] zallocate method
/// This is the same as [VTableAllocate], but ensures allocated memory is zeroed
///
/// void* self  - Pointer to self (may be null if Allocator has no state!)
///               Typically you will cast this to your derived Allocator type in this method implementation 
/// usize size  - Size of allocation requested in bytes
/// usize align - Alignment of requested allocation
typedef void*(*VTableZallocate)(void* self, usize size, usize align);

/// Function pointer typedef for [Allocator] [AllocVTable] expand method
/// This is the same as [VTableReallocate], but does nothing  if
/// memory cannot be expanded in place to new_size
///
/// void* self     - Pointer to self (may be null if Allocator has no state!)
///                  Typically you will cast this to your derived Allocator type in this method implementation 
/// void* ptr      - Pointer to begging of block of memory to be expanded in place
/// usize new_size - Size of requested reallocation in bytes
typedef void*(*VTableExpand)(void* self, void* ptr, usize new_size);

/// Function pointer typedef for [Allocator] [AllocVTable] free method.
///
/// void* self - Pointer to self (may be null if Allocator has no state!)
///              Typically you will cast this to your derived Allocator type in this method implementation
/// void* ptr  - Pointer to block of memory to be freed by this allocator
typedef void(*VTableFree)(void* self, void* ptr);


/// [Allocator] VTable struct that contains function pointers
/// to Allocator implementations
struct AllocVTable {
  /// See [VTableReallocate]
  VTableAllocate allocate;
  /// See [VTableReallocate]
  VTableReallocate reallocate;
  /// See [VTableZallocate]
  VTableZallocate zallocate;
  /// See [VTableExpand]
  VTableExpand expand_in_place;
  /// See [VTableFree]
  VTableFree free;
};
typedef struct AllocVTable AllocVTable;

/// C-Style Allocator Interface
struct Allocator {
  void* ctx;
  const AllocVTable* vtable;
};
 typedef struct Allocator Allocator;
