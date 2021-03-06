/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once

#if defined(__cplusplus)
/// Enums must be 32-bit size in C++.
#  define NE_CORE_ENUM : uint32_t
/// Use C name mangling.
#  define NE_CORE_C_LINKAGE "C"
/// Indicates a null pointer.
#  define NE_CORE_NULL nullptr
#else
/// We can't force enum sizes, so we make an enum constant called 'force_size'
/// that is as big as a 32-bit number.
#  define NE_CORE_ENUM
/// We're already in C, so we use C name mangling.
#  define NE_CORE_C_LINKAGE
/// Indicates a null pointer.
#  define NE_CORE_NULL 0
#endif

/// Used to pad platform strings so that we can guarnatee they are at least 8
/// bytes long, so that they may be reinterpreted as a uint64_t for efficient
/// checking.
#define NE_CORE_NULL_PADDING "\0\0\0\0\0\0\0"

// Below are the supported platform names. More may be added, and if external
// users implement their own platforms then they may not appear in this list
// yet. All names are guarnteed to be at least 8 bytes long and the first 8
// bytes are unique.

/// Standard name for the NE portable platform.
/// The standard define is NE_CORE_PLATFORM_NE.
#define NE_CORE_PLATFORM_NAME_NE "NE" NE_CORE_NULL_PADDING

/// Standard name for the Windows platform.
/// The standard define is NE_CORE_PLATFORM_WINDOWS.
#define NE_CORE_PLATFORM_NAME_WINDOWS "Windows" NE_CORE_NULL_PADDING

/// Standard name for an unknown platform.
/// The standard define is NE_CORE_PLATFORM_UNKNOWN.
#define NE_CORE_PLATFORM_NAME_UNKNOWN "Unknown" NE_CORE_NULL_PADDING

// This should always be defined by the build system, however
// if a user grabbed the header directly and included it in their
// own project then it would not be defined. This is acceptable
// practice if the user is only including the header, but to build
// the c files (or a library) the user/build system must specify.
#if !defined(NE_CORE_PLATFORM_NAME)
/// The name of the platform we're currently on. This name is always guaranteed
/// to be at least 8 bytes long (padded with nulls, including the implicit null
/// terminator) and the first 8 bytes are guaranteed to be unique so that the
/// user may efficiently reinterpret the string as a uint64_t for efficient
/// platform checks. When compiling the platform specific libraries, this will
/// be the name of the platform. When compiling with LLVM into a portable
/// ne-executable this will always be "NE" if you walk to the first null
/// terminator (trimmed).
#  define NE_CORE_PLATFORM_NAME NE_CORE_PLATFORM_NAME_UNKNOWN
/// A conditional constant for detecing which platform we are on.
#  define NE_CORE_PLATFORM_UNKNOWN 1
#endif

#if defined(NE_CORE_PLATFORM_NE)
// This attribute is valid in clang/llvm (the only compiler we use for
// NE_CORE_PLATFORM_NE).
/// Declares external linkage with a special type of linking that means
/// an *invalid* dummy will be stubbed in if it is not found/linked.
#  define NE_CORE_API extern NE_CORE_C_LINKAGE __attribute__((section("ne")))

// Since we're compiling through LLVM / Clang with a specified data-layout
// then we know the exact sizes of the following types.

/// 8 bit signed integer (1 byte).
typedef signed char int8_t;
/// 16 bit signed integer (2 byte).
typedef short int16_t;
/// 32 bit signed integer (4 byte).
typedef int int32_t;
/// 64 bit signed integer (8 byte).
typedef long long int64_t;
/// Max 64 bit signed integer (8 byte).
typedef long long intmax_t;
/// Pointer 64 bit signed integer (8 byte).
typedef long long intptr_t;
/// 8 bit unsigned integer (1 byte).
typedef unsigned char uint8_t;
/// 16 bit unsigned integer (2 byte).
typedef unsigned short uint16_t;
/// 32 bit unsigned integer (4 byte).
typedef unsigned int uint32_t;
/// 64 bit unsigned integer (8 byte).
typedef unsigned long long uint64_t;
/// Max 64 bit unsigned integer (8 byte).
typedef unsigned long long uintmax_t;
/// Pointer 64 bit unsigned integer (8 byte).
typedef unsigned long long uintptr_t;
#else
/// Declares external linkage with a special type of linking that means
/// an *invalid* dummy will be stubbed in if it is not found/linked.
#  define NE_CORE_API extern NE_CORE_C_LINKAGE

