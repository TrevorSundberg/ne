// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

NE_CORE_DECLARE_PACKAGE(ne_mouse, 0, 0);

NE_CORE_API ne_core_bool (*ne_mouse_is_touch_emulated)(uint64_t *result);

typedef uint64_t ne_mouse_index;
static const ne_mouse_index ne_mouse_index_left = 0;
static const ne_mouse_index ne_mouse_index_right = 1;
static const ne_mouse_index ne_mouse_index_middle = 2;
static const ne_mouse_index ne_mouse_index_x1_back = 3;
static const ne_mouse_index ne_mouse_index_x2_forward = 4;
static const ne_mouse_index ne_mouse_index_count = 5;

// Returns the state of a mouse button at the exact time of call.
// Note that events don't need to be pumped for this value to change.
// Returning 'ne_core_true' means the button is down and 'ne_core_false' means
// up. This represents the physical mouse, which means it may change independent
// of any application focus.
NE_CORE_API ne_core_bool (*ne_mouse_is_down)(uint64_t *result,
                                             ne_mouse_index index);

NE_CORE_API void (*ne_mouse_get_position_screen)(uint64_t *result,
                                                 int64_t *screen_x_out,
                                                 int64_t *screen_y_out);
