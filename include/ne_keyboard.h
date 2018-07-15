// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_keyboard_supported(ne_result* result);

ne_bool NE_API ne_keyboard_request_permission(ne_result* result);

typedef uint64_t ne_keyboard_index;
static const ne_keyboard_index ne_keyboard_index_a = 0;
static const ne_keyboard_index ne_keyboard_index_b = 1;
static const ne_keyboard_index ne_keyboard_index_c = 2;
static const ne_keyboard_index ne_keyboard_index_count = 3;

// Returns the state of a keyboard key at the exact time of call.
// Note that events don't need to be pumped for this value to change.
// Returning 'ne_true' means the key is down and 'ne_false' means up.
// This represents the physical keyboard, which means it may change
// independent of any application focus.
ne_bool NE_API ne_keyboard_is_down(ne_result* result, ne_keyboard_index index);

NE_END
