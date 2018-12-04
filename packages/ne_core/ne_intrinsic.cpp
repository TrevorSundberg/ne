// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_core/ne_intrinsic.h"
#include <cstring>

/******************************************************************************/
void ne_intrinsic_memory_set(void *memory, uint8_t value, uint64_t size)
{
  std::memset(memory, static_cast<int>(value), static_cast<size_t>(size));
}

/******************************************************************************/
void ne_intrinsic_memory_copy(void *destination,
                              const void *source,
                              uint64_t size)
{
  std::memcpy(destination, source, static_cast<size_t>(size));
}

/******************************************************************************/
int64_t ne_intrinsic_memory_compare(const void *a, const void *b, uint64_t size)
{
  return static_cast<int64_t>(std::memcmp(a, b, static_cast<size_t>(size)));
}
