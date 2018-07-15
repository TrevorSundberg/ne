// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_memory_supported(ne_result* result);

static const ne_result ne_result_memory_allocation_failed = 0;

//    ne_result_memory_allocation_failed:
//      Not enough system memory or address space, or other system error.
uint8_t* NE_API ne_memory_allocate(ne_result* result, uint64_t sizeBytes);

//    ne_result_core_invalid_parameter:
//      The exact memory pointer in was not returned from 'ne_memory_allocate'.
void NE_API ne_memory_free(ne_result* result, void* memory);

NE_END