// If we're not compiling with NE_CORE_PLATFORM_NE defined, it means we may be
// on any compiler and we are most likely compiling with libc. Because of this,
// we must rely on each compilers definition of sized types.
#  if defined(__cplusplus)
#    include <cstdint>
typedef std::int8_t int8_t;
typedef std::int16_t int16_t;
typedef std::int32_t int32_t;
typedef std::int64_t int64_t;
typedef std::intmax_t intmax_t;
typedef std::intptr_t intptr_t;
typedef std::uint8_t uint8_t;
typedef std::uint16_t uint16_t;
typedef std::uint32_t uint32_t;
typedef std::uint64_t uint64_t;
typedef std::uintmax_t uintmax_t;
typedef std::uintptr_t uintptr_t;
#  else
#    include <stdint.h>
#  endif
#endif

/// A boolean value that may hold NE_CORE_TRUE or NE_CORE_FALSE. All other
/// values are invalid.
typedef uint8_t ne_core_bool;

/// Represents a boolean true value, to be used with #ne_core_bool.
#define NE_CORE_TRUE 1
/// Represents a boolean false value, to be used with #ne_core_bool.
#define NE_CORE_FALSE 0

/// A permission id that will never be a valid permission.
#define NE_CORE_PERMISSION_INVALID 0

/// Only used for unit testing to set the result before calling a function.
/// Using this value allows us to validate that the result was properly set.
#define NE_CORE_RESULT_INVALID 0

/// This is how many bytes we allocate for any struct that contains opaque data
/// (typically defined by the platform). This is akin to the private
/// implementation pattern, except ideally without allocating.
#define NE_CORE_OPAQUE_SIZE 16

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

// If a 'ne_xxxxx_permission' token exists then the ne_core_request_permission
// must be called and passed, and a reply in the event loop must occur before
// the library becomes usable. If access is not granted or it is never called,
// all calls to that library *invalid* and return
// #NE_CORE_RESULT_PERMISSION_DENIED.

// A 'supported' call will indicate that a particular sub-api
// may not exist on some platforms, e.g. 'ne_thread_exists'.
// If a sub-api does not exist, all calls to it are *invalid* and return
// 'NE_CORE_RESULT_NOT_SUPPORTED'. This also includes a 'request_permission'
// function if applicable. It is safe to call functions without checking
// existence as long as return values/results are checked.

// If any errors are returned from a function, the function must also return
// null/0 and must not mutate any parameters (no outputs). The function
// also must act as if there were no side effects (no partial completions). This
// is similar to the 'strong exception guarantee' in C++.

// The possible results are listed in order of priority (top to bottom). If it
// is possible for a function to return multiple errors, the first in the list
// will be chosen.

// The manifest file may specify required permissions that will be requested
// prior to the application running any executable code.

// The manifest file may also specify dependencies (by name and version)
// and alternate package management servers. Packages are grabbed by http
// request.

// Dependencies will be checked for 'supported', and if they are not supported
// the application may not run unless it's a loose dependency. We should
// automatically generate supported functions for libraries that don't have
// them, and make their support dependent upon dependent library support.

// All file paths are in unix format and use '/' as well as start with a '/'.

// Newlines are always '\n' (the '\r' or '\r\n' line endings will never be
// used).

// All parameters that output will be have '_out' as a suffix. Outputs should
// always be at the end of a parameter list. Outputs will not be filled out if
// an error occurs. A parameter that is both an input and output is labelled as
// '_inout'. In-outs also remain unchanged in the event of an error.

// All functions that accept pointers do not implicitly allow null unless
// otherwise specified. Results are undefined if null is given.

// Any parameter on a function pointer inside a struct named `self` means the
// function takes the struct itself (like a C++ this object).

// When implementing a function pointer, the implementation should be static
// (internal linkage) and should match the name of the function
// pointer, with the added prefix '_' (e.g. _ne_core_event_peek).

// Any time we refer to a char* or const char*, it will be null terminated.
// All buffers of arbitrary sized data (non-null terminated) will be uint8_t*.

// All functions are non-blocking, unless otherwise noted.

