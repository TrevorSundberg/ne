// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once

#if defined(__cplusplus)
#define NE_CORE_BEGIN extern "C" {
#define NE_CORE_END }
#else
#define NE_CORE_BEGIN
#define NE_CORE_END
#endif

// This should always be defined by the build system, however
// if a user grabbed the header directly and included it in their
// own project then it would not be defined. This is acceptable
// practice if the user is only including the header, but to build
// the c files (or a library) the user/build system must specify.
#if !defined(NE_CORE_PLATFORM_NAME)
#define NE_CORE_PLATFORM_NAME "Undefined"
#define NE_CORE_PLATFORM_UNDEFINED 1
#endif

#if defined(NE_CORE_PLATFORM_NE)
// This attribute is valid in clang/llvm (the only compiler we use for
// NE_CORE_PLATFORM_NE).
#define NE_CORE_API extern __attribute__((section("ne")))

// Since we're compiling through LLVM / Clang with a specified data-layout
// then we know the exact sizes of the following types.
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef long long intmax_t;
typedef unsigned long long uintmax_t;
#else
#define NE_CORE_API extern

// If we're not compiling with NE_CORE_PLATFORM_NE defined, it means we may be
// on any compiler and we are most likely compiling with libc. Because of this,
// we must rely on each compilers definition of sized types.
#include <stdint.h>
#endif

typedef uint8_t ne_core_bool;

#define NE_CORE_TRUE 1
#define NE_CORE_FALSE 0
#define NE_CORE_NULL 0

typedef uint64_t ne_core_primitive_type;
static const ne_core_primitive_type ne_core_primitive_type_int8 = 0;
static const ne_core_primitive_type ne_core_primitive_type_int16 = 1;
static const ne_core_primitive_type ne_core_primitive_type_int32 = 2;
static const ne_core_primitive_type ne_core_primitive_type_int64 = 3;
static const ne_core_primitive_type ne_core_primitive_type_uint8 = 4;
static const ne_core_primitive_type ne_core_primitive_type_uint16 = 5;
static const ne_core_primitive_type ne_core_primitive_type_uint32 = 6;
static const ne_core_primitive_type ne_core_primitive_type_uint64 = 7;
static const ne_core_primitive_type ne_core_primitive_type_float = 8;
static const ne_core_primitive_type ne_core_primitive_type_double = 9;
static const ne_core_primitive_type ne_core_primitive_type_pointer = 10;

#define NE_CORE_ENCLOSURE(code)                                                \
  do {                                                                         \
    code                                                                       \
  } while (NE_CORE_FALSE)

#if !defined(NDEBUG)
#define NE_CORE_ASSERT(condition, text)                                        \
  NE_CORE_ENCLOSURE(if (!(condition)) ne_core_error(0, (text));)
#else
#define NE_CORE_ASSERT(condition, text)
#endif
#define NE_CORE_ERROR_IF(condition, text) NE_CORE_ASSERT(!(condition), text)
#define NE_CORE_ERROR(text) NE_CORE_ASSERT(0, text)

#define NE_CORE_RESULT(code) NE_CORE_ENCLOSURE(if (result) *result = code;)

