// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include <stdint.h>

#if defined(__cplusplus)
#  define NE_BEGIN extern "C" {
#  define NE_END }
#else
#  define NE_BEGIN
#  define NE_END
#endif

#if defined(NE)
#  define NE_API __declspec(dllimport)
#else
#  define NE_API
#endif

NE_BEGIN

// This header is considered tentative and is a work in progress. It contains the API
// as well as notes about how the API should work. Some of these notes may eventually
// be transcribed into a standards document.

// Compile once. Run Everywhere.

// Strings are UTF8.
// Assume platform is 64 bit little endian.

// Definitions:
//
//   invalid: Every call to the API functions will return null,
//            0, ne_false, or the binary 0 equivalent.
//
//   sub-api: All functions labeled ne_xxxxx where xxxxx is a name.
//
// supported: A platform supports this function.

// If a 'request_permission' function exists then it must be called and must return
// ne_true, otherwise all calls are *invalid* and return 'ne_result_core_permission_denied'.

// A 'supported' call will indicate that a particular sub-api
// may not exist on some platforms, e.g. 'ne_thread_exists'.
// If a sub-api does not exist, all calls to it are *invalid* and return 'ne_result_core_not_supported'.
// This also includes a 'request_permission' function if applicable.
// It is safe to call functions without checking existence as long as return values/results are checked.

// The manifest file may specify required permissions that will be requested prior
// to the application running any executable code.

// The manifest file may also specify dependencies (by name and version)
// and alternate package management servers. Packages are grabbed by http request.

// All file paths are in unix format and use '/' as well as start with a '/'.

// Newlines are always '\n' (the '\r' or '\r\n' line endings will never be used).

// All functions are thread safe to call and their
// operations may be treated as atomic, unless otherwise noted.

// All functions also validate all inputs will set the result 'ne_result_core_invalid_parameter'.
// This includes checking if all free/close calls are passed
// valid pointers, misuse of special thread functions, etc.

// All functions take an ne_result* parameter as the first argument.
// If the argument is not null, it will be filled out with either an error code
// or 'ne_result_core_success' if it succeeded.
// If a function does not specify that it can return any errors, then it will always succeed
// provided that the sub-api is supported and permission was requested (where applicable).

// The goal is to make behavior as consistent as possible.

// Where possible, fixed size memory structures should be copied out to the user.

// Memory types:
// application-owned
// platform-owned
// user-owned

// 'screen' means relative to the primary monitor.
// 'border' means relative to the top left of the window's border.
// 'client' means relative to the top left of the window's client/drawable area.

// NE will also look for the standard forms of 'main' (whether it returns a value or not, or takes argc/argv).

typedef uint8_t ne_bool;

static const ne_bool ne_true = 1;
static const ne_bool ne_false = 0;
static void* const ne_null = 0;

// An empty string that can be pointer compared.
extern const char* const ne_empty_string;

struct ne_int64_2
{
  int64_t x;
  int64_t y;
};
typedef struct ne_int64_2 ne_int64_2;

struct ne_image
{
  uint8_t* buffer;
  uint64_t size_x;
  uint64_t size_y;
};
typedef struct ne_image ne_image;

/// Errors:

typedef uint64_t ne_result;

// Enumeration of status codes.
// Each sub-api may define their own values and may use any value within 0 to 2^63.
// Results between sub-apis may overlap, so only use the correspond sub-api results, or the following:

// The operation was a success.
static const ne_result ne_result_core_success = (ne_result)-1;

// A parameter that was passed in was not valid for the function.
static const ne_result ne_result_core_invalid_parameter = (ne_result)-2;

// An attempt to call a function was denied because the application never requested
// permission for the sub-api, or permission was requested but denied.
static const ne_result ne_result_core_permission_denied = (ne_result)-3;

// The sub-api was not implemented and so the function cannot be called.
static const ne_result ne_result_core_not_supported = (ne_result)-4;

// Only used for unit testing to set the result before calling a function.
// Using this value allows us to validate that the result was properly set.
static const ne_result ne_result_core_not_set = (ne_result)-5;

NE_END
