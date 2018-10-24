// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_CORE_BEGIN

NE_CORE_DECLARE_PACKAGE(ne_touch, 0, 0);

struct ne_touch_finger {
  int64_t screen_x;
  int64_t screen_y;
  uint64_t pressure;
};
typedef struct ne_touch_finger ne_touch_finger;

NE_CORE_API ne_core_bool (*ne_touch_is_mouse_emulated)(uint64_t *result);

NE_CORE_API uint64_t (*ne_touch_device_count)(uint64_t *result);

NE_CORE_API uint64_t (*ne_touch_finger_count)(uint64_t *result, uint64_t device);

NE_CORE_API void (*ne_touch_get_finger)(uint64_t *result, uint64_t device,
                                   uint64_t finger,
                                   ne_touch_finger *finger_out);

NE_CORE_END
