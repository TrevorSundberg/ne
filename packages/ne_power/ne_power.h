// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_power, 0, 0);

typedef uint64_t ne_power_state;
static const ne_power_state ne_power_state_powered = 0;
static const ne_power_state ne_power_state_draining = 1;
static const ne_power_state ne_power_state_charging = 2;
static const ne_power_state ne_power_state_charged = 3;
static const ne_power_state ne_power_state_count = 4;

NE_API ne_power_state (*ne_power_get_state)(uint64_t *result, int64_t *seconds,
                                            int64_t *percent);

NE_END