// All functions are thread safe to call and their
// operations may be treated as atomic, unless otherwise noted.

// Functions will not typically validate all inputs. This may be implemented
// as a seperate validation layer (results in #NE_CORE_RESULT_INVALID_PARAMETER.
// This includes checking if all free/close calls are passed valid pointers,
// misuse of special thread functions, etc.

// All functions take an uint64_t* parameter as the first argument.
// If the argument is not null, it will be filled out with either an error code
// or #NE_CORE_RESULT_SUCCESS if it succeeded.

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

// It is illegal to call any ne_xxxx function pre-main.

// Each result code and permission should be a randomly generated uint64_t to
// avoid collisions.

/// Major version of the library (bumped for breaking changes).
#define NE_CORE_MAJOR 0
/// Minor version of the library (bumped for added features).
#define NE_CORE_MINOR 0

// The following are tags and are only used for documentation purposes:

/// All functions can return the following result codes and therefore will not
/// be documented on each individual function:
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_ALLOCATION_FAILED:
///     Not enough system memory or address space.
///   - #NE_CORE_RESULT_INVALID_PARAMETER:
///     A parameter was checked and its value deemed invalid.
///   - #NE_CORE_RESULT_INTERNAL_ERROR:
///     A critical internal error occurred that should be reported:
///     https://github.com/TrevorSundberg/ne/issues/new
typedef struct ne_core_tag_routine_results ne_core_tag_routine_results;

/// Memory returned is owned by the platform/host. The next call to the
/// same function will free or re-use the previously returned memory.
typedef struct ne_core_tag_platform_owned ne_core_tag_platform_owned;

/// Memory returned is owned by the user. The user must call #ne_core_free on
/// the returned memory.
typedef struct ne_core_tag_user_owned ne_core_tag_user_owned;

/// This function may be called by the user, but must be called by the
/// platform internally. If the function pointer is replaced, it is undefined
/// behavior to call any other ne function at that time as the platform may
/// not expect it.
typedef struct ne_core_tag_internally_called ne_core_tag_internally_called;

/// Null is considered a valid value for the corresponding pointer.
typedef struct ne_core_tag_nullable ne_core_tag_nullable;

/// The function must only be called from the main thread. If the function
/// outputs a struct with function pointers (such as #ne_core_stream or
/// #ne_core_enumerator) then it is implied that those function pointers are
/// also main thread only. When applied to a callback, this means the callback
/// will always occur on the main thread.
typedef struct ne_core_tag_main_thread_only ne_core_tag_main_thread_only;

/// It is not possible for a function pointer within the struct to result in
/// #NE_CORE_RESULT_NOT_SUPPORTED or #NE_CORE_RESULT_PERMISSION_DENIED because
/// it is impossible to get a valid struct instance from an unsupported or
/// denied package.
typedef struct ne_core_tag_struct_results ne_core_tag_struct_results;

/// Intrinsic functions are not treated as function pointers because they are
/// expected to be implemented directly by the compiler and heavily optimized.
/// They also do not return the typical result codes.
typedef struct ne_core_tag_intrinsic ne_core_tag_intrinsic;

/// The first call to be made in an ne application. Static initialization in
/// llvm will occur before this call. When relying on libc, 'main' will be
/// invoked at this time. The program will only terminate after when this call
/// is finished and all queued tasks are completed.
/// @param argc
///   The number of parameters passed.
/// @param argv
///   An array with each paramter passed in.
/// @return
///   An arbitrary process return code (0 indicates success).
NE_CORE_API int32_t ne_core_main(int32_t argc, char *argv[]);

/// The operation completed successfully. This is a routine result and be output
/// by any ne function (see #ne_core_tag_routine_results).
#define NE_CORE_RESULT_SUCCESS 0x0000000000000001

/// The sub-api was not implemented and so the function cannot be called. This
/// is a routine result and be output by any ne function (see
/// #ne_core_tag_routine_results).
#define NE_CORE_RESULT_NOT_SUPPORTED 0xd4f8b0d7d966bda5

/// An attempt to allocate memory failed because the system does not have enough
/// memory or addressable space. This is a routine result and be output by any
/// ne function (see #ne_core_tag_routine_results).
#define NE_CORE_RESULT_ALLOCATION_FAILED 0xc1aa191cc1ccb51c

