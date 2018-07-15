// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_cpu_supported(ne_result* result);

ne_bool NE_API ne_cpu_is_little_endian(ne_result* result);
uint64_t NE_API ne_cpu_get_cache_line_size(ne_result* result);
uint64_t NE_API ne_cpu_get_logical_count(ne_result* result);
uint64_t NE_API ne_cpu_get_memory_size(ne_result* result);

NE_END
