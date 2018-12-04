/// @file
/// @see LICENSE.md MIT License Copyright (c) 2018 Trevor Sundberg

#pragma once

#if defined(__cplusplus)
/// Enums must be 32-bit size in C++.
#define NE_CORE_ENUM : uint32_t
/// Use C name mangling.
#define NE_CORE_C_LINKAGE "C"
#else
/// We can't force enum sizes, so we make an enum constant called 'force_size'
/// that is as big as a 32-bit number.
#define NE_CORE_ENUM
/// We're already in C, so we use C name mangling.
#define NE_CORE_C_LINKAGE
#endif

// This should always be defined by the build system, however
// if a user grabbed the header directly and included it in their
// own project then it would not be defined. This is acceptable
// practice if the user is only including the header, but to build
// the c files (or a library) the user/build system must specify.
#if !defined(NE_CORE_PLATFORM_NAME)
/// The name of the platform we're currently on. When compiling the platform
/// specific libraries, this will be the name of the platform. When compiling
/// to LLVM into an ne-executable, this will always be "NE".
#define NE_CORE_PLATFORM_NAME "Unknown"
/// A conditional constant for detecing which platform we are on.
#define NE_CORE_PLATFORM_UNKNOWN 1
#endif

#if defined(NE_CORE_PLATFORM_NE)
// This attribute is valid in clang/llvm (the only compiler we use for
// NE_CORE_PLATFORM_NE).
/// Declares external linkage with a special type of linking that means
/// an *invalid* dummy will be stubbed in if it is not found/linked.
#define NE_CORE_API extern NE_CORE_C_LINKAGE __attribute__((section("ne")))

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
#define NE_CORE_API extern NE_CORE_C_LINKAGE

// If we're not compiling with NE_CORE_PLATFORM_NE defined, it means we may be
// on any compiler and we are most likely compiling with libc. Because of this,
// we must rely on each compilers definition of sized types.
#if defined(__cplusplus)
#include <cstdint>
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
#else
#include <stdint.h>
#endif
#endif

/// A boolean value that may hold NE_CORE_TRUE or NE_CORE_FALSE. All other
/// values are invalid.
typedef uint8_t ne_core_bool;

/// Represents a boolean true value, to be used with #ne_core_bool.
#define NE_CORE_TRUE 1
/// Represents a boolean false value, to be used with #ne_core_bool.
#define NE_CORE_FALSE 0

/// Indicates a null pointer.
#define NE_CORE_NULL 0

// typedef uint64_t ne_core_primitive_type;
// static const ne_core_primitive_type ne_core_primitive_type_int8 = 0;
// static const ne_core_primitive_type ne_core_primitive_type_int16 = 1;
// static const ne_core_primitive_type ne_core_primitive_type_int32 = 2;
// static const ne_core_primitive_type ne_core_primitive_type_int64 = 3;
// static const ne_core_primitive_type ne_core_primitive_type_uint8 = 4;
// static const ne_core_primitive_type ne_core_primitive_type_uint16 = 5;
// static const ne_core_primitive_type ne_core_primitive_type_uint32 = 6;
// static const ne_core_primitive_type ne_core_primitive_type_uint64 = 7;
// static const ne_core_primitive_type ne_core_primitive_type_float = 8;
// static const ne_core_primitive_type ne_core_primitive_type_double = 9;
// static const ne_core_primitive_type ne_core_primitive_type_pointer = 10;

/// Encloses one or more statements into a single statement safely.
#define NE_CORE_ENCLOSURE(code)                                                \
  do                                                                           \
  {                                                                            \
    code                                                                       \
  } while (NE_CORE_FALSE)

#if !defined(NDEBUG)
/// Reports an error if a condition is false. Does not abort.
#define NE_CORE_ASSERT(condition, text)                                        \
  NE_CORE_ENCLOSURE(if (!(condition))                                          \
                        ne_core_error(0, __FILE__, __LINE__, (text));)
#else
/// Reports an error if a condition is false. Does not abort.
#define NE_CORE_ASSERT(condition, text)
#endif

/// Reports an error if a condition is true. Does not abort.
#define NE_CORE_ERROR_IF(condition, text) NE_CORE_ASSERT(!(condition), text)

/// Reports an error. Does not abort.
#define NE_CORE_ERROR(text) NE_CORE_ASSERT(0, text)

/// Sets the `result` to a given code if it's available/non-null.
#define NE_CORE_RESULT(code) NE_CORE_ENCLOSURE(if (result) *result = code;)

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
// 'NE_CORE_RESULT_PERMISSION_DENIED'.

// A 'supported' call will indicate that a particular sub-api
// may not exist on some platforms, e.g. 'ne_thread_exists'.
// If a sub-api does not exist, all calls to it are *invalid* and return
// 'NE_CORE_RESULT_NOT_SUPPORTED'. This also includes a 'request_permission'
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