/// A parameter that was passed in was not valid for the function. By default,
/// all ne APIs will not do any extra error checking unless it is of free cost.
/// Helper libraries may patch over functions and apply thier own checking that
/// may result in this error (a debug layer for example). This is a routine
/// result and be output by any ne function (see #ne_core_tag_routine_results).
#define NE_CORE_RESULT_INVALID_PARAMETER 0x9b93094f88a11b54

/// A special error code that should never surface to any application and occurs
/// only when a dramatic internal error occurs. These errors should be reported
/// with: https://github.com/TrevorSundberg/ne/issues/new
/// This is a routine result and be output by any ne function (see
/// #ne_core_tag_routine_results).
#define NE_CORE_RESULT_INTERNAL_ERROR 0x5bdb7f43fd650811

/// An error occurred. The function documentation may have more information
/// about why this result is returned.
#define NE_CORE_RESULT_ERROR 0x625f2ae2c625f5dc

/// An error occurred on the stream (such as a disk error, file deleted,
/// socket closed, etc.), or #ne_core_stream.is_valid returns NE_CORE_FALSE.
#define NE_CORE_RESULT_STREAM_ERROR 0x72c55e0140f35004

/// An attempt was made to seek before the begginning of the stream (negative
/// position) or beyond the capability of the stream.
#define NE_CORE_RESULT_STREAM_OUT_OF_BOUNDS 0x549d2cb6d0af5065

/// An attempt to call a function was denied because the application never
/// requested permission for the sub-api, or permission was requested but
/// denied.
#define NE_CORE_RESULT_PERMISSION_DENIED 0xf44cf12e4feba8a0

/// Determines if this package is fully supported on this platform.
/// @param result
///   - #ne_core_tag_routine_results.
/// @return
///   #NE_CORE_TRUE if supported, #NE_CORE_FALSE otherwise.
NE_CORE_API ne_core_bool (*ne_core_supported)(uint64_t *result);

/// Returns the name of the current platform. When compiling as a portable
/// executable, #NE_CORE_PLATFORM_NAME will always return the string "NE" and
/// therefore this function must be called to dynamically retrieve the name of
/// the platform. The value returned will always match the names that
/// #NE_CORE_PLATFORM_NAME can produce. Each name is guaranteed to be at least 8
/// bytes long, and the first 8 bytes are guaranteed to be unqiue per platform.
/// This allows you to reinterpret the string as if it were a uint64_t for
/// efficient platform checks. It is recommended to avoid using this function if
/// possible to maximize platform independence. Do not free the returned memory.
/// @param result
///   - #ne_core_tag_routine_results.
/// @return
///   The name of the current platform that we are running on, or nullptr on
///   error. The possible names are the same as #NE_CORE_PLATFORM_NAME.
///   - #ne_core_tag_platform_owned.
NE_CORE_API const char *(*ne_core_get_platform_name)(uint64_t *result);

/// Returns a globally unique identifier for the application. The returned
/// identifier is guaranteed to only contain lower-case ASCII letters [a-z],
/// numbers [0-9], and dashes '-'. Do not free the returned memory.
/// @param result
///   - #ne_core_tag_routine_results.
/// @return
///   The globally unique identifier, or nullptr on error.
///   - #ne_core_tag_platform_owned.
NE_CORE_API const char *(*ne_core_get_application_guid)(uint64_t *result);

/// Reserved for future use.
typedef struct ne_core_exit_event ne_core_exit_event;

/// Signature for the callback used in #ne_core_on_exit.
typedef void (*ne_core_exit_callback)(const ne_core_exit_event *event,
                                      const void *user_data);

/// Registers callbacks that will be invoked when the program is exiting or
/// when #ne_core_exit is called. The callbacks are executed in reverse order
/// from how they are added (LIFO). During these callbacks it is legal to invoke
/// any ne function, however any tasks queued during this point (such as
/// #ne_core_request_frame) will not execute. Invoking #ne_core_on_exit during
/// an #ne_core_exit_callback will result in the callback being invoked
/// immediately after.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param callback
///   A user provided callback that will be invoked upon exiting.
/// @param user_data
///   Opaque data provided by the user that will be passed to the \p callback.
NE_CORE_API void (*ne_core_on_exit)(uint64_t *result,
                                    ne_core_exit_callback callback,
                                    const void *user_data);

