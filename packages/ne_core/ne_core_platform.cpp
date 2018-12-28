/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_core/ne_core_platform.hpp"
#include <cstring>

#if defined(NE_CORE_PLATFORM_WINDOWS)
#  define VC_EXTRALEAN
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <Windows.h>
#endif

/******************************************************************************/
_file_opaque::_file_opaque(void *_handle) :
    buffer{0},
    start(0),
    end(0),
    handle(_handle)
{
  std::memset(buffer, 0, sizeof(buffer));
}

/******************************************************************************/
void _file_initialize(ne_core_stream *self, void *handle)
{
  // Note that the handle MAY be invalid, but every function should handle this.
  new (self->opaque) _file_opaque(handle);
}

/******************************************************************************/
uint64_t _file_read(uint64_t *result,
                    ne_core_stream *self,
                    void *buffer,
                    uint64_t size,
                    ne_core_bool allow_blocking)
{
  auto opaque = reinterpret_cast<_file_opaque *>(self->opaque);

#if defined(NE_CORE_PLATFORM_WINDOWS)
  // See http://hackage.haskell.org/package/base-4.6.0.0/src/cbits/inputReady.c
  HANDLE handle = opaque->handle;
  DWORD type = GetFileType(handle);

  uint64_t bytes_read = 0;
  uint8_t *byte_buffer = (uint8_t *)buffer;

  // If we have buffered data to copy then copy it now.
  while (opaque->start != opaque->end && bytes_read < size)
  {
    byte_buffer[bytes_read] = opaque->buffer[opaque->start];
    ++bytes_read;
    ++opaque->start;
  }

  // We need to handle console types separately in Windows because
  // characters are treated as wide characters (we must convert to UTF-8).
  // Asychronous IO (overlapped) does not work with consoles, and they are
  // also / not a named pipe, therefore you cannot use PeekNamedPipe and must
  // use / specific console functions.
  if (type == FILE_TYPE_CHAR)
  {
    // Not quite sure this is even needed since we always get wide character
    // events from the key events and translate them to utf8 manually.
    SetConsoleCP(CP_UTF8);

    while (bytes_read < size)
    {
      // Peek to see if there is any records available.
      INPUT_RECORD record;
      DWORD events_peeked = 0;
      if (!PeekConsoleInputW(handle, &record, 1, &events_peeked))
      {
        NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
        return bytes_read;
      }

      // If we peeked and found nothing...
      if (events_peeked == 0)
      {
        if (allow_blocking)
        {
          // In blocking mode, we wait until something becomes available,
          // and continue to try again until we fill the buffer.
          if (WaitForSingleObject(handle, INFINITE) != WAIT_OBJECT_0)
          {
            NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
            return bytes_read;
          }
          continue;
        }
        else
        {
          // In non-blocking mode, we're done as soon as there is nothing.
          break;
        }
      }
      NE_CORE_ASSERT(events_peeked == 1, "Should have only peeked 1 event");

      // We only care about the record if it's a key event, and if the key is
      // down, and if it was a key that produced a character.
      if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown &&
          record.Event.KeyEvent.uChar.UnicodeChar &&
          record.Event.KeyEvent.uChar.UnicodeChar != '\b')
      {
        WCHAR *wide_char = &record.Event.KeyEvent.uChar.UnicodeChar;

        uint32_t utf8_code_point = 0;
        uint64_t multibyte_size =
            (uint64_t)WideCharToMultiByte(CP_UTF8,
                                          0,
                                          wide_char,
                                          1,
                                          (char *)&utf8_code_point,
                                          sizeof(utf8_code_point),
                                          0,
                                          0);

        // If this character would cause us to read outside the buffer, then we
        // need to only read part of the encoded character and leave the rest
        // for the next read operation.
        uint64_t bytes_remaining = size - bytes_read;
        if (multibyte_size > bytes_remaining)
        {
          uint64_t leftover_size = multibyte_size - bytes_remaining;
          multibyte_size = bytes_remaining;

          // Copy the partial data to our temporary buffer.
          opaque->start = 0;
          opaque->end = (uint8_t)leftover_size;
          std::memcpy(opaque->buffer,
                      (uint8_t *)&utf8_code_point + multibyte_size,
                      (size_t)leftover_size);
        }

        // Copy the code-point to the output buffer.
        std::memcpy((uint8_t *)buffer + bytes_read,
                    &utf8_code_point,
                    (size_t)multibyte_size);
        bytes_read += multibyte_size;
      }

      DWORD events_read = 0;
      if (!ReadConsoleInputW(handle, &record, 1, &events_read))
      {
        NE_CORE_ERROR(
            "If we peeked the input, we should have been able to read it");
        NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
        return 0;
      }

      NE_CORE_ASSERT(events_read == 1, "Should have only read 1 event");
    }

    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
    return bytes_read;
  }
  else
  {
    // Unlike console, the following handle types follow the same read pattern,
    // but with slightly different ways of controlling blocking behavior.

    if (type == FILE_TYPE_UNKNOWN)
    {
      COMMTIMEOUTS timeouts;
      if (allow_blocking)
      {
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = MAXDWORD;
        timeouts.WriteTotalTimeoutMultiplier = MAXDWORD;
        timeouts.WriteTotalTimeoutConstant = MAXDWORD;
      }
      else
      {
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 0;
      }

      // We purposefully ignore the return value because this is an unknown type
      // of stream, meaning it could NOT be a COM port.
      SetCommTimeouts(handle, &timeouts);
    }
    else if (type == FILE_TYPE_PIPE)
    {
      DWORD mode =
          PIPE_READMODE_BYTE | (allow_blocking ? PIPE_WAIT : PIPE_NOWAIT);
      if (SetNamedPipeHandleState(handle, &mode, 0, 0) == 0)
      {
        NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
        return 0;
      }
    }

    // For FILE_TYPE_DISK we don't need to do anything since it doesn't block.
    DWORD amount = 0;
    if (ReadFile(handle, buffer, (DWORD)size, &amount, nullptr))
    {
      NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
      return amount;
    }
    else
    {
      NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
      return 0;
    }
  }
