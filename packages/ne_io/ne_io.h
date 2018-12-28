/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

/// Major version of the library (bumped for breaking changes).
#define NE_IO_MAJOR 0
/// Minor version of the library (bumped for added features).
#define NE_IO_MINOR 0

/// Determines if this package is fully supported on this platform.
/// @param result
///   - #ne_core_tag_routine_results.
/// @return
///   #NE_CORE_TRUE if supported, #NE_CORE_FALSE otherwise.
NE_CORE_API ne_core_bool (*ne_io_supported)(uint64_t *result);

/// The standard input is a stream that can be read from. The data in the stream
/// typically comes from a terminal or is piped from a file, socket,
/// application, or other source. When data is read from a terminal it will
/// always be in UTF-8 format. All other sources will be read directly as
/// binary with no translation.
/// When using non-blocking read, if the input comes from a terminal then
/// pressing enter/return will always result in a single '\\r' character and
/// characters typed in the terminal will not be visible.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param stream_out
///   Outputs the created stream with the following operations:
///   - \ref ne_core_stream.read.
///   - \ref ne_core_stream.free.
NE_CORE_API void (*ne_io_get_input)(uint64_t *result,
                                    ne_core_stream *stream_out);

/// The standard output is a stream that can be written to. The data written to
/// the stream may be shown in a terminal or is piped to a file, socket,
/// application, or other destination. When data is written to a terminal it
/// will always be in UTF-8 format. All other desitinations will be written
/// directly as binary with no translation.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param stream_out
///   Outputs the created stream with the following operations:
///   - \ref ne_core_stream.write.
///   - \ref ne_core_stream.flush.
///   - \ref ne_core_stream.free.
NE_CORE_API void (*ne_io_get_output)(uint64_t *result,
                                     ne_core_stream *stream_out);

/// The standard error is a stream that can be written to and is intended for
/// errors. The data written to the stream may be shown in a terminal or is
/// piped to a file, socket, application, or other destination. When data is
/// written to a terminal it will always be in UTF-8 format. All other
/// desitinations will be written directly as binary with no translation.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param stream_out
///   Outputs the created stream with the following operations:
///   - \ref ne_core_stream.write.
///   - \ref ne_core_stream.flush.
///   - \ref ne_core_stream.free.
NE_CORE_API void (*ne_io_get_error)(uint64_t *result,
                                    ne_core_stream *stream_out);
