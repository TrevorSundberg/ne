// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_mouse_supported(ne_result* result);

ne_bool NE_API ne_mouse_request_permission(ne_result* result);

ne_bool NE_API ne_mouse_is_touch_emulated(ne_result* result);

typedef uint64_t ne_mouse_index;
static const ne_mouse_index ne_mouse_index_left = 0;
static const ne_mouse_index ne_mouse_index_right = 1;
static const ne_mouse_index ne_mouse_index_middle = 2;
static const ne_mouse_index ne_mouse_index_x1_back = 3;
static const ne_mouse_index ne_mouse_index_x2_forward = 4;
static const ne_mouse_index ne_mouse_index_count = 5;

// Returns the state of a mouse button at the exact time of call.
// Note that events don't need to be pumped for this value to change.
// Returning 'ne_true' means the button is down and 'ne_false' means up.
// This represents the physical mouse, which means it may change
// independent of any application focus.
ne_bool NE_API ne_mouse_is_down(ne_result* result, ne_keyboard_index index);

ne_int64_2 NE_API ne_mouse_get_position_screen(ne_result* result);

NE_END