/// The state of a permission. Defaults to #ne_core_permission_state_prompt for
/// all permissions.
typedef enum ne_core_permission_state NE_CORE_ENUM
{
  /// The user has yet to be prompted for the permission (denied).
  ne_core_permission_state_prompt = 0,

  /// The user or host explicitly granted permission.
  ne_core_permission_state_granted = 1,

  /// The user or host explicitly denied permission.
  ne_core_permission_state_denied = 2,

  /// The permission is unsupported or invalid.
  ne_core_permission_state_invalid = 3,

  /// Enum entry count.
  ne_core_permission_state_max = 4,

  /// Force enums to be 32-bit.
  ne_core_permission_state_force_size = 0x7FFFFFFF
} ne_core_permission_state;

/// Forward declaration and alias.
typedef struct ne_core_permission_event ne_core_permission_event;
/// Describes the state of a given permission.
struct ne_core_permission_event
{
  /// The permission token that was requested.
  uint64_t permission;

  /// The current state of the permission (the state we changed to).
  ne_core_permission_state current_state;

  /// The perivous state of the permission. Note that this may be the
  /// same as #current_state in cases where the user requested permission
  /// multiple times, even though it was already granted.
  ne_core_permission_state previous_state;
};

/// Signature for the callback used in #ne_core_request_permission and
/// #ne_core_request_permission.
typedef void (*ne_core_permission_callback)(
    const ne_core_permission_event *event, const void *user_data);

/// Queries the state of a given permission and invokes the provided
/// callback when the state is discovered.
/// All permissions will be #ne_core_permission_state_prompt by default unless
/// they were previously granted or denied. If an invalid or unsupported
/// permission is given the callback will be invoked with a result of
/// #ne_core_permission_state_invalid.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param permission
///   The permission that we want to check the state of.
/// @param callback
///   A user provided callback that will be invoked when the state of the
///   permission is discovered.
/// @param user_data
///   Opaque data provided by the user that will be passed to the \p callback.
NE_CORE_API void (*ne_core_query_permission)(
    uint64_t *result,
    uint64_t permission,
    ne_core_permission_callback callback,
    const void *user_data);

/// Requests permission from the user or host to access functions in a package.
/// For each permission in the array the callback will be called, even if the
/// state of the permission did not changed (e.g. it was already granted).
/// Permissions may be granted for some tokens and not others. If an invalid or
/// unsupported permission is given the callback will be invoked with a result
/// of #ne_core_permission_state_invalid. Note that even after a permission is
/// granted it may be revoked due to external events, such as a user changing
/// application permission settings or operating system policy, therefore you
/// should always call #ne_core_query_permission before usage.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param permissions
///   An array of permissions that the user wants to request. Platforms will
///   attempt to show all permission requests in a single dialog.
///   The \p count parameter specifies how many are in the array.
/// @param count
///   The number of permissions in the \p permissions array.
/// @param message
///   An optional message that a platform may show to the user in a request
///   dialog.
///   - #ne_core_tag_nullable.
/// @param callback
///   A user provided callback that will be invoked when the user or host
///   grants or denies permission. The callback will be invoked for each
///   permission in the \p permissions array (\p count times).
/// @param user_data
///   Opaque data provided by the user that will be passed to the \p callback.
NE_CORE_API void (*ne_core_request_permission)(
    uint64_t *result,
    const uint64_t permissions[],
    uint64_t count,
    const char *message,
    ne_core_permission_callback callback,
    const void *user_data);

/// This is the first function that should be called to test if functionality is
/// working. The application will display a Hello World message in a console,
/// notification, or non-blocking/non-modal message box. This is also used as an
/// introductory function to demonstrate that an application is working. This
/// function should be non-blocking.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
NE_CORE_API void (*ne_core_hello_world)(uint64_t *result);

/// Reserved for future use.
typedef struct ne_core_frame_event ne_core_frame_event;

/// Signature for the callback used in #ne_core_request_frame.
typedef void (*ne_core_frame_callback)(const ne_core_frame_event *event,
                                       const void *user_data);