// Any parameter on a function pointer inside a struct named `self` means the
// function takes the struct itself (like a C++ this object).

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
// 'NE_CORE_RESULT_INVALID_PARAMETER'. This includes checking if all free/close
// calls are passed valid pointers, misuse of special thread functions, etc.

// All functions take an uint64_t* parameter as the first argument.
// If the argument is not null, it will be filled out with either an error code
// or 'NE_CORE_RESULT_SUCCESS' if it succeeded.
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

/// Major version of the library (bumped for breaking changes).
#define NE_CORE_MAJOR 0
/// Minor version of the library (bumped for added features).
#define NE_CORE_MINOR 0

// The following are tags and are only used for documentation purposes:

/// Memory returned is owned by the platform/host. The next call to the
/// same function will free or re-use the previously returned memory.
typedef struct ne_core_tag_host_owned ne_core_tag_host_owned;

/// Memory returned is owned by the user. The user must call 'ne_core_free' on
/// the returned memory.
typedef struct ne_core_tag_user_owned ne_core_tag_user_owned;

/// This function may be called by the user, but must be called by the
/// platform internally. If the function pointer is replaced, it is undefined
/// behavior to call any other ne function at that time as the platform may
/// not expect it.
typedef struct ne_core_tag_internally_called ne_core_tag_internally_called;

/// Null is considered a valid value for the corresponding pointer.
typedef struct ne_core_tag_nullable ne_core_tag_nullable;

/// The function must only be called from the main thread.
/// When applied to a callback, this means the callback will always occur on the
/// main thread.
typedef struct ne_core_tag_main_thread_only ne_core_tag_main_thread_only;

/// The first call to be made in an ne application.
/// Static initialization in llvm will occur before this call.
/// When relying on libc, 'main' will be invoked at this time.
/// See #ne_core_set_event_callback to understand program termination.
/// @param argc The number of parameters passed.
/// @param argv An array with each paramter passed in.
/// @return An arbitrary process return code (0 indicates success).
extern int32_t ne_core_main(int32_t argc, char *argv[]);

// Each result code should be a randomly generated uint64_t to avoid collisions.

/// The operation completed successfully.
#define NE_CORE_RESULT_SUCCESS 0x95221af3245a2169

/// A parameter that was passed in was not valid for the function.
#define NE_CORE_RESULT_INVALID_PARAMETER 0x9b93094f88a11b54

/// An attempt to call a function was denied because the application never
/// requested permission for the sub-api, or permission was requested but
/// denied.
#define NE_CORE_RESULT_PERMISSION_DENIED 0xf44cf12e4feba8a0

/// The sub-api was not implemented and so the function cannot be called.
#define NE_CORE_RESULT_NOT_SUPPORTED 0xd4f8b0d7d966bda5

/// An error occurred.
#define NE_CORE_RESULT_ERROR 0x625f2ae2c625f5dc

/// Only used for unit testing to set the result before calling a function.
/// Using this value allows us to validate that the result was properly set.
#define NE_CORE_RESULT_NOT_SET 0x2dac613666331563

/// An attempt to allocate memory failed because the system does not have enough
/// memory or addressable space.
#define NE_CORE_RESULT_ALLOCATION_FAILED 0xc1aa191cc1ccb51c

/// Determines if the package is fully supported on this platform.
/// @param result
///   #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
/// @return #NE_CORE_TRUE if supported, #NE_CORE_FALSE otherwise.
NE_CORE_API ne_core_bool (*ne_core_supported)(uint64_t *result);

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

  /// Number of entries in the enum.
  ne_core_permission_state_max = 3,

  /// Force enums to be 32-bit.
  ne_core_permission_state_force_size = 0x7FFFFFFF
} ne_core_permission_state;

/// Determines if the given package token is supported on this platform.
/// All package permissions will be denied by default unless they were
/// previously granted or the manifest requires them.
/// @param result
///   #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   #NE_CORE_RESULT_NOT_SUPPORTED:
///     The tokens was not supported.
/// @return NE_CORE_TRUE if granted, NE_CORE_FALSE if denied.
NE_CORE_API ne_core_bool (*ne_core_check_permission)(uint64_t *result,
                                                     uint64_t permission);

/// Requests permission from the user or host to use the given packages.
/// For each token the #NE_CORE_EVENT_PERMISSION_GRANTED or
/// #NE_CORE_EVENT_PERMISSION_DENIED events will be sent.
/// Permissions may be granted for some tokens and not others.
/// If the corresponding 'supported' call returned #NE_CORE_FALSE for a given
/// token then it will always result in #NE_CORE_EVENT_PERMISSION_DENIED for
/// that token.
/// @param result
///   #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
NE_CORE_API void (*ne_core_request_permission)(uint64_t *result,
                                               uint64_t permissions[],
                                               uint64_t count,
                                               const char *message,
                                               const void *user_data);

