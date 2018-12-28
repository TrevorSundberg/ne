/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

/// Major version of the library (bumped for breaking changes).
#define NE_TIME_MAJOR 0
/// Minor version of the library (bumped for added features).
#define NE_TIME_MINOR 0

/// Determines if this package is fully supported on this platform.
/// @param result
///   - #ne_core_tag_routine_results.
/// @return
///   #NE_CORE_TRUE if supported, #NE_CORE_FALSE otherwise.
NE_CORE_API ne_core_bool (*ne_time_supported)(uint64_t *result);

/// Represents the system time in nanoseconds that may change due to external
/// events such as a user adjusting the system clock (non-monotonic). The time
/// is relative to the UNIX epoch which began on the 1st of January, 1970 at
/// 00:00:00 UTC (not accounting for leap seconds). A 64-bit representation in
/// nanoseconds will yeild a time period of ~584 years which limits the lifetime
/// of this function to the year 2554.
/// @param result
///   - #ne_core_tag_routine_results.
/// @return
///   The number of nanoseconds since the UNIX epoch.
NE_CORE_API uint64_t (*ne_time_system)(uint64_t *result);

/// Represents the highest frequency monotonic time in nanoseconds that
/// may NOT change due to external events (must never decrease). The initial
/// value of the timer is not guaranteed, as this timer is primarily to be used
/// for durations.
/// @param result
///   - #ne_core_tag_routine_results.
/// @return
///   A number of nanoseconds.
NE_CORE_API uint64_t (*ne_time_high_frequency_monotonic)(uint64_t *result);
