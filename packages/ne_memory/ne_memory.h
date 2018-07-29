// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_memory, 0, 0);

static const uint64_t ne_memory_result_allocation_failed = 0;

//    ne_memory_result_allocation_failed:
//      Not enough system memory or address space, or other system error.
NE_API uint8_t* (*ne_memory_allocate)(uint64_t* result, uint64_t sizeBytes);

// Frees the memory (only should be memory returned from 'ne_memory_allocate').
NE_API void (*ne_memory_free)(uint64_t* result, void* memory);

NE_END
