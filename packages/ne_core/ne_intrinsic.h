/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

// Intrinsic functions are not treated as function pointers because they are
// expected to be implemented directly by the compiler and heavily optimized.
// They also do not return the typical result codes.

/// Sets a contiguous block of memory to a specified \p value, starting with the
/// address given by \p memory and ending at \p memory + \p size (in bytes).
/// @param memory
///   The base address of memory that we want to set.
/// @param value
///   The value we want writtten to each byte.
/// @param size
///   Number of bytes in memory we want to set, starting from \p memory.
NE_CORE_API void ne_intrinsic_memory_set(void *memory,
                                         uint8_t value,
                                         uint64_t size);

/// Copies a contiguous block of memory from the \p source to the \p
/// destination. The contiguous blocks from \p source and \p destination must
/// not overlap.
/// @param destination
///   The address we wish to copy to.
/// @param source
///   The address we wish to copy from.
/// @param size
///   Number of bytes in memory we want to copy.
NE_CORE_API void ne_intrinsic_memory_copy(void *destination,
                                          const void *source,
                                          uint64_t size);

/// Compares two contiguous blocks of memory.
/// @param a
///   The first address whose bytes we wish to compare.
/// @param b
///   The second address whose bytes we wish to compare.
/// @param size
///   Number of bytes in memory we want to compare.
/// @return
///   Returns 0 if the memory is equal, less than 0 if the bytes in \p a are
///   less than the bytes in \p b, and greater than 0 if the bytes in \p a are
///   greater than the bytes in \p b.
NE_CORE_API int64_t ne_intrinsic_memory_compare(const void *a,
                                                const void *b,
                                                uint64_t size);
