// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once

#if defined(__cplusplus)
# define NE_BEGIN extern "C" {
# define NE_END }
#else
# define NE_BEGIN
# define NE_END
#endif

#if defined(NE)
# define NE_API extern __attribute__((section("ne")))

  // Since we're compiling through LLVM / Clang with a specified data-layout
  // then we know the exact sizes of the following types.
  typedef signed char         int8_t;
  typedef short               int16_t;
  typedef int                 int32_t;
  typedef long long           int64_t;
  typedef unsigned char       uint8_t;
  typedef unsigned short      uint16_t;
  typedef unsigned int        uint32_t;
  typedef unsigned long long  uint64_t;
  typedef long long           intmax_t;
  typedef unsigned long long  uintmax_t;
#else
# define NE_API extern

  // If we're not compiling with NE defined, it means we may be on any compiler
  // and we are most likely compiling with libc. Because of this, we must rely
  // on each compilers definition of sized types.
# include <stdint.h>
#endif

#define NE_DECLARE_PACKAGE(name, major, minor)\
  NE_API ne_core_bool (*name##_supported)(uint64_t* result);\
  NE_API ne_core_bool (*name##_request_permission)(uint64_t* result);\
  NE_API uint64_t (*name##_version_linked_major)(uint64_t* result);\
  NE_API uint64_t (*name##_version_linked_minor)(uint64_t* result);\
  static const uint64_t name##_version_header_major = major;\
  static const uint64_t name##_version_header_minor = minor;

#define NE_DEFINE_PACKAGE_VERSION(name)\
    uint64_t name##_version_linked_major_implementation(uint64_t* result) { *result = ne_core_result_success; return name##_version_header_major; }\
    uint64_t name##_version_linked_minor_implementation(uint64_t* result) { *result = ne_core_result_success; return name##_version_header_minor; }\
    uint64_t (*name##_version_linked_major)(uint64_t* result) = &name##_version_linked_major_implementation;\
    uint64_t (*name##_version_linked_minor)(uint64_t* result) = &name##_version_linked_minor_implementation;

#if defined(NE)
  // When defining a package in NE, we never implement the 'supported' or 'request_permission' functions.
  // It is up to the sandbox to automatically provide implementations for those functions for every package.
# define NE_DEFINE_PACKAGE(name)\
    NE_DEFINE_PACKAGE_VERSION(name)
#else
  // When just linking against NE and using it as a library, we the library will define
  // that it is supported and that all request permission functions automatically return true.
# define NE_DEFINE_PACKAGE(name)\
    NE_DEFINE_PACKAGE_VERSION(name)\
    static ne_core_bool name##_supported_implementation(uint64_t* result) { *result = ne_core_result_success; return ne_core_true; }\
    static ne_core_bool name##_request_permission_implementation(uint64_t* result) { *result = ne_core_result_success; return ne_core_true; }\
    ne_core_bool (*name##_supported)(uint64_t* result) = &name##_supported_implementation;\
    ne_core_bool (*name##_request_permission)(uint64_t* result) = &name##_request_permission_implementation;
#endif

NE_BEGIN

// This header is considered tentative and is a work in progress. It contains the API
// as well as notes about how the API should work. Some of these notes may eventually
// be transcribed into a standards document.

// Compile once. Run Everywhere.

// All of the ne API are done as function pointers. This is so that any of them
// may be replaced at any time (for example, to add debug layers like a memory leak checker).
// All global function pointers should be initialized by the sandbox before any user
// code is allowed to run.

// Strings are UTF8.
// Assume platform is 64 bit little endian.

// Definitions:
//
//   invalid: Every call to the API functions will return null,
//            0, ne_core_false, or the binary 0 equivalent.
//
//   sub-api: All functions labeled ne_xxxxx where xxxxx is a name.
//
// supported: A platform supports this function.

// If a 'request_permission' function exists then it must be called and must return
// ne_core_true, otherwise all calls are *invalid* and return 'ne_core_result_permission_denied'.

// The 'request_permission' functions are never defined by a library, and are always defined by the sandbox.

// A 'supported' call will indicate that a particular sub-api
// may not exist on some platforms, e.g. 'ne_thread_exists'.
// If a sub-api does not exist, all calls to it are *invalid* and return 'ne_core_result_not_supported'.
// This also includes a 'request_permission' function if applicable.
// It is safe to call functions without checking existence as long as return values/results are checked.

// The manifest file may specify required permissions that will be requested prior
// to the application running any executable code.

// The manifest file may also specify dependencies (by name and version)
// and alternate package management servers. Packages are grabbed by http request.

// All file paths are in unix format and use '/' as well as start with a '/'.

// Newlines are always '\n' (the '\r' or '\r\n' line endings will never be used).

// All parameters that output will be have '_out' as a suffix.

// All functions that accept pointers do not implicitly allow null unless otherwise specified.

// When implementing a function pointer, the implementation should be static (internal linkage)
// and should generally match the name of the function pointer, with the added suffix '_implementation'.

// All functions are non-blocking, unless otherwise noted.

// All functions are thread safe to call and their
// operations may be treated as atomic, unless otherwise noted.

// All functions also validate all inputs will set the result 'ne_core_result_invalid_parameter'.
// This includes checking if all free/close calls are passed
// valid pointers, misuse of special thread functions, etc.

// All functions take an uint64_t* parameter as the first argument.
// If the argument is not null, it will be filled out with either an error code
// or 'ne_core_result_success' if it succeeded.
// If a function does not specify that it can return any errors, then it will always succeed
// provided that the sub-api is supported and permission was requested (where applicable).

// The goal is to make behavior as consistent as possible.

// Where possible, fixed size memory structures should be copied out to the user.

// The typedef for any bit flags will end in '_flags'.

// All functions that are querying for true/false and return 'ne_core_bool' should start with 'is_' after 'ne_'.

// If an API operates on an object, treating it almost like a 'this' pointer, the object
// pointer should always come directly after the uint64_t* (second parameter).

// Ne APIS never abbreviate words and always use the whole word, for example 'microphone' instead of 'mic',
// 'rectangle' instead of 'rect'...

// Memory types:
// application-owned
// platform-owned
// user-owned

// 'screen' means relative to the primary monitor.
// 'border' means relative to the top left of the window's border.
// 'client' means relative to the top left of the window's client/drawable area.

// NE will also look for the standard forms of 'main' (whether it returns a value or not, or takes argc/argv).

typedef uint8_t ne_core_bool;

static const ne_core_bool ne_core_true = 1;
static const ne_core_bool ne_core_false = 0;

NE_DECLARE_PACKAGE(ne_core, 0, 0);

static void* const ne_core_null = 0;

// An empty string that can be pointer compared.
extern const char* const ne_core_empty_string;

// Enumeration of status codes.
// Each sub-api may define their own values and may use any value within 0 to 2^63.
// Results between sub-apis may overlap, so only use the correspond sub-api results, or the following:

// The operation was a success.
static const uint64_t ne_core_result_success = (uint64_t)-1;

// A parameter that was passed in was not valid for the function.
static const uint64_t ne_core_result_invalid_parameter = (uint64_t)-2;

// An attempt to call a function was denied because the application never requested
// permission for the sub-api, or permission was requested but denied.
static const uint64_t ne_core_result_permission_denied = (uint64_t)-3;

// The sub-api was not implemented and so the function cannot be called.
static const uint64_t ne_core_result_not_supported = (uint64_t)-4;

// An error occurred.
static const uint64_t ne_core_result_error = (uint64_t)-5;

// Only used for unit testing to set the result before calling a function.
// Using this value allows us to validate that the result was properly set.
static const uint64_t ne_core_result_not_set = (uint64_t)-6;

// Signal an error to the application. This is mostly used for unit tests.
// Note we do not call this function for unit testing.
NE_API void (*ne_core_error)(uint64_t* result, const char* message);

// This is the first function that should be called to test if functionality is working.
// The application will display a Hello World message in a console or as a message box.
// This is also used as an introductory function to demonstrate that an application is working.
// This function should be non blocking.
NE_API void (*ne_core_hello_world)(uint64_t* result);

typedef uint64_t ne_core_stream_seek_origin;
static const ne_core_stream_seek_origin ne_core_stream_seek_origin_begin = 0;
static const ne_core_stream_seek_origin ne_core_stream_seek_origin_current = 1;
static const ne_core_stream_seek_origin ne_core_stream_seek_origin_end = 2;
static const ne_core_stream_seek_origin ne_core_stream_seek_origin_count = 3;

typedef struct ne_core_stream ne_core_stream;

// All the functionality needed for any type of stream.
// Note that if any ne function outputs 'ne_core_stream', it must specify the exact set of functionality that is expected.
// Only the functions that are expected should be filled out, all others will be nulled out by the platform.
struct ne_core_stream
{
  // Reads the entire size specified unless the end of the stream occurs or an error occurs.
  // Note that when it reaches the end of the stream it will still return ne_core_result_success.
  // This operation is non-blocking and will return how much was read.
  //    ne_core_result_invalid_parameter:
  //      If the stream was a write only stream or invalid stream pointer.
  //    ne_core_result_error:
  //      If an error occurred on reading the stream.
  uint64_t (*read)(uint64_t* result, ne_core_stream* stream, void* buffer, uint64_t size, ne_core_bool allow_blocking);

  // This operation is non-blocking and will return how much was written.
  //    ne_core_result_invalid_parameter:
  //      If the stream was a read only stream or invalid stream pointer.
  //    ne_core_result_error:
  //      If an error occurred on writing the stream.
  uint64_t (*write)(uint64_t* result, ne_core_stream* stream, const void* buffer, uint64_t size, ne_core_bool allow_blocking);

  //    ne_core_result_invalid_parameter:
  //      If the stream was a read only stream or invalid stream pointer.
  //    ne_core_result_error:
  //      If an error occurred on flushing the stream.
  void (*flush)(uint64_t* result, ne_core_stream* stream);

  uint64_t (*get_position)(uint64_t* result, ne_core_stream* stream);

  uint64_t (*get_length)(uint64_t* result, ne_core_stream* stream);

  ne_core_bool (*is_eof)(uint64_t* result, ne_core_stream* stream);

  void (*seek)(uint64_t* result, ne_core_stream* stream, ne_core_stream_seek_origin origin, uint64_t position);

  void (*free)(uint64_t* result, ne_core_stream* stream);
  
  // An opaque pointer used by the platform.
  void* user_data;
};

NE_END