#define NE_CORE_DECLARE_PACKAGE(name, major, minor)                            \
  NE_CORE_API ne_core_bool (*name##_supported)(uint64_t * result);             \
  NE_CORE_API ne_core_bool (*name##_request_permission)(uint64_t * result);    \
  NE_CORE_API uint64_t (*name##_version_linked_major)(uint64_t * result);      \
  NE_CORE_API uint64_t (*name##_version_linked_minor)(uint64_t * result);      \
  static const uint64_t name##_version_header_major = major; /* NOLINT */      \
  static const uint64_t name##_version_header_minor = minor  /* NOLINT */

#define NE_CORE_DEFINE_PACKAGE_VERSION(name)                                   \
  static uint64_t _##name##_version_linked_major(uint64_t *result) {           \
    NE_CORE_RESULT(ne_core_result_success);                                    \
    return name##_version_header_major;                                        \
  }                                                                            \
  static uint64_t _##name##_version_linked_minor(uint64_t *result) {           \
    NE_CORE_RESULT(ne_core_result_success);                                    \
    return name##_version_header_minor;                                        \
  }                                                                            \
  uint64_t (*name##_version_linked_major)(uint64_t * result) =                 \
      &_##name##_version_linked_major;                                         \
  uint64_t (*name##_version_linked_minor)(uint64_t * result) =                 \
      &_##name##_version_linked_minor

#if defined(NE_CORE_PLATFORM_NE)
// When defining a package in NE_CORE_PLATFORM_NE, we never implement the
// 'supported' or 'request_permission' functions. It is up to the sandbox to
// automatically provide implementations for those functions for every package.
#define NE_CORE_DEFINE_PACKAGE(name) NE_CORE_DEFINE_PACKAGE_VERSION(name)
#else
// When just linking against ne and using it as a library, we the library will
// define that it is supported and that all request permission functions
// automatically return true.

// NOLINTNEXTLINE
#define NE_CORE_DEFINE_PACKAGE(name)                                           \
  NE_CORE_DEFINE_PACKAGE_VERSION(name);                                        \
  static ne_core_bool _##name##_supported(uint64_t *result) {                  \
    NE_CORE_RESULT(ne_core_result_success);                                    \
    return NE_CORE_TRUE;                                                       \
  }                                                                            \
  static ne_core_bool _##name##_request_permission(uint64_t *result) {         \
    NE_CORE_RESULT(ne_core_result_success);                                    \
    return NE_CORE_TRUE;                                                       \
  }                                                                            \
  ne_core_bool (*name##_supported)(uint64_t * result) = &_##name##_supported;  \
  ne_core_bool (*name##_request_permission)(uint64_t * result) =               \
      &_##name##_request_permission
#endif

NE_CORE_BEGIN

// This header is considered tentative and is a work in progress. It contains
// the API as well as notes about how the API should work. Some of these notes
// may eventually be transcribed into a standards document.

// Compile once. Run Everywhere.

// All of the ne API are done as function pointers. This is so that any of them
// may be replaced at any time (for example, to add debug layers like a memory
// leak checker). All global function pointers should be initialized by the
// sandbox before any user code is allowed to run.

// Strings are UTF8 and null terminated.
// Assume platform is 64 bit little endian.

// Definitions:
//
//  invalid:
//    Every call to the API functions will return null, 0, NE_CORE_FALSE, or the
//    binary 0 equivalent.
//  sub-api:
//    All functions labeled ne_xxxxx where xxxxx is a name.
//  supported:
//    A platform supports this function.
//  screen-space:
//    Relative to the top left of the primary monitor.
//  border-space:
//    Relative to the top left of the window's border.
//  client-space:
//    Relative to the top left of the window's client/drawable area.
//  filename:
//    Only the last name in a file path. In "/tmp/test.txt", "test.txt" is the
//    filename.
//  path:
//    The entire path to a directory or file. For example "/tmp/test.txt" or
//    "/tmp/".

// Tags:
//
//  sandbox-owned-memory:
//    Memory returned is owned by the platform. The next call to the
//    same function will free or re-use the previously returned memory.
//  user-owned-memory:
//    Memory returned is owned by the user. The user must call 'ne_core_free' on
//    the returned memory.
//  internally-called:
//    This function may be called by the user, but must be called by the
//    platform internally. If the function pointer is replaced, it is undefined
//    behavior to call any other ne function at that time as the platform may
//    not expect it.

// If a 'request_permission' function exists then it must be called and must
// return NE_CORE_TRUE, otherwise all calls are *invalid* and return
// 'ne_core_result_permission_denied'.

// The 'request_permission' functions are never defined by a library, and are
// always defined by the sandbox.

// A 'supported' call will indicate that a particular sub-api
// may not exist on some platforms, e.g. 'ne_thread_exists'.
// If a sub-api does not exist, all calls to it are *invalid* and return
// 'ne_core_result_not_supported'. This also includes a 'request_permission'
// function if applicable. It is safe to call functions without checking
// existence as long as return values/results are checked.

// If any errors are returned from a function, the function must also return
// null/0, etc.

// The manifest file may specify required permissions that will be requested
// prior to the application running any executable code.

// The manifest file may also specify dependencies (by name and version)
// and alternate package management servers. Packages are grabbed by http
// request.

// All file paths are in unix format and use '/' as well as start with a '/'.

// Newlines are always '\n' (the '\r' or '\r\n' line endings will never be
// used).

// All parameters that output will be have '_out' as a suffix. Outputs should
// always be at the end of a parameter list.

// If a function results in an error, it must either output all possible values
// and returns, or it must return nothing and output nothing (null).

// All functions that accept pointers do not implicitly allow null unless
// otherwise specified. Results are undefined if null is given.

// Error results are listed in priority order, meaning that if two errors are
// possible on a single function call, the one that will occur is the first one
// listed.

// When implementing a function pointer, the implementation should be static
// (internal linkage) and should match the name of the function
// pointer, with the added prefix '_' (e.g. _ne_core_event_peek).

// Any time we refer to a char* or const char*, it will be null terminated.
// All buffers of arbitrary sized data (non-null terminated) will be uint8_t*.

// All functions are non-blocking, unless otherwise noted.

// All functions are thread safe to call and their
// operations may be treated as atomic, unless otherwise noted.

// All functions also validate all inputs will set the result
// 'ne_core_result_invalid_parameter'. This includes checking if all free/close
// calls are passed valid pointers, misuse of special thread functions, etc.

// All functions take an uint64_t* parameter as the first argument.
// If the argument is not null, it will be filled out with either an error code
// or 'ne_core_result_success' if it succeeded.
// If a function does not specify that it can return any errors, then it will
// always succeed provided that the sub-api is supported and permission was
// requested (where applicable).

// The goal is to make behavior as consistent as possible.

// Where possible, fixed size memory structures should be copied out to the
// user.

// The typedef for any bit flags will end in '_flags'.

// All functions that are querying for true/false and return 'ne_core_bool'
// should start with 'is_' after 'ne_'.

// If an API operates on an object, treating it almost like a 'this' pointer,
// the object pointer should always come directly after the uint64_t* (second
// parameter).

// Ne APIS never abbreviate words and always use the whole word, for example
// 'microphone' instead of 'mic', 'rectangle' instead of 'rect'...

NE_CORE_DECLARE_PACKAGE(ne_core, 0, 0);

// The first call to be made in an ne application.
// Static initialization in llvm will occur before this call.
// When relying on libc, 'main' will be invoked at this time.
extern int32_t ne_core_main(int32_t argc, char *argv[]);

// Enumeration of status codes.
// Each sub-api may define their own values and may use any value within 0 to
// 2^63. Results between sub-apis may overlap, so only use the correspond
// sub-api results, or the following:

// The operation was a success.
static const uint64_t ne_core_result_success = (uint64_t)-1;

// A parameter that was passed in was not valid for the function.
static const uint64_t ne_core_result_invalid_parameter = (uint64_t)-2;

// An attempt to call a function was denied because the application never
// requested permission for the sub-api, or permission was requested but denied.
static const uint64_t ne_core_result_permission_denied = (uint64_t)-3;

// The sub-api was not implemented and so the function cannot be called.
static const uint64_t ne_core_result_not_supported = (uint64_t)-4;

// An error occurred.
static const uint64_t ne_core_result_error = (uint64_t)-5;

// Only used for unit testing to set the result before calling a function.
// Using this value allows us to validate that the result was properly set.
static const uint64_t ne_core_result_not_set = (uint64_t)-6;

// An attempt to allocate memory failed because the system does not have enough
// memory or addressable space.
static const uint64_t ne_core_result_allocation_failed = (uint64_t)-7;

// Intrinsic functions are not treated as function pointers because they are
// expected to be implemented directly by the compiler and heavily optimized.
NE_CORE_API void ne_intrinsic_memory_set(void *memory, uint8_t value,
                                         uint64_t size);
NE_CORE_API int64_t ne_intrinsic_memory_compare(void *a, void *b,
                                                uint64_t size);

// This is the first function that should be called to test if functionality is
// working. The application will display a Hello World message in a console,
// notification, or non-blocking/non-modal message box. This is also used as an
// introductory function to demonstrate that an application is working. This
// function should be non-blocking.
NE_CORE_API void (*ne_core_hello_world)(uint64_t *result);

typedef void (*ne_core_main_loop_callback)(void *user_data);
// To run on any cooperative multi-tasked system (e.g. Emscripten) we must
// call this function so that we may periodically yield to the OS/browser.
// Calling this function sets a callback that will be called as fast as
// possible by the operating system. This function never returns.
// Stack objects allocated at the time of this call will not
// be cleaned up (destructors will not run), and their lifetime is not
// guaranteed. It is safest to call this immediately in main and perform all
// initialization on the first iteration. This may only be called once.
// Note that this function is also supported by non-cooperative
// multi-tasked systems. To stop the program, call 'ne_core_exit'.
//     ne_core_result_error:
//       If the function was called more than once.
NE_CORE_API void (*ne_core_main_loop)(uint64_t *result,
                                      ne_core_main_loop_callback callback,
                                      void *user_data);

// Terminates the program immediately. Flushes and closes any open streams.
// Invokes global destructors.
NE_CORE_API void (*ne_core_exit)(uint64_t *result, int32_t return_code);

// Signal an error to the application. This is mostly used for unit tests.
NE_CORE_API void (*ne_core_error)(uint64_t *result, const char *message);

//    #internally-called, #user-owned-memory
//    @ne_memory_result_allocation_failed:
//      Not enough system memory or address space, or other system error.
NE_CORE_API uint8_t *(*ne_core_allocate)(uint64_t *result, uint64_t sizeBytes);

// Frees the memory (only should be memory returned from 'ne_memory_allocate').
NE_CORE_API void (*ne_core_free)(uint64_t *result, void *memory);

typedef struct ne_core_event ne_core_event;
struct ne_core_event {
  // The type of an event is uniquely identified by a pointer to a global
  // variable. The value in the uint64_t should always be 0 (it is
  // non-consequential).
  const uint64_t *type;

  // The data of an event is variable sized and therefore must be freed by
  // 'ne_core_free'.
  //    #user-owned-memory
  void *data;

  // The originating time of the event on this machine in nanoseconds since the
  // Unix Epoch (00:00:00 Coordinated Universal Time (UTC), January 1st 1970).
  // Events are ordered by creation therefore this time is always equal to or
  // greater than the previous event in the queue. This time will always be the
  // highest precision that the platform may offer.
  uint64_t time;
};

// Gets the event at the front of the event queue (does not remove it from the
// queue). Returns true if an event was retrieved, false otherwise.
NE_CORE_API ne_core_bool (*ne_core_event_peek)(uint64_t *result,
                                               ne_core_event *event_out);

// Removes the event at the front of the event queue. The queue is first in
// first out (FIFO). Returns true if an event was removed, false otherwise.
NE_CORE_API ne_core_bool (*ne_core_event_dequeue)(uint64_t *result);

// Queues an event onto the event queue. The queue is first in first out (FIFO).
//    #internally-called
NE_CORE_API ne_core_bool (*ne_core_event_enqueue)(uint64_t *result,
                                                  const uint64_t *type,
                                                  void *data);

typedef struct ne_core_enumerator ne_core_enumerator;
struct ne_core_enumerator {
  // Checks to see if the enumerator is empty (has no more values within it).
  ne_core_bool (*empty)(uint64_t *result, const ne_core_enumerator *enumerator);

  // Advanced the enumerator by one element.
  //     ne_core_result_error:
  //       Advance was called on an empty enumerator.
  void (*advance)(uint64_t *result, ne_core_enumerator *enumerator);

  // Reads the current value of the enumerator. The type output by
  // each enumerator is specified by the API call that outputs the enumerator.
  //     ne_core_result_error:
  //       Advance was called on an empty enumerator.
  void (*dereference)(uint64_t *result, const ne_core_enumerator *enumerator,
                      void *value_out);

  // Frees and resources or memory held by the enumerator.
  // After this all calls to the 'ne_core_enumerator' are undefined.
  void (*free)(uint64_t *result, ne_core_enumerator *enumerator);

  // Opaque data used by the platform / implementation to represent the
  // enumerator.
  uint8_t opaque[16];
};

typedef uint64_t ne_core_stream_seek_origin;
static const ne_core_stream_seek_origin ne_core_stream_seek_origin_begin = 0;
static const ne_core_stream_seek_origin ne_core_stream_seek_origin_current = 1;
static const ne_core_stream_seek_origin ne_core_stream_seek_origin_end = 2;
static const ne_core_stream_seek_origin ne_core_stream_seek_origin_count = 3;

// All the functionality needed for any type of stream.
// Note that if any ne function outputs 'ne_core_stream', it must specify the
// exact set of functionality that is expected. Only the functions that are
// expected should be filled out, all others will be nulled out by the platform.
// The stream structure must be passed in to each function. It is undefined
// behavior to pass in the wrong stream.
typedef struct ne_core_stream ne_core_stream;
struct ne_core_stream {
  // When blocking, this always reads the entire size specified unless the end
  // of the stream or an error occurs. When non-blocking, this will read as much
  // as possible and will not wait for more. Note that when it reaches the end
  // of the stream it will still return ne_core_result_success. Returns how much
  // data was read.
  //    ne_core_result_error:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  uint64_t (*read)(uint64_t *result, ne_core_stream *stream, void *buffer,
                   uint64_t size, ne_core_bool allow_blocking);

  // When blocking, this always writes the entire size specified unless the end
  // of the stream or an error occurs. When non-blocking, this will writes as
  // much as possible and will not wait for more. Returns how much data was
  // written.
  //    ne_core_result_error:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  uint64_t (*write)(uint64_t *result, ne_core_stream *stream,
                    const void *buffer, uint64_t size,
                    ne_core_bool allow_blocking);

  //    ne_core_result_error:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  void (*flush)(uint64_t *result, const ne_core_stream *stream);

  //    ne_core_result_error:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  uint64_t (*get_position)(uint64_t *result, const ne_core_stream *stream);

  //    ne_core_result_error:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  uint64_t (*get_length)(uint64_t *result, const ne_core_stream *stream);

  ne_core_bool (*is_eof)(uint64_t *result, const ne_core_stream *stream);

  //    ne_core_result_error:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  void (*seek)(uint64_t *result, ne_core_stream *stream,
               ne_core_stream_seek_origin origin, uint64_t position);

  // Frees any resources or memory held by the stream.
  // After this all calls to the 'ne_core_stream' are undefined.
  void (*free)(uint64_t *result, ne_core_stream *stream);

  // Opaque data used by the platform / implementation to represent the stream.
  uint8_t opaque[16];
};

NE_CORE_END