/// Sent whenever permission to use a package is granted. This event may be sent
/// even if a permission was already granted however the event data contains the
/// previous state. This event may come from external events, such as a user
/// granting permission or operating system policy changes.
/// Event: #ne_core_event_permission_data
#define NE_CORE_EVENT_PERMISSION_GRANTED 0x96e939c04a253a6c
/// Sent whenever permission to use a package is denied. This event may be sent
/// even if a permission was already denied however the event data contains the
/// previous state. This event may come from external events, such as a user
/// ervoking permission or operating system policy changes.
/// Event: #ne_core_event_permission_data
#define NE_CORE_EVENT_PERMISSION_DENIED 0xde840cdf2f2d89a7

/// Forward declaration and alias.
typedef struct ne_core_event_permission_data ne_core_event_permission_data;
/// Event data for the events #NE_CORE_EVENT_PERMISSION_GRANTED or
/// #NE_CORE_EVENT_PERMISSION_DENIED.
struct ne_core_event_permission_data
{
  /// The permission token that was requested.
  uint64_t permission;

  /// The state that the permission changed to.
  ne_core_permission_state current_state;

  /// The perivous state of the permission. Note that this may be the
  /// same as #current_state in cases where the user requested permission
  /// multiple times, even though it was already granted.
  ne_core_permission_state previous_state;
};

/// This is the first function that should be called to test if functionality is
/// working. The application will display a Hello World message in a console,
/// notification, or non-blocking/non-modal message box. This is also used as an
/// introductory function to demonstrate that an application is working. This
/// function should be non-blocking.
NE_CORE_API void (*ne_core_hello_world)(uint64_t *result);

/// Requests an event to be sent as soon as possible.
/// To run on any cooperative multi-tasked system (e.g. Emscripten) we must
/// rely on event callbacks to drive the application rather than our own
/// infinite loop, thus yielding time back to the OS/browser. This function is
/// supported by non-cooperative multi-tasked systems.
/// #ne_core_tag_main_thread_only.
NE_CORE_API void (*ne_core_request_frame_event)(uint64_t *result);

/// Sent upon calling #ne_core_request_frame_event.
#define NE_CORE_EVENT_FRAME 0xe3b309b47ac670a4

/// Terminates the program immediately. Flushes and closes any open streams.
/// Invokes global destructors.
NE_CORE_API void (*ne_core_exit)(uint64_t *result, int32_t return_code);

/// Signal an error to the application. This is mostly used for unit tests.
NE_CORE_API void (*ne_core_error)(uint64_t *result,
                                  const char *file,
                                  int64_t line,
                                  const char *message);

/// Allocates memory from the host.
/// Tags: #ne_core_tag_user_owned.
/// @param result
///    #NE_CORE_RESULT_ALLOCATION_FAILED:
///      Not enough system memory or address space, or other system error.
NE_CORE_API uint8_t *(*ne_core_allocate)(uint64_t *result, uint64_t sizeBytes);

/// Frees memory that was returned from #ne_core_allocate.
NE_CORE_API void (*ne_core_free)(uint64_t *result, void *memory);

/// Forward declaration and alias.
typedef struct ne_core_event ne_core_event;
/// Represent an event that occurred at a specific #time.
struct ne_core_event
{
  /// The type of an event is identified by a unique random integer.
  const uint64_t type;

  /// The data of an event is variable sized and therefore must be freed by
  /// #ne_core_free.
  /// Tags: #ne_core_tag_user_owned, #ne_core_tag_nullable.
  void *data;

  /// Events generated by an API call may take a user data parameter that is
  /// retrivable here.
  /// Tags: #ne_core_tag_nullable.
  const void *user_data;

  /// The originating time of the event on this machine in nanoseconds since the
  /// Unix Epoch (00:00:00 Coordinated Universal Time (UTC), January 1st 1970).
  /// Events are ordered by creation therefore this time is always equal to or
  /// greater than the previous event in the queue. This time will always be the
  /// highest precision that the platform may offer.
  uint64_t time;
};

/// Signature for the callback used in #ne_core_set_event_callback.
typedef void (*ne_core_event_callback)(ne_core_event *event,
                                       const void *user_data);

/// Registers a callback to be called when any event process wide event is sent.
/// Events may be sent by the platform during any call to an ne function
/// and may also be sent while yielding to the operating system.
/// The program will exit only if the callback is null or was never set.
/// #ne_core_tag_main_thread_only.
NE_CORE_API void (*ne_core_set_event_callback)(uint64_t *result,
                                               ne_core_event_callback callback,
                                               const void *user_data);
