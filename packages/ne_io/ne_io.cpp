/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_io/ne_io.h"
#include <cstdio>
#include <cstring>

#if defined(NE_CORE_PLATFORM_WINDOWS)
#define NO_WIN #@ MIN_MAX
#define NOMINMAX
#include <Windows.h>
#include <algorithm>

struct _input_opaque
{
  uint8_t start;
  uint8_t end;
  uint8_t buffer[4];
};
#define NE_IO_OUTPUT STD_OUTPUT_HANDLE
#define NE_IO_ERROR STD_ERROR_HANDLE
static const constexpr bool _supported = true;
#else
#define NE_IO_OUTPUT 0
#define NE_IO_ERROR 0
static const constexpr bool _supported = false;
#endif

/******************************************************************************/
static ne_core_bool _ne_io_supported(uint64_t *result)
{
  NE_CORE_SUPPORTED_IMPLEMENTATION(_supported);
}
ne_core_bool (*ne_io_supported)(uint64_t *result) = &_ne_io_supported;

/******************************************************************************/
static uint64_t _input_read(uint64_t *result,
                            ne_core_stream *stream,
                            void *buffer,
                            uint64_t size,
                            ne_core_bool allow_blocking)
{
#if defined(NE_CORE_PLATFORM_WINDOWS)
  // See http://hackage.haskell.org/package/base-4.6.0.0/src/cbits/inputReady.c

  _input_opaque *opaque = (_input_opaque *)stream->opaque;
  HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
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
        NE_CORE_RESULT(NE_CORE_RESULT_ERROR);
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
            NE_CORE_RESULT(NE_CORE_RESULT_ERROR);
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
        uint64_t multibyte_size = (uint64_t)WideCharToMultiByte(
            CP_UTF8, 0, wide_char, 1, (char *)&utf8_code_point,
            sizeof(utf8_code_point), 0, 0);

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
        std::memcpy((uint8_t *)buffer + bytes_read, &utf8_code_point,
                    (size_t)multibyte_size);
        bytes_read += multibyte_size;
      }

      DWORD events_read = 0;
      if (!ReadConsoleInputW(handle, &record, 1, &events_read))
      {
        NE_CORE_ERROR(
            "If we peeked the input, we should have been able to read it");
        NE_CORE_RESULT(NE_CORE_RESULT_ERROR);
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
        NE_CORE_RESULT(NE_CORE_RESULT_ERROR);
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
      NE_CORE_RESULT(NE_CORE_RESULT_ERROR);
      return 0;
    }
  }
#else
  (void)result;
  (void)stream;
  (void)buffer;
  (void)size;
  (void)allow_blocking;
  NE_CORE_RESULT(NE_CORE_RESULT_INTERNAL_ERROR);
  NE_CORE_ERROR("Invalid call");
  return 0;
#endif
}

/******************************************************************************/
static void _input_free(uint64_t *result, ne_core_stream *stream)
{
#if defined(NE_CORE_PLATFORM_WINDOWS)
  HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
  CloseHandle(handle);
  SetStdHandle(STD_INPUT_HANDLE, 0);
  NE_CORE_ASSERT(GetStdHandle(STD_INPUT_HANDLE) == 0, "Expected null handle");
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
#else
  (void)stream;
  NE_CORE_RESULT(NE_CORE_RESULT_INTERNAL_ERROR);
  NE_CORE_ERROR("Invalid call");
#endif
}

/******************************************************************************/
static void _ne_io_get_input(uint64_t *result, ne_core_stream *stream_out)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  std::memset(stream_out, 0, sizeof(*stream_out));
  stream_out->read = &_input_read;
  stream_out->free = &_input_free;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_io_get_input)(uint64_t *result,
                        ne_core_stream *stream_out) = &_ne_io_get_input;

/******************************************************************************/
static uint64_t _outgoing_write(uint64_t *result,
                                ne_core_stream *stream,
                                const void *buffer,
                                uint64_t size,
                                ne_core_bool allow_blocking,
                                uint64_t os_handle)
{
#if defined(NE_CORE_PLATFORM_WINDOWS)
  // NOTE: We NEED to handle blocking here. This won't happen for a file or
  // console, however it may happen for a COM port or a pipe (especially if the
  // end of the pipe isn't reading).
  HANDLE handle = GetStdHandle((DWORD)os_handle);
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
      NE_CORE_RESULT(NE_CORE_RESULT_ERROR);
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
    NE_CORE_RESULT(NE_CORE_RESULT_ERROR);
    return 0;
  }
