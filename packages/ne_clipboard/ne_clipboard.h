// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_CORE_BEGIN

NE_CORE_DECLARE_PACKAGE(ne_clipboard, 0, 0);

static const uint64_t ne_clipboard_result_texttype_not_supported = 0;

typedef uint64_t ne_clipboard_texttype;
static const ne_clipboard_texttype ne_clipboard_texttype_plain = 0;
static const ne_clipboard_texttype ne_clipboard_texttype_html = 1;
static const ne_clipboard_texttype ne_clipboard_texttype_rtf = 2;

// Each file or directory name will be separated by a single '\n' (no '\r' on
// any platform).
static const ne_clipboard_texttype ne_clipboard_texttype_files = 3;

// Returns null if the clipboard does not store this text type. Memory is
// *platform-owned*. May result in 'ne_clipboard_result_texttype_not_supported'.
NE_CORE_API const char *(*ne_clipboard_get_text)(
    uint64_t *result, ne_clipboard_texttype texttype);

// May result in 'ne_clipboard_result_texttype_not_supported'.
NE_CORE_API void (*ne_clipboard_set_text)(uint64_t *result, const char *text,
                                          ne_clipboard_texttype texttype);

struct ne_clipboard_image {
  uint8_t *buffer;
  uint64_t size_x;
  uint64_t size_y;
};
typedef struct ne_clipboard_image ne_clipboard_image;

// Grab an image off the clipboard. We use the most uncompressed
// format available with the highest bit depth and supports HDR.
NE_CORE_API void (*ne_clipboard_get_image_rgba32f)(uint64_t *result,
                                                   ne_clipboard_image *image);

NE_CORE_API void (*ne_clipboard_set_image_rgba32f)(uint64_t *result,
                                                   ne_clipboard_image *image);

NE_CORE_END
