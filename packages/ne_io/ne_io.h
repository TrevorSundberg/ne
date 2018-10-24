// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_io, 0, 0);

// Note that console input and output will always be UTF-8, but either may be
// redirected to a non UTF-8 stream, such as a file, socket, etc.

// When using non-blocking reading, if the input comes from a console/terminal then
// pressing enter/return will always result in a single '\r' character.
// Must only support: read, get_position, get_length, is_eof.
NE_API void (*ne_io_get_input)(uint64_t *result, ne_core_stream *stream_out);

// Must only support: write, flush, get_position, get_length.
NE_API void (*ne_io_get_output)(uint64_t *result, ne_core_stream *stream_out);

NE_END