/// Get the data that was passed in to #ne_core_set_event_callback. If
/// #ne_core_set_event_callback was never called, this will return null for both
/// callback and userdata.
/// #ne_core_tag_main_thread_only.
NE_CORE_API void (*ne_core_get_event_callback)(uint64_t *result,
                                               ne_core_event_callback *callback,
                                               const void **user_data);

/// Forward declaration and alias.
typedef struct ne_core_enumerator ne_core_enumerator;
/// An interface for enumerating over any container or generated set of items.
/// Functions that construct enumerators should denote the output value's
/// type when calling #dereference.
struct ne_core_enumerator
{
  /// Checks to see if the enumerator is empty (has no more values within it).
  /// @param result
  ///   NE_CORE_RESULT_SUCCESS:
  ///     The operation completed successfully.
  /// @param self The enumerator that owns the function pointer.
  /// @returns NE_CORE_TRUE if empty or NE_CORE_FALSE if it has items.
  ne_core_bool (*empty)(uint64_t *result, const ne_core_enumerator *self);

  /// Advanced the enumerator by one element.
  /// @param result
  ///   NE_CORE_RESULT_SUCCESS:
  ///     The operation completed successfully.
  /// @param self The enumerator that owns the function pointer.
  void (*advance)(uint64_t *result, ne_core_enumerator *self);

  /// Reads the current value of the enumerator. The type output by
  /// each enumerator is specified by the API call that outputs the enumerator.
  /// @param result
  ///   NE_CORE_RESULT_SUCCESS:
  ///     The operation completed successfully.
  /// @param self The enumerator that owns the function pointer.
  void (*dereference)(uint64_t *result,
                      const ne_core_enumerator *self,
                      void *value_out);

  /// Frees and resources or memory held by the enumerator.
  /// After this all calls to the 'ne_core_enumerator' are undefined.
  /// @param result
  ///   NE_CORE_RESULT_SUCCESS:
  ///     The operation completed successfully.
  /// @param self The enumerator that owns the function pointer.
  void (*free)(uint64_t *result, ne_core_enumerator *self);

  /// Opaque data used by the platform / implementation to represent the
  /// enumerator.
  uint8_t opaque[16];
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
/// Any function that outputs a stream must specify the
/// exact set of functionality that is expected. Only the functions that are
/// expected should be filled out, all others will be nulled out by the
/// platform. The stream structure must be passed in to each function. It is
/// undefined behavior to pass in the wrong stream.
struct ne_core_stream
{
  // When blocking, this always reads the entire size specified unless the end
  // of the stream or an error occurs. When non-blocking, this will read as much
  // as possible and will not wait for more. Note that when it reaches the end
  // of the stream it will still return NE_CORE_RESULT_SUCCESS. Returns how much
  // data was read.
  //    NE_CORE_RESULT_ERROR:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  uint64_t (*read)(uint64_t *result,
                   ne_core_stream *self,
                   void *buffer,
                   uint64_t size,
                   ne_core_bool allow_blocking);

  // When blocking, this always writes the entire size specified unless the end
  // of the stream or an error occurs. When non-blocking, this will writes as
  // much as possible and will not wait for more. Returns how much data was
  // written.
  //    NE_CORE_RESULT_ERROR:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  uint64_t (*write)(uint64_t *result,
                    ne_core_stream *self,
                    const void *buffer,
                    uint64_t size,
                    ne_core_bool allow_blocking);

  // Note that flush may result in a blocking operation if data is unable to be
  // written at the time.
  //    NE_CORE_RESULT_ERROR:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  void (*flush)(uint64_t *result, const ne_core_stream *self);

  //    NE_CORE_RESULT_ERROR:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  uint64_t (*get_position)(uint64_t *result, const ne_core_stream *self);

  //    NE_CORE_RESULT_ERROR:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  uint64_t (*get_size)(uint64_t *result, const ne_core_stream *self);

  // If the stream is no longer usable. If this returns true, all other function
  // calls will result in 'NE_CORE_RESULT_ERROR'.
  ne_core_bool (*is_terminated)(uint64_t *result, const ne_core_stream *self);

  // Seek will clamp a streams position if it attempts to move before the
  // beggining (0) or after the end (get_size).
  //    NE_CORE_RESULT_ERROR:
  //      If an error occurred on the stream (such as a disk error, file
  //      deleted, etc).
  void (*seek)(uint64_t *result,
               ne_core_stream *self,
               ne_core_stream_seek_origin origin,
               uint64_t position);

  // Frees any resources or memory held by the stream.
  // After this all calls to the 'ne_core_stream' are undefined, including
  // 'is_terminated'.
  void (*free)(uint64_t *result, ne_core_stream *self);

  // Opaque data used by the platform / implementation to represent the stream.
  uint8_t opaque[16];
};
