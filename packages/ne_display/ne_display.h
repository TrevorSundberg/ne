// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_CORE_BEGIN

NE_CORE_DECLARE_PACKAGE(ne_display, 0, 0);

NE_CORE_API uint64_t (*ne_display_count)(uint64_t *result);

typedef uint64_t ne_display_format;

typedef struct ne_display_mode ne_display_mode;
/// Represents a possible monitor color format, resolution, and refresh rate.
struct ne_display_mode
{
  ne_display_format format;
  uint64_t w;
  uint64_t h;
  uint64_t refresh_rate;
};

NE_CORE_API uint64_t (*ne_display_get_mode)(uint64_t *result,
                                            uint64_t display_index,
                                            ne_display_mode *mode_out);

// Enumerator dereference takes 'ne_display_mode*'.
NE_CORE_API void (*ne_display_mode_enumerator)(
    uint64_t *result,
    uint64_t display_index,
    ne_core_enumerator *enumerator_out);

typedef uint64_t ne_window_flags;
static const ne_window_flags ne_window_flags_visible = 1;
static const ne_window_flags ne_window_flags_borderless = 2;
static const ne_window_flags ne_window_flags_sizable = 4;
static const ne_window_flags ne_window_flags_on_top = 8;
static const ne_window_flags ne_window_flags_on_taskbar = 16;
NE_CORE_API ne_window_flags (*ne_window_get_flags)(uint64_t *result);
NE_CORE_API void (*ne_window_set_flags)(uint64_t *result,
                                        ne_window_flags flags);

typedef uint64_t ne_window_state;
static const ne_window_state ne_window_state_minimized = 0;
static const ne_window_state ne_window_state_maximized = 1;
static const ne_window_state ne_window_state_restored = 2;
static const ne_window_state ne_window_state_fullscreen_true = 3;
static const ne_window_state ne_window_state_fullscreen_display = 3;
NE_CORE_API ne_window_state (*ne_window_get_state)(uint64_t *result);
NE_CORE_API void (*ne_window_set_state)(uint64_t *result,
                                        ne_window_state state);

NE_CORE_END
