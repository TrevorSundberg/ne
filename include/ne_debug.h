// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_debug_supported(ne_result* result);

// Attempt to trigger breakpoint with the sandbox application.
// The purpose of this to allow code to be debugged at a certain point.
// If a debugger is not attached or cannot attach on the fly for the given platform
// then the sandbox may log information and continue execution.
void NE_API ne_debug_break(ne_result* result);

typedef uint64_t ne_debug_memory_access;
static const ne_debug_memory_access ne_debug_memory_access_invalid = 0;
static const ne_debug_memory_access ne_debug_memory_access_read = 1;
static const ne_debug_memory_access ne_debug_memory_access_write = 2;
static const ne_debug_memory_access ne_debug_memory_access_readwrite = 3;
static const ne_debug_memory_access ne_debug_memory_access_count = 4;
ne_debug_memory_access NE_API ne_debug_memory_query(const void* memory);

NE_END
