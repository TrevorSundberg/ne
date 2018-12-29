/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
#include "../ne_core/ne_core_private.h"

// All exported symbols in this header are prefixed with _ because they should
// not be linked against by any user packages. This header contains any platform
// specific implementations that can be shared accross all ne packages. This
// header is explicitly C++. We still use a private implementation approach here
// to shield every library from including swaths of platform specific headers.

#if !defined(NE_CORE_PLATFORM_NE)
struct _file_opaque
{
  _file_opaque(void *_handle);

  uint8_t buffer[4];
  uint8_t start;
  uint8_t end;

  // HANDLE on Windows, fd on Posix.
  void *handle;
};
static_assert(sizeof(_file_opaque) <= NE_CORE_OPAQUE_SIZE,
              "Struct must be small enough to fit in opaque data");

///   $ #read.
///   $ #write.
///   $ #flush.
///   - #get_position.
///   - #get_size.
///   - #seek.
///   - #is_valid.
///   $ #free.

extern void _file_initialize(ne_core_stream *self, void *handle);

extern uint64_t _file_read(uint64_t *result,
                           ne_core_stream *self,
                           void *buffer,
                           uint64_t size,
                           ne_core_bool allow_blocking);

extern uint64_t _file_write(uint64_t *result,
                            ne_core_stream *self,
                            const void *buffer,
                            uint64_t size,
                            ne_core_bool allow_blocking);

extern void _file_flush(uint64_t *result, ne_core_stream *self);

extern uint64_t _file_get_position(uint64_t *result,
                                   const ne_core_stream *self);

extern uint64_t _file_get_size(uint64_t *result, const ne_core_stream *self);

extern uint64_t _file_seek(uint64_t *result,
                           ne_core_stream *self,
                           ne_core_stream_seek_origin origin,
                           int64_t position);

extern ne_core_bool _file_is_valid(uint64_t *result,
                                   const ne_core_stream *self);

extern void _file_free(uint64_t *result, ne_core_stream *self);
#endif
