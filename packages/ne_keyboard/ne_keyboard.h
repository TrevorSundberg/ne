// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

NE_CORE_DECLARE_PACKAGE(ne_keyboard, 0, 0);

typedef uint64_t ne_keyboard_index;
static const ne_keyboard_index ne_keyboard_index_a = 0;
static const ne_keyboard_index ne_keyboard_index_b = 1;
static const ne_keyboard_index ne_keyboard_index_c = 2;
static const ne_keyboard_index ne_keyboard_index_count = 3;

// Returns the state of a keyboard key at the exact time of call.
// Note that events don't need to be pumped for this value to change.
// Returning 'ne_core_true' means the key is down and 'ne_core_false' means up.
// This represents the physical keyboard, which means it may change
// independent of any application focus.
NE_CORE_API ne_core_bool (*ne_keyboard_is_down)(uint64_t *result,
                                                ne_keyboard_index index);
