// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_clipboard_supported(ne_result* result);

ne_bool NE_API ne_clipboard_request_permission(ne_result* result);

static const ne_result ne_result_clipboard_texttype_not_supported = 0;

// Common text types that should be handled by this every platform.
// If a particular text type is not supported it should return null.
static const char* const ne_clipboard_texttype_plain = "plain";
static const char* const ne_clipboard_texttype_rtf = "rtf";
static const char* const ne_clipboard_texttype_html = "html";
static const char* const ne_clipboard_texttype_url = "url";

// Each file or directory name will be separated by a single '\n' (no '\r' on any platform).
static const char* const ne_clipboard_texttype_files = "files";

// Returns null if the clipboard does not store this text type. Memory is *platform-owned*.
// May result in 'ne_result_clipboard_texttype_not_supported'.
const char* NE_API ne_clipboard_get_text(ne_result* result, const char* texttype);

// May result in 'ne_result_clipboard_texttype_not_supported'.
void NE_API ne_clipboard_set_text(ne_result* result, const char* text, const char* texttype);

// Grab an image off the clipboard.
void NE_API ne_clipboard_get_image_rgba32f(ne_result* result, ne_image* image);

void NE_API ne_clipboard_set_image_rgba32f(ne_result* result, ne_image* image);

NE_END
