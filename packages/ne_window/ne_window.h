// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_CORE_BEGIN

NE_CORE_DECLARE_PACKAGE(ne_window, 0, 0);

typedef struct ne_window ne_window;

typedef struct ne_window_rectangle ne_window_rectangle;
/// A rectangle where the origin is the top left corner.
struct ne_window_rectangle
{
  int64_t x;
  int64_t y;
  int64_t w;
  int64_t h;
};

// Creates a window. The window will not be visible until flags are set that
// show the window.
// This gives the user an opportunity to move, size, and style the window as
// they need before showing it.
// When created a window's position is 0,0 and the size is 0,0 until positioned.
NE_CORE_API ne_window *(*ne_window_create)(uint64_t *result);

NE_CORE_API void (*ne_window_free)(uint64_t *result, ne_window *window);

// Returns the rectangle of the window's border in display space (origin is top
// left). If the window is borderless then this will return the client area
// rectangle.
NE_CORE_API void (*ne_window_get_bordered_rectangle)(
    uint64_t *result, ne_window_rectangle *rectangle_out);
NE_CORE_API void (*ne_window_set_bordered_rectangle)(
    uint64_t *result, ne_window_rectangle *rectangle);

// Returns the rectangle of the window's client-area in display space (origin is
// top left).
NE_CORE_API void (*ne_window_get_client_rectangle)(
    uint64_t *result, ne_window_rectangle *rectangle_out);
NE_CORE_API void (*ne_window_set_client_rectangle)(
    uint64_t *result, ne_window_rectangle *rectangle);

// Set the minimum size of the client area.
NE_CORE_API void (*ne_window_get_minimum_client_size)(uint64_t *result,
                                                      uint64_t *x_out,
                                                      uint64_t *y_out);
NE_CORE_API void (*ne_window_set_minimum_client_size)(uint64_t *result,
                                                      uint64_t x,
                                                      uint64_t y);

// Set the maximum size of the client area.
NE_CORE_API void (*ne_window_get_maximum_client_size)(uint64_t *result,
                                                      uint64_t *x_out,
                                                      uint64_t *y_out);
NE_CORE_API void (*ne_window_set_maximum_client_size)(uint64_t *result,
                                                      uint64_t x,
                                                      uint64_t y);

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

// Get an index for the display that primarily owns the window.
NE_CORE_API uint64_t (*ne_window_display)(uint64_t *result,
                                          ne_window_state state);

NE_CORE_API const char *(*ne_window_get_title)(uint64_t *result);
NE_CORE_API void (*ne_window_set_title)(uint64_t *result, const char *title);

NE_CORE_API float (*ne_window_get_opacity)(uint64_t *result);
NE_CORE_API void (*ne_window_set_opacity)(uint64_t *result, float opacity);

NE_CORE_API float (*ne_window_get_progress)(uint64_t *result);
NE_CORE_API void (*ne_window_set_progress)(uint64_t *result, float progress);

NE_CORE_END
