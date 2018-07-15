// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_touch_supported(ne_result* result);

struct ne_touch_finger
{
  ne_int64_2 position_screen;
  uint64_t pressure;
};
typedef struct ne_touch_finger ne_touch_finger;

ne_bool NE_API ne_touch_is_mouse_emulated(ne_result* result);

uint64_t NE_API ne_touch_device_count(ne_result* result);

uint64_t NE_API ne_touch_finger_count(ne_result* result, uint64_t device);

void NE_API ne_touch_get_finger(ne_result* result, uint64_t device, uint64_t finger, ne_touch_finger* finger_out);

NE_END
