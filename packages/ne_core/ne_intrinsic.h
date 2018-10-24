// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

NE_CORE_BEGIN

// Intrinsic functions are not treated as function pointers because they are
// expected to be implemented directly by the compiler and heavily optimized.

NE_CORE_API void ne_intrinsic_memory_set(void *memory, uint8_t value,
                                         uint64_t size);

NE_CORE_API int64_t ne_intrinsic_memory_compare(void *a, void *b,
                                                uint64_t size);

NE_CORE_END
