// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

NE_CORE_BEGIN

#define NE_IO_MAJOR 0
#define NE_IO_MINOR 0

/// Determines if the package is fully supported on this platform.
/// @param result
///   NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
/// @return #NE_CORE_TRUE if supported, #NE_CORE_FALSE otherwise.
NE_CORE_API ne_core_bool (*ne_io_supported)(uint64_t *result);

// Terminal input and output will always be UTF-8, but either may be
// redirected to a non UTF-8 stream, such as a file, socket, etc.

// When using non-blocking read, if the input comes from a terminal:
//  - Pressing enter/return will always result in a single '\r' character.
//  - Characters typed in the terminal will not be visible.
// Must only support: read, free.
NE_CORE_API void (*ne_io_get_input)(uint64_t *result,
                                    ne_core_stream *stream_out);

// Must only support: write, flush, free.
NE_CORE_API void (*ne_io_get_output)(uint64_t *result,
                                     ne_core_stream *stream_out);

// Must only support: write, flush, free.
NE_CORE_API void (*ne_io_get_error)(uint64_t *result,
                                    ne_core_stream *stream_out);

NE_CORE_END