/// Requests the callback to be called after a frame has gone by as soon as
/// possible. To run on any cooperative multi-tasked system (e.g. Emscripten) we
/// must rely on event callbacks to drive the application rather than our own
/// infinite loop in main, thus yielding time back to the OS/browser. This
/// function is supported by non-cooperative multi-tasked systems.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param callback
///   A user provided callback that will be invoked on the next frame.
/// @param user_data
///   Opaque data provided by the user that will be passed to the \p callback.
NE_CORE_API void (*ne_core_request_frame)(uint64_t *result,
                                          ne_core_frame_callback callback,
                                          const void *user_data);

/// Terminates the program immediately (control flow will not continue after
/// this call). When called all callbacks registered by #ne_core_on_exit will be
/// called and any open streams will be closed and global destructors will be
/// invoked.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param return_code
///   The process exit code to be returned (0 typically indicates success).
NE_CORE_API void (*ne_core_exit)(uint64_t *result, int32_t return_code);

/// Tells the application to log an error. This is used for unit tests.
/// If possible the platform will print to the standard error stream.
///   - #ne_core_tag_main_thread_only.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param error_result
///   The result that caused the error. If the error was not caused by a result,
///   use #NE_CORE_RESULT_INVALID.
/// @param file
///   The name or path to the file the error occurred in (typically __FILE__).
/// @param line
///   The line number that the error occurred on (typically __LINE__).
/// @param message
///   A custom message that will be printed.
NE_CORE_API void (*ne_core_error)(uint64_t *result,
                                  uint64_t error_result,
                                  const char *file,
                                  int64_t line,
                                  const char *message);

/// Attempts to allocate memory from the host or returns NE_CORE_NULL on
/// failure.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param size_bytes
///   The number of bytes we are requesting to allocate.
/// @return
///   The base memory address of the allocated region, or NE_CORE_NULL on
///   failure.
///   - #ne_core_tag_user_owned.
NE_CORE_API uint8_t *(*ne_core_allocate)(uint64_t *result, uint64_t size_bytes);

/// Frees memory that was returned from #ne_core_allocate. If null is passed
/// for \p memory then this function will do nothing.
/// @param result
///   - #ne_core_tag_routine_results.
/// @param memory
///   The base memory address of the allocated region we wish to free, or null.
NE_CORE_API void (*ne_core_free)(uint64_t *result, void *memory);

/// Forward declaration and alias.
typedef struct ne_core_enumerator ne_core_enumerator;
/// An interface for enumerating over any container or generated set of items.
/// Functions that construct enumerators should denote the output value's
/// type when calling #dereference. All functions filled out will be non-null.
///   - #ne_core_tag_struct_results.
struct ne_core_enumerator
{
  /// Checks to see if the enumerator is empty (has no more values within it).
  /// @param result
  ///   - #ne_core_tag_routine_results.
  /// @param self
  ///   The struct that owns the function pointer.
  /// @return
  ///   NE_CORE_TRUE if empty or NE_CORE_FALSE if it has items.
  ne_core_bool (*empty)(uint64_t *result, const ne_core_enumerator *self);

  /// Advanced the enumerator by one element. It is illegal to call this
  /// on an empty enumerator.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  /// @param self
  ///   The struct that owns the function pointer.
  void (*advance)(uint64_t *result, ne_core_enumerator *self);

  /// Reads the current value of the enumerator. The type output by
  /// each enumerator is specified by the API call that outputs the enumerator.
  /// It is illegal to call this on an empty enumerator.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  /// @param self
  ///   The struct that owns the function pointer.
  /// @param value_out
  ///   Outputs the current value of the enumerator. The type that is output is
  ///   defined by the API that creates the enumerator.
  void (*dereference)(uint64_t *result,
                      const ne_core_enumerator *self,
                      void *value_out);

  /// Frees and resources or memory held by the enumerator.
  /// After this all calls to the #ne_core_enumerator are undefined.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  /// @param self
  ///   The struct that owns the function pointer.
  void (*free)(uint64_t *result, ne_core_enumerator *self);

  /// Opaque data used by the platform / implementation.
  uint8_t opaque[NE_CORE_OPAQUE_SIZE];
};

