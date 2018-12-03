// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

NE_CORE_DECLARE_PACKAGE(ne_debug, 0, 0);

// Attempt to trigger breakpoint with the sandbox application.
// The purpose of this to allow code to be debugged at a certain point.
// If a debugger is not attached or cannot attach on the fly for the given
// platform then the sandbox may log information and continue execution.
NE_CORE_API void (*ne_debug_break)(uint64_t *result);

typedef uint64_t ne_debug_memory_access;
static const ne_debug_memory_access ne_debug_memory_access_invalid = 0;
static const ne_debug_memory_access ne_debug_memory_access_read = 1;
static const ne_debug_memory_access ne_debug_memory_access_write = 2;
static const ne_debug_memory_access ne_debug_memory_access_readwrite = 3;
static const ne_debug_memory_access ne_debug_memory_access_count = 4;
NE_CORE_API ne_debug_memory_access (*ne_debug_memory_query)(const void *memory);
