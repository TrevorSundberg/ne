/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_io/ne_io.h"
#include "../ne_core/ne_core_platform.hpp"
#include "../ne_core/ne_core_private.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

#if defined(NE_CORE_PLATFORM_WINDOWS)
#  define VC_EXTRALEAN
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <Windows.h>

static const constexpr bool _supported = true;
#else
static const constexpr bool _supported = false;
#endif

/******************************************************************************/
static ne_core_bool _ne_io_supported(uint64_t *result)
{
  NE_CORE_SUPPORTED_IMPLEMENTATION(_supported);
}
ne_core_bool (*ne_io_supported)(uint64_t *result) = &_ne_io_supported;

/******************************************************************************/
static void _ne_io_get_input(uint64_t *result, ne_core_stream *stream_out)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  std::memset(stream_out, 0, sizeof(*stream_out));
  _file_initialize(
      stream_out,
      NE_CORE_PLATFORM_IF_WINDOWS(GetStdHandle(STD_INPUT_HANDLE), nullptr));
  stream_out->read = &_file_read;
  stream_out->free = &_file_free;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_io_get_input)(uint64_t *result,
                        ne_core_stream *stream_out) = &_ne_io_get_input;

/******************************************************************************/
static void _ne_io_get_output(uint64_t *result, ne_core_stream *stream_out)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  std::memset(stream_out, 0, sizeof(*stream_out));
  _file_initialize(
      stream_out,
      NE_CORE_PLATFORM_IF_WINDOWS(GetStdHandle(STD_OUTPUT_HANDLE), nullptr));
  stream_out->write = &_file_write;
  stream_out->flush = &_file_flush;
  stream_out->free = &_file_free;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_io_get_output)(uint64_t *result,
                         ne_core_stream *stream_out) = &_ne_io_get_output;

/******************************************************************************/
static void _ne_io_get_error(uint64_t *result, ne_core_stream *stream_out)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  std::memset(stream_out, 0, sizeof(*stream_out));
  _file_initialize(
      stream_out,
      NE_CORE_PLATFORM_IF_WINDOWS(GetStdHandle(STD_ERROR_HANDLE), nullptr));
  stream_out->write = &_file_write;
  stream_out->flush = &_file_flush;
  stream_out->free = &_file_free;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_io_get_error)(uint64_t *result,
                        ne_core_stream *stream_out) = &_ne_io_get_error;