/// When seeking a stream the seek is always relative to a given origin.
typedef enum ne_core_stream_seek_origin NE_CORE_ENUM
{
  /// Seek to an offset relative to the beginning of the stream.
  /// Typically you will use positive values to move forwards.
  ne_core_stream_seek_origin_begin = 0,
  /// Seek to an offset relative to the current position of the stream.
  /// Typically you may use positive or negative values to move the posiiton.
  ne_core_stream_seek_origin_current = 1,
  /// Seek to an offset relative to the end of the stream.
  /// Typically you will use negative values to move backwards.
  ne_core_stream_seek_origin_end = 2,

  /// Number of entries in the enum.
  ne_core_stream_seek_origin_max = 3,
  /// Force enums to be 32-bit.
  ne_core_stream_seek_origin_force_size = 0x7FFFFFFF
} ne_core_stream_seek_origin;

/// Forward declaration and alias.
typedef struct ne_core_stream ne_core_stream;
/// An interface for reading and writing to any stream of bytes.
/// Any function that outputs a stream must specify the exact set of
/// functionality that is expected. Only the functions that are expected should
/// be filled out, all others will be nulled out by the platform. If the
/// funciton that creates the stream returns an error, the stream will be left
/// as is. Streams may support the following operations:
///   - #read.
///   - #write.
///   - #flush.
///   - #get_position.
///   - #get_size.
///   - #seek.
///   - #is_valid.
///   - #free.
/// The stream structure must be passed in to each function. It is undefined
/// behavior to pass in the wrong stream.
///   - #ne_core_tag_struct_results.
struct ne_core_stream
{
  /// When blocking, this always reads the entire size specified unless the end
  /// of the stream or an error occurs. When non-blocking, this will read as
  /// much as possible and will not wait for more. Reaching the end of a stream
  /// does not produce an error result. Returns how much data was read.
  ///   - #ne_core_tag_nullable.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  ///   - #NE_CORE_RESULT_STREAM_ERROR:
  ///     An error occurred on the stream.
  /// @param self
  ///   The struct that owns the function pointer.
  /// @param buffer
  ///   A buffer of \p size bytes that will receive the read data.
  /// @param size
  ///   The size of the \p buffer array in bytes.
  /// @param allow_blocking
  ///   NE_CORE_TRUE if it may block or NE_CORE_FALSE if it must not block.
  /// @return
  ///   The number of bytes that were read into the \p buffer, up to \p size.
  uint64_t (*read)(uint64_t *result,
                   ne_core_stream *self,
                   void *buffer,
                   uint64_t size,
                   ne_core_bool allow_blocking);

  /// When blocking, this always writes the entire size specified unless the end
  /// of the stream or an error occurs. When non-blocking, this will writes as
  /// much as possible and will not wait for more. Returns how much data was
  /// written.
  ///   - #ne_core_tag_nullable.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  ///   - #NE_CORE_RESULT_STREAM_ERROR:
  ///     An error occurred on the stream.
  /// @param self
  ///   The struct that owns the function pointer.
  /// @param buffer
  ///   A buffer of \p size bytes that holds the data to be written.
  /// @param size
  ///   The size of the \p buffer array in bytes.
  /// @param allow_blocking
  ///   NE_CORE_TRUE if it may block or NE_CORE_FALSE if it must not block.
  /// @return
  ///   The number of bytes that were written from the \p buffer, up to \p size.
  uint64_t (*write)(uint64_t *result,
                    ne_core_stream *self,
                    const void *buffer,
                    uint64_t size,
                    ne_core_bool allow_blocking);

  /// Writes any buffered data to the underlying hardware stream. Flushing may
  /// block if data is unable to be written at the time.
  ///   - #ne_core_tag_nullable.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  ///   - #NE_CORE_RESULT_STREAM_ERROR:
  ///     An error occurred on the stream.
  /// @param self
  ///   The struct that owns the function pointer.
  void (*flush)(uint64_t *result, ne_core_stream *self);

  /// Gets the position relative to the base of the stream (0 indicates the
  /// beginning).
  ///   - #ne_core_tag_nullable.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  ///   - #NE_CORE_RESULT_STREAM_ERROR:
  ///     An error occurred on the stream.
  /// @param self
  ///   The struct that owns the function pointer.
  /// @return
  ///   The current position of the stream.
  uint64_t (*get_position)(uint64_t *result, const ne_core_stream *self);

  /// Gets the current size of the stream. The size may change over time such as
  /// when reading from a file and data is written to the same file externally.
  ///   - #ne_core_tag_nullable.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  ///   - #NE_CORE_RESULT_STREAM_ERROR:
  ///     An error occurred on the stream.
  /// @param self
  ///   The struct that owns the function pointer.
  /// @return
  ///   The current size of the stream.
  uint64_t (*get_size)(uint64_t *result, const ne_core_stream *self);