#else
  (void)opaque;
  (void)buffer;
  (void)size;
  (void)allow_blocking;
  NE_CORE_INTERNAL_ERROR_RESULT_RETURN(0);
#endif
}

/******************************************************************************/
extern uint64_t _file_write(uint64_t *result,
                            ne_core_stream *self,
                            const void *buffer,
                            uint64_t size,
                            ne_core_bool allow_blocking)
{
  auto opaque = reinterpret_cast<_file_opaque *>(self->opaque);

#if defined(NE_CORE_PLATFORM_WINDOWS)
  // NOTE: We NEED to handle blocking here. This won't happen for a file or
  // console, however it may happen for a COM port or a pipe (especially if the
  // end of the pipe isn't reading).
  HANDLE handle = opaque->handle;
  DWORD type = GetFileType(handle);

  if (type == FILE_TYPE_CHAR)
  {
    SetConsoleCP(CP_UTF8);
  }
  else if (type == FILE_TYPE_UNKNOWN)
  {
    COMMTIMEOUTS timeouts;
    if (allow_blocking)
    {
      timeouts.ReadIntervalTimeout = MAXDWORD;
      timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
      timeouts.ReadTotalTimeoutConstant = MAXDWORD;
      timeouts.WriteTotalTimeoutMultiplier = MAXDWORD;
      timeouts.WriteTotalTimeoutConstant = MAXDWORD;
    }
    else
    {
      timeouts.ReadIntervalTimeout = MAXDWORD;
      timeouts.ReadTotalTimeoutMultiplier = 0;
      timeouts.ReadTotalTimeoutConstant = 0;
      timeouts.WriteTotalTimeoutMultiplier = 0;
      timeouts.WriteTotalTimeoutConstant = 0;
    }

    // We purposefully ignore the return value because this is an unknown type
    // of stream, meaning it could NOT be a COM port.
    SetCommTimeouts(handle, &timeouts);
  }
  else if (type == FILE_TYPE_PIPE)
  {
    DWORD mode =
        PIPE_READMODE_BYTE | (allow_blocking ? PIPE_WAIT : PIPE_NOWAIT);
    if (SetNamedPipeHandleState(handle, &mode, 0, 0) == 0)
    {
      NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
      return 0;
    }
  }

  DWORD amount = 0;
  if (WriteFile(handle, buffer, (DWORD)size, &amount, nullptr))
  {
    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
    return amount;
  }
  else
  {
    NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
    return 0;
  }
#else
  (void)opaque;
  (void)buffer;
  (void)size;
  (void)allow_blocking;
  NE_CORE_INTERNAL_ERROR_RESULT_RETURN(0);
#endif
}

/******************************************************************************/
void _file_flush(uint64_t *result, ne_core_stream *self)
{
  auto opaque = reinterpret_cast<_file_opaque *>(self->opaque);

#if defined(NE_CORE_PLATFORM_WINDOWS)
  HANDLE handle = opaque->handle;

  // Console is not buffered (see FlushFileBuffers docs).
  if (GetFileType(handle) == FILE_TYPE_CHAR)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
    return;
  }

  if (FlushFileBuffers(handle) != 0)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  }
  else
  {
    NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
  }
#else
  (void)opaque;
  NE_CORE_INTERNAL_ERROR_RESULT();
#endif
}

/******************************************************************************/
uint64_t _file_get_position(uint64_t *result, const ne_core_stream *self)
{
  auto stream = const_cast<ne_core_stream *>(self);

  // On both POSIX and Windows, there is no 'tell' type function.
  return _file_seek(result, stream, ne_core_stream_seek_origin_current, 0);
}

