// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_timer_supported(ne_result* result);

// A platform may support multiple types of timers, ordered from highest to lowest frequency.
// The minimum frequency is a timer that increments every second.
// The value returned will always be at least 1.
uint64_t NE_API ne_timer_count(ne_result* result);

// Get how many ticks have passed for a given timer.
// At the start of the application the ticks will be will be 0.
// The valid timer index is from [0, ne_timer_count), ordered from highest to lowest frequency.
uint64_t NE_API ne_timer_ticks(ne_result* result, uint64_t timer);

// Get the frequency of a specific timer (how many ticks per second, non-zero).
// The valid timer index is from [0, ne_timer_count), ordered from highest to lowest frequency.
uint64_t NE_API ne_timer_frequency(ne_result* result, uint64_t timer);

NE_END
