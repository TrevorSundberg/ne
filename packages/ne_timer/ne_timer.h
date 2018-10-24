// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_timer, 0, 0);

// A platform may support multiple types of timers, ordered from highest to
// lowest frequency. The minimum frequency is a timer that increments every
// second. The value returned will always be at least 1. Since you are
// guaranteed at least 1 timer, index 0 is always valid and has the highest
// frequency.
NE_API uint64_t (*ne_timer_count)(uint64_t *result);

// Get how many ticks have passed for a given timer.
// At the start of the application the ticks will be will be 0.
// The valid timer index is from [0, ne_timer_count), ordered from highest to
// lowest frequency.
//    ne_core_result_invalid_parameter:
//      The timer provided was greater or equal to the value returned from
//      'ne_timer_count'.
NE_API uint64_t (*ne_timer_ticks)(uint64_t *result, uint64_t timer);

// Get the frequency of a specific timer (how many ticks per second, non-zero).
// The valid timer index is from [0, ne_timer_count), ordered from highest to
// lowest frequency.
//    ne_core_result_invalid_parameter:
//      The timer provided was greater or equal to the value returned from
//      'ne_timer_count'.
NE_API uint64_t (*ne_timer_frequency)(uint64_t *result, uint64_t timer);

NE_END
