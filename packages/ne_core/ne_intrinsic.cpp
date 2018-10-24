// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_core/ne_intrinsic.h"
#include <string.h>

NE_CORE_BEGIN

/******************************************************************************/
void ne_intrinsic_memory_set(void *memory, uint8_t value, uint64_t size) {
  memset(memory, (int)value, (size_t)size);
}

/******************************************************************************/
int64_t ne_intrinsic_memory_compare(void *a, void *b, uint64_t size) {
  return (int64_t)memcmp(a, b, (size_t)size);
}

NE_CORE_END