#else
  (void)result;
  (void)stream;
  (void)buffer;
  (void)size;
  (void)allow_blocking;
  (void)os_handle;
  NE_CORE_RESULT(NE_CORE_RESULT_INTERNAL_ERROR);
  NE_CORE_ERROR("Invalid call");
  return 0;
#endif
}

/******************************************************************************/
static void _outgoing_flush(uint64_t *result,
                            const ne_core_stream *stream,
                            uint64_t os_handle)
{
#if defined(NE_CORE_PLATFORM_WINDOWS)
  HANDLE handle = GetStdHandle((DWORD)os_handle);

  // Console is not buffered (see FlushFileBuffers docs).
  if (GetFileType(handle) == FILE_TYPE_CHAR)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
    return;
  }

  if (FlushFileBuffers(handle))
    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  else
    NE_CORE_RESULT(NE_CORE_RESULT_ERROR);
#else
  (void)result;
  (void)stream;
  (void)os_handle;
  NE_CORE_RESULT(NE_CORE_RESULT_INTERNAL_ERROR);
  NE_CORE_ERROR("Invalid call");
#endif
}

/******************************************************************************/
static void _outgoing_free(uint64_t *result,
                           ne_core_stream *stream,
                           uint64_t os_handle)
{
#if defined(NE_CORE_PLATFORM_WINDOWS)
  HANDLE handle = GetStdHandle((DWORD)os_handle);
  CloseHandle(handle);
  SetStdHandle((DWORD)os_handle, 0);
  NE_CORE_ASSERT(GetStdHandle((DWORD)os_handle) == 0, "Expected null handle");
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
#else
  (void)result;
  (void)stream;
  (void)os_handle;
  NE_CORE_RESULT(NE_CORE_RESULT_INTERNAL_ERROR);
  NE_CORE_ERROR("Invalid call");
#endif
}

/******************************************************************************/
static uint64_t _output_write(uint64_t *result,
                              ne_core_stream *stream,
                              const void *buffer,
                              uint64_t size,
                              ne_core_bool allow_blocking)
{
  return _outgoing_write(result, stream, buffer, size, allow_blocking,
                         NE_IO_OUTPUT);
}

/******************************************************************************/
static void _output_flush(uint64_t *result, const ne_core_stream *stream)
{
  return _outgoing_flush(result, stream, NE_IO_OUTPUT);
}

/******************************************************************************/
static void _output_free(uint64_t *result, ne_core_stream *stream)
{
  return _outgoing_free(result, stream, NE_IO_OUTPUT);
}

/******************************************************************************/
static uint64_t _error_write(uint64_t *result,
                             ne_core_stream *stream,
                             const void *buffer,
                             uint64_t size,
                             ne_core_bool allow_blocking)
{
  return _outgoing_write(result, stream, buffer, size, allow_blocking,
                         NE_IO_ERROR);
}

/******************************************************************************/
static void _error_flush(uint64_t *result, const ne_core_stream *stream)
{
  return _outgoing_flush(result, stream, NE_IO_ERROR);
}

/******************************************************************************/
static void _error_free(uint64_t *result, ne_core_stream *stream)
{
  return _outgoing_free(result, stream, NE_IO_ERROR);
}

/******************************************************************************/
static void _ne_io_get_output(uint64_t *result, ne_core_stream *stream_out)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  std::memset(stream_out, 0, sizeof(*stream_out));
  stream_out->write = &_output_write;
  stream_out->flush = &_output_flush;
  stream_out->free = &_output_free;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_io_get_output)(uint64_t *result,
                         ne_core_stream *stream_out) = &_ne_io_get_output;

/******************************************************************************/
static void _ne_io_get_error(uint64_t *result, ne_core_stream *stream_out)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  std::memset(stream_out, 0, sizeof(*stream_out));
  stream_out->write = &_error_write;
  stream_out->flush = &_error_flush;
  stream_out->free = &_error_free;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_io_get_error)(uint64_t *result,
                        ne_core_stream *stream_out) = &_ne_io_get_error;
