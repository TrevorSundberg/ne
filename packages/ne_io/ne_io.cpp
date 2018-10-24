// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_io/ne_io.h"
#include <stdio.h>

#if defined(NE_CORE_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

NE_CORE_BEGIN

NE_CORE_DEFINE_PACKAGE(ne_io);

/******************************************************************************/
static uint64_t _input_read(uint64_t *result, ne_core_stream *stream,
                            void *buffer, uint64_t size,
                            ne_core_bool allow_blocking) {

#if defined(NE_CORE_PLATFORM_WINDOWS)
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);

  HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
  DWORD type = GetFileType(handle);

  // We need to handle console types separately in Windows because
  // characters are treated as wide characters (we must convert to UTF-8).
  if (type == FILE_TYPE_CHAR) {

    // We always request to read exactly the number of characters that the user
    // requested with 'size'. When converted to UTF-8, each character read could
    // be anywhere from 1 byte to 4 bytes (8 bytes?) which means that in the
    // best case scenario all characters turned into 1 byte and we read the
    // exact amount needed. In the worst case scenario, all characters turned
    // into 4 bytes and we over-read.
    // size_t wide_size = (size_t)size * sizeof(wchar_t);
    // wchar_t *wide_buffer = (wchar_t *)malloc(wide_size);
    // if (!wide_buffer) {
    //  NE_CORE_RESULT(ne_core_result_error);
    //  return;
    //}

    // size_t input_size = (size_t)size * sizeof(INPUT_RECORD);
    // INPUT_RECORD *input_buffer = (INPUT_RECORD *)malloc(input_size);
    // if (!input_buffer) {
    //  NE_CORE_RESULT(ne_core_result_error);
    //  return 0;
    //}

    uint64_t bytes_read = 0;
    for (;;) {
      INPUT_RECORD record;
      DWORD events_peeked = 0;
      if (!PeekConsoleInputW(handle, &record, 1, &events_peeked)) {
        NE_CORE_RESULT(ne_core_result_error);
        return 0;
      }

      if (events_peeked == 0)
        break;
      NE_CORE_ASSERT(events_peeked == 1, "Should have only peeked 1 event");

      if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
        WCHAR *wide_char = &record.Event.KeyEvent.uChar.UnicodeChar;
        int multibyte_size =
            WideCharToMultiByte(CP_UTF8, 0, wide_char, 1, 0, 0, 0, 0);
        if ((uint64_t)multibyte_size + bytes_read <= size) {
          WideCharToMultiByte(CP_UTF8, 0, wide_char, 1,
                              (char *)buffer + bytes_read, multibyte_size, 0,
                              0);
          bytes_read += (uint64_t)multibyte_size;
        }
      }

      DWORD events_read = 0;
      if (!ReadConsoleInputW(handle, &record, 1, &events_read)) {
        NE_CORE_ERROR(
            "If we peeked the input, we should have been able to read it");
        NE_CORE_RESULT(ne_core_result_error);
        return 0;
      }

      NE_CORE_ASSERT(events_read == 1, "Should have only read 1 event");
    }

    return bytes_read;

    // DWORD characters_read = 0;
    // if (!ReadConsoleW(handle, wide_buffer, (DWORD)size, &characters_read, 0))
    // {
    //  NE_CORE_RESULT(ne_core_result_error);
    //  return;
    //}

    // DWORD bytes_read = 0;
    // DWORD total_bytes_available = 0;
    // DWORD bytes_left_this_message = 0;
    // if (!PeekNamedPipe(handle, wide_buffer, wide_size, &bytes_read,
    //                   &total_bytes_available, &bytes_left_this_message)) {
    //  DWORD error = GetLastError();
    //  NE_CORE_RESULT(ne_core_result_error);
    //  free(wide_buffer);
    //  return 0;
    //}

    // NE_CORE_ASSERT(
    //    bytes_read % sizeof(wchar_t) == 0,
    //    "With the console we should always be reading multiples of wchar_t");

    // int bytes = WideCharToMultiByte(CP_UTF8, 0, wide_buffer, (int)size,
    // buffer,
    //                                (int)size, 0, 0);
    // free(wide_buffer);

    // wchar_t buffer;
    // DWORD chars_read = 0;
    // ReadConsoleW(handle, &buffer, 1, &chars_read, 0);
    // PeekNamedPipe(
  } else {
    DWORD mode =
        PIPE_READMODE_BYTE | (allow_blocking ? PIPE_WAIT : PIPE_NOWAIT);
    if (SetNamedPipeHandleState(handle, &mode, 0, 0) == 0) {
      DWORD error = GetLastError();
      if (result)
        *result = ne_core_result_error;
      return 0;
    }

    DWORD amount = 0;
    if (ReadFile(handle, buffer, (DWORD)size, &amount, 0)) {
      if (result)
        *result = ne_core_result_success;
      return amount;
    } else {
      if (result)
        *result = ne_core_result_error;
      return 0;
    }
  }
