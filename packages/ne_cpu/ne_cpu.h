// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_CORE_BEGIN

NE_CORE_DECLARE_PACKAGE(ne_cpu, 0, 0);

NE_CORE_API ne_core_bool (*ne_cpu_is_little_endian)(uint64_t *result);
NE_CORE_API uint64_t (*ne_cpu_get_cache_line_size)(uint64_t *result);
NE_CORE_API uint64_t (*ne_cpu_get_logical_count)(uint64_t *result);
NE_CORE_API uint64_t (*ne_cpu_get_memory_size)(uint64_t *result);

NE_CORE_END
