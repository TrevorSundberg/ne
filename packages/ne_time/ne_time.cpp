/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_time/ne_time.h"
#include "../ne_core/ne_core_private.h"
#include <chrono>
#include <type_traits>

static const constexpr bool _supported = true;

/******************************************************************************/
static ne_core_bool _ne_time_supported(uint64_t *result)
{
  NE_CORE_SUPPORTED_IMPLEMENTATION(_supported);
}
ne_core_bool (*ne_time_supported)(uint64_t *result) = &_ne_time_supported;

/******************************************************************************/
static uint64_t _ne_time_system(uint64_t *result)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, 0);
  auto now = std::chrono::system_clock::now();
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          now.time_since_epoch())
          .count());
}
uint64_t (*ne_time_system)(uint64_t *result) = &_ne_time_system;

/******************************************************************************/
static uint64_t _ne_time_high_frequency_monotonic(uint64_t *result)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, 0);
  typedef
      typename std::conditional<std::chrono::high_resolution_clock::is_steady,
                                std::chrono::high_resolution_clock,
                                std::chrono::steady_clock>::type clock_type;
  auto now = clock_type::now();
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          now.time_since_epoch())
          .count());
}
uint64_t (*ne_time_high_frequency_monotonic)(uint64_t *result) =
    &_ne_time_high_frequency_monotonic;