#endif

  // if (allow_blocking) {
  //  uint64_t amount = (uint64_t)fread(buffer, (size_t)1, (size_t)size, stdin);
  //  if (ferror(stdin)) {
  //    if (result)
  //      *result = ne_core_result_error;
  //    clearerr(stdin);
  //  } else {
  //    if (result)
  //      *result = ne_core_result_success;
  //  }
  //  return amount;
  //}
  //
  // int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  // fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
  // read(STDIN_FILENO,  buffer
  //
  // if (result)
  //  *result = ne_core_result_not_supported;
  // return 0;

  return 0;
}

/******************************************************************************/
static uint64_t _input_get_position(uint64_t *result,
                                    const ne_core_stream *stream) {
  long position = ftell(stdin);
  if (position == -1) {
    if (result)
      *result = ne_core_result_error;
    return 0;
  } else {
    if (result)
      *result = ne_core_result_success;
  }
  return (uint64_t)position;
}

/******************************************************************************/
static uint64_t _input_get_length(uint64_t *result,
                                  const ne_core_stream *stream) {
  // Since we can't seek the stream, then we always know the length is the
  // position.
  return _input_get_position(result, stream);
}

/******************************************************************************/
static ne_core_bool _input_is_eof(uint64_t *result,
                                  const ne_core_stream *stream) {
  ne_core_bool eof = (feof(stdin) != 0);
  if (result)
    *result = ne_core_result_success;
  return eof;
}

/******************************************************************************/
void _ne_io_get_input(uint64_t *result, ne_core_stream *stream_out) {
  ne_intrinsic_memory_set(stream_out, 0, sizeof(*stream_out));
  stream_out->read = &_input_read;
  stream_out->get_position = &_input_get_position;
  stream_out->get_length = &_input_get_length;
  stream_out->is_eof = &_input_is_eof;
  if (result)
    *result = ne_core_result_success;
}
void (*ne_io_get_input)(uint64_t *result,
                        ne_core_stream *stream_out) = &_ne_io_get_input;

/******************************************************************************/
static uint64_t _output_write(uint64_t *result, ne_core_stream *stream,
                              const void *buffer, uint64_t size,
                              ne_core_bool allow_blocking) {
  // We don't need to worry about blocking here.
  (void)allow_blocking;

  uint64_t amount = (uint64_t)fwrite(buffer, (size_t)1, (size_t)size, stdout);

  if (ferror(stdout)) {
    if (result)
      *result = ne_core_result_error;
    clearerr(stdout);
  } else {
    if (result)
      *result = ne_core_result_success;
  }
  return amount;
}

/******************************************************************************/
void _output_flush(uint64_t *result, const ne_core_stream *stream) {
  if (fflush(stdout) != 0 || ferror(stdout)) {
    if (result)
      *result = ne_core_result_error;
    clearerr(stdout);
  } else {
    if (result)
      *result = ne_core_result_success;
  }
}

/******************************************************************************/
static uint64_t _output_get_position(uint64_t *result,
                                     const ne_core_stream *stream) {
  long position = ftell(stdout);
  if (position == -1) {
    if (result)
      *result = ne_core_result_error;
    return 0;
  } else {
    if (result)
      *result = ne_core_result_success;
  }
  return (uint64_t)position;
}

/******************************************************************************/
static uint64_t _output_get_length(uint64_t *result,
                                   const ne_core_stream *stream) {
  // Since we can't seek the stream, then we always know the length is the
  // position.
  return _output_get_position(result, stream);
}

/******************************************************************************/
void _ne_io_get_output(uint64_t *result, ne_core_stream *stream_out) {
  ne_intrinsic_memory_set(stream_out, 0, sizeof(*stream_out));
  stream_out->write = &_output_write;
  stream_out->flush = &_output_flush;
  stream_out->get_position = &_output_get_position;
  stream_out->get_length = &_output_get_length;
  if (result)
    *result = ne_core_result_success;
}
void (*ne_io_get_output)(uint64_t *result,
                         ne_core_stream *stream_out) = &_ne_io_get_output;

NE_CORE_END