/******************************************************************************/
uint64_t _file_get_size(uint64_t *result, const ne_core_stream *self)
{
#if defined(NE_CORE_PLATFORM_WINDOWS)
  auto opaque = reinterpret_cast<const _file_opaque *>(self->opaque);
  HANDLE handle = opaque->handle;

  LARGE_INTEGER size;
  size.QuadPart = 0;

  if (GetFileSizeEx(handle, &size) != 0)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
    return static_cast<uint64_t>(size.QuadPart);
  }
  else
  {
    NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
    return 0;
  }
#else
  // We effectively treat the stream as const by returning it back to its state.
  auto stream = const_cast<ne_core_stream *>(self);

  // On POSIX systems we may attempt to use fstat instead.

  // If any error occurs in the middle after we moved a position, it would be a
  // violation of the 'no side effects' when an error occurs. However, the most
  // likely cause for an error is a handle being closed due to a deleted file or
  // other external event, in which seeking would become impossible and there
  // would be no observable side effects.

  uint64_t passed_result = NE_CORE_RESULT_INVALID;
  uint64_t position = _file_get_position(&passed_result, stream);
  if (passed_result != NE_CORE_RESULT_SUCCESS)
  {
    NE_CORE_RESULT(passed_result);
    return 0;
  }

  uint64_t size =
      _file_seek(&passed_result, stream, ne_core_stream_seek_origin_end, 0);
  if (passed_result != NE_CORE_RESULT_SUCCESS)
  {
    NE_CORE_RESULT(passed_result);
    return 0;
  }

  uint64_t original_position = _file_seek(&passed_result,
                                          stream,
                                          ne_core_stream_seek_origin_begin,
                                          static_cast<int64_t>(position));
  if (passed_result != NE_CORE_RESULT_SUCCESS)
  {
    NE_CORE_RESULT(passed_result);
    return 0;
  }
  if (original_position != position)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
    return 0;
  }
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  return size;
#endif
}

/******************************************************************************/
uint64_t _file_seek(uint64_t *result,
                    ne_core_stream *self,
                    ne_core_stream_seek_origin origin,
                    int64_t position)
{
  auto opaque = reinterpret_cast<const _file_opaque *>(self->opaque);

#if defined(NE_CORE_PLATFORM_WINDOWS)
  HANDLE handle = opaque->handle;

  LARGE_INTEGER delta;
  delta.QuadPart = static_cast<LONGLONG>(position);

  LARGE_INTEGER large_position;
  large_position.QuadPart = 0;

  DWORD move_method = 0;
  switch (origin)
  {
  case ne_core_stream_seek_origin_begin:
    move_method = FILE_BEGIN;
    break;
  case ne_core_stream_seek_origin_current:
    move_method = FILE_CURRENT;
    break;
  case ne_core_stream_seek_origin_end:
    move_method = FILE_END;
    break;
  case ne_core_stream_seek_origin_max:
  case ne_core_stream_seek_origin_force_size:
  default:
    NE_CORE_RESULT(NE_CORE_RESULT_INVALID_PARAMETER);
    return 0;
  }

  if (SetFilePointerEx(handle, delta, &large_position, move_method) != 0)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
    return static_cast<uint64_t>(large_position.QuadPart);
  }
  else
  {
    switch (GetLastError())
    {
    case ERROR_NEGATIVE_SEEK:
      NE_CORE_RESULT(NE_CORE_RESULT_STREAM_OUT_OF_BOUNDS);
      break;
    default:
      NE_CORE_RESULT(NE_CORE_RESULT_STREAM_ERROR);
      break;
    }
    return 0;
  }
#else
  (void)opaque;
  (void)origin;
  (void)position;
  NE_CORE_INTERNAL_ERROR_RESULT_RETURN(0);
#endif
}

/******************************************************************************/
ne_core_bool _file_is_valid(uint64_t *result, const ne_core_stream *self)
{
  auto opaque = reinterpret_cast<const _file_opaque *>(self->opaque);

#if defined(NE_CORE_PLATFORM_WINDOWS)
  HANDLE handle = opaque->handle;
  DWORD flags = 0;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  return GetHandleInformation(handle, &flags) != 0;
#else
  (void)opaque;
  NE_CORE_INTERNAL_ERROR_RESULT_RETURN(NE_CORE_FALSE);
#endif
}

/******************************************************************************/
void _file_free(uint64_t *result, ne_core_stream *self)
{
  auto opaque = reinterpret_cast<_file_opaque *>(self->opaque);

#if defined(NE_CORE_PLATFORM_WINDOWS)
  HANDLE handle = opaque->handle;
  CloseHandle(handle);
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
#else
  (void)opaque;
  NE_CORE_INTERNAL_ERROR_RESULT();
#endif
  opaque->~_file_opaque();
}