  /// Moves the position of the stream to a specified location realtive to
  /// either the beginning, current position, or end. Seeking before the
  /// begginning of the stream will result in
  /// #NE_CORE_RESULT_STREAM_OUT_OF_BOUNDS. Seeking after the end is explicitly
  /// allowed. If a stream is seeked to a position well beyond the end and a
  /// write occurs, the streams will fill the gap with 0 bytes. It may do this
  /// by explicitly writing the bytes, or it may store metadata knows where the
  /// holes/gaps are located (non-contiguous).
  ///   - #ne_core_tag_nullable.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  ///   - #NE_CORE_RESULT_STREAM_ERROR:
  ///     An error occurred on the stream.
  ///   - #NE_CORE_RESULT_STREAM_OUT_OF_BOUNDS:
  ///     Seek position was before the begginning or beyond capbility.
  /// @param self
  ///   The struct that owns the function pointer.
  /// @param origin
  ///   The \p position is relative to this origin. The absolute position is
  ///   calculated as if it were \p origin + \p position.
  /// @param position
  ///   An offset from the \p origin (or an absolute position if \p origin is
  ///   #ne_core_stream_seek_origin_begin).
  /// @return
  ///   The absolute position in the stream that we seeked to (relative to the
  ///   beginning). This position may have been clamped.
  uint64_t (*seek)(uint64_t *result,
                   ne_core_stream *self,
                   ne_core_stream_seek_origin origin,
                   int64_t position);

  /// Indicates the stream is usable. If this returns false, all other function
  /// calls will typically result in #NE_CORE_RESULT_STREAM_ERROR. It is illegal
  /// to call this on invalid or freed streams.
  ///   - #ne_core_tag_nullable.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  /// @param self
  ///   The struct that owns the function pointer.
  /// @return
  ///   NE_CORE_TRUE if the stream is valid and usable, otherwise NE_CORE_FALSE.
  ne_core_bool (*is_valid)(uint64_t *result, const ne_core_stream *self);

  /// Frees any resources or memory held by the stream. After this all calls to
  /// the #ne_core_stream are undefined, including #is_valid.
  ///   - #ne_core_tag_nullable.
  /// @param result
  ///   - #ne_core_tag_routine_results.
  /// @param self
  ///   The struct that owns the function pointer.
  void (*free)(uint64_t *result, ne_core_stream *self);

  /// Opaque data used by the platform / implementation.
  uint8_t opaque[NE_CORE_OPAQUE_SIZE];
};

// The following intrinsic functions are intended to be defined by the compiler.

/// Sets a contiguous block of memory to a specified \p value, starting with the
/// address given by \p memory and ending at \p memory + \p size (in bytes).
///   - #ne_core_tag_intrinsic.
/// @param memory
///   The base address of memory that we want to set.
/// @param value
///   The value we want writtten to each byte.
/// @param size
///   Number of bytes in memory we want to set, starting from \p memory.
NE_CORE_API void ne_core_memory_set(void *memory, uint8_t value, uint64_t size);

/// Copies a contiguous block of memory from the \p source to the \p
/// destination. The contiguous blocks from \p source and \p destination must
/// not overlap.
///   - #ne_core_tag_intrinsic.
/// @param destination
///   The address we wish to copy to.
/// @param source
///   The address we wish to copy from.
/// @param size
///   Number of bytes in memory we want to copy.
NE_CORE_API void ne_core_memory_copy(void *destination,
                                     const void *source,
                                     uint64_t size);

/// Compares two contiguous blocks of memory.
///   - #ne_core_tag_intrinsic.
/// @param a
///   The first address whose bytes we wish to compare.
/// @param b
///   The second address whose bytes we wish to compare.
/// @param size
///   Number of bytes in memory we want to compare.
/// @return
///   Returns 0 if the memory is equal, less than 0 if the bytes in \p a are
///   less than the bytes in \p b, and greater than 0 if the bytes in \p a are
///   greater than the bytes in \p b.
NE_CORE_API int64_t ne_core_memory_compare(const void *a,
                                           const void *b,
                                           uint64_t size);
