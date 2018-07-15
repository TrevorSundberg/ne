#pragma once
#ifdef __cplusplus
extern "C" {
#endif

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
// ne_true, otherwise all calls are *invalid* and return 'ne_result_permission_denied'.

// An 'supported' call will indicate that a particular sub-api
// may not exist on some platforms, e.g. 'ne_thread_exists'.
// If a sub-api does not exist, all calls to it are *invalid* and return 'ne_result_not_supported'.
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

// All functions also validate all inputs will set the result 'ne_result_invalid_parameter'.
// This includes checking if all free/close calls are passed
// valid pointers, misuse of special thread functions, etc.

// Functions that can fail will take an ne_result* as the first argument.
// If the argument is not null, it will be filled out with either an error code
// or 'ne_result_success' if it succeeded.
// All functions except 'supported' and 'request_permission' functions
// take an ne_result* parameter as the first argument

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

// Return an empty string that can be pointer compared.
const char* ne_empty_string(void);

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
static const ne_result ne_result_success = 0;
static const ne_result ne_result_invalid_parameter = 1;
static const ne_result ne_result_permission_denied = 2;
static const ne_result ne_result_not_supported = 3;
static const ne_result ne_result_allocation_failed = 4;
static const ne_result ne_result_io_error = 5;
static const ne_result ne_result_clipboard_texttype_not_supported = 6;
static const ne_result ne_result_mutex_locked = 7;
static const ne_result ne_result_count = 8;

/// Version:

static const uint64_t ne_version_header_major = 1;
static const uint64_t ne_version_header_minor = 0;

// Get the major version of the API. Between major versions breaking changes are applied.
uint64_t ne_version_major(ne_result* result);

// Get the minor version of the API. Between minor versions features are added.
uint64_t ne_version_minor(ne_result* result);

// Returns a the name of the current platform.
// "Windows", "Linux", "Android", "iOS", "Mac OS X", etc.
const char* ne_version_platform(ne_result* result);

// Returns a the name of the current sandbox application.
// This can be used to detect specific sandboxes when using sandbox targeted functions such as
// 'ne_package_get_sandbox' or 'ne_sandbox_communicate'.
const char* ne_version_sandbox(ne_result* result);

/// Memory:
//    ne_result_allocation_failed:
//      Not enough system memory or address space, or other system error.
void* ne_memory_allocate(ne_result* result, uint64_t sizeBytes);

//    ne_result_invalid_parameter:
//      The exact memory pointer in was not returned from 'ne_memory_allocate'.
void ne_memory_free(ne_result* result, void* memory);

typedef uint64_t ne_memory_access;
static const ne_memory_access ne_memory_access_invalid = 0;
static const ne_memory_access ne_memory_access_read = 1;
static const ne_memory_access ne_memory_access_write = 2;
static const ne_memory_access ne_memory_access_readwrite = 3;
static const ne_memory_access ne_memory_access_count = 4;
ne_memory_access ne_memory_query(ne_result* result, const void* memory);


/// Timers:

// A platform may support multiple types of timers, ordered from highest to lowest frequency.
// The minimum frequency is a timer that increments every second.
// The value returned will always be at least 1.
uint64_t ne_timer_count(ne_result* result);

// Get how many ticks have passed for a given timer.
// At the start of the application the ticks will be will be 0.
// The valid timer index is from [0, ne_timer_count), ordered from highest to lowest frequency.
uint64_t ne_timer_ticks(ne_result* result, uint64_t timer);

// Get the frequency of a specific timer (how many ticks per second, non-zero).
// The valid timer index is from [0, ne_timer_count), ordered from highest to lowest frequency.
uint64_t ne_timer_frequency(ne_result* result, uint64_t timer);


/// File IO:

typedef struct ne_file ne_file;

// This operation is non-blocking and will return how much was read.
//    ne_result_invalid_parameter:
//      If the file was a write only file or invalid file pointer.
//    ne_result_io_error:
//      If an error occurred on reading the file.
uint64_t ne_file_read(ne_result* result, ne_file* file, void* buffer, uint64_t size);

// This operation is non-blocking and will return how much was written.
//    ne_result_invalid_parameter:
//      If the file was a read only file or invalid file pointer.
//    ne_result_io_error:
//      If an error occurred on writing the file.
uint64_t ne_file_write(ne_result* result, ne_file* file, const void* buffer, uint64_t size);

//    ne_result_invalid_parameter:
//      If the file was a read only file or invalid file pointer.
//    ne_result_io_error:
//      If an error occurred on writing the file.
void ne_file_flush(ne_result* result, ne_file* file);


uint64_t ne_file_get_position(ne_result* result, ne_file* file);


typedef uint64_t ne_file_seek_origin;
static const ne_file_seek_origin ne_file_seek_origin_begin = 0;
static const ne_file_seek_origin ne_file_seek_origin_current = 1;
static const ne_file_seek_origin ne_file_seek_origin_end = 2;
static const ne_file_seek_origin ne_file_seek_origin_count = 3;

void ne_file_seek(ne_result* result, ne_file* file, ne_file_seek_origin origin, uint64_t position);

void ne_file_free(ne_result* result, ne_file* file);

/// Input / Output:

ne_file* ne_io_get_input(ne_result* result);
ne_file* ne_io_get_output(ne_result* result);

/// File System:

ne_bool ne_filesystem_supported(ne_result* result);

ne_bool ne_filesystem_request_permission(ne_result* result);

typedef uint64_t ne_filesystem_mode;
static const ne_filesystem_mode ne_filesystem_mode_read = 0;
static const ne_filesystem_mode ne_filesystem_mode_write = 1;
static const ne_filesystem_mode ne_filesystem_mode_append = 2;
static const ne_filesystem_mode ne_filesystem_mode_count = 3;

ne_file* ne_filesystem_open_file(ne_result* result, ne_filesystem_mode mode, const char* filename);

const char* ne_filesystem_get_path_separator(ne_result* result);

// Get the current working directory (all relative paths are relative to this location).
const char* ne_filesystem_get_working_directory(ne_result* result);

// Set the current working directory (all relative paths are relative to this location).
const char* ne_filesystem_set_working_directory(ne_result* result, const char* directory);

// Get the directory that the executable lives in.
const char* ne_filesystem_get_executable_directory(ne_result* result);

// Get a directory that your application will have permission to write to.
const char* ne_filesystem_get_documents_directory(ne_result* result);

// Get the temporary directory which your application may write temporary files to.
// These files may be periodically cleared.
const char* ne_filesystem_get_temporary_directory(ne_result* result);

typedef uint64_t ne_filesystem_dialog_mode;
static const ne_filesystem_dialog_mode ne_filesystem_dialog_mode_save = 0;
static const ne_filesystem_dialog_mode ne_filesystem_dialog_mode_open = 1;
static const ne_filesystem_dialog_mode ne_filesystem_dialog_mode_count = 2;

typedef uint64_t ne_filesystem_dialog_type;
static const ne_filesystem_dialog_type ne_filesystem_dialog_type_file = 0;
static const ne_filesystem_dialog_type ne_filesystem_dialog_type_directory = 1;
static const ne_filesystem_dialog_type ne_filesystem_dialog_type_count = 2;

struct ne_filesystem_dialog_config
{
  // Uses the '*' wildcard notation as well as separating '\n' for multiple types.
  const char* filter;
  
  // .
  const char* starting_file_name;
  ne_bool multi_select;
  ne_filesystem_dialog_mode mode;
  ne_filesystem_dialog_type type;
};
typedef struct ne_filesystem_dialog_config ne_filesystem_dialog_config;

// Show a file dialog for opening or saving files.
const char* ne_filesystem_dialog(ne_result* result, const ne_filesystem_dialog_config* config);

/// Thread:

ne_bool ne_thread_supported(ne_result* result);

typedef struct ne_thread ne_thread;

typedef int64_t(*ne_thread_routine)(void*);

// Pass in null to get the first thread.
//    ne_result_invalid_parameter:
//      Parameter 'current' does not point to a valid thread.
ne_thread* ne_thread_enumerate_next(ne_result* result, ne_thread* current);

ne_thread* ne_thread_get_current(ne_result* result);

ne_thread* ne_thread_get_main(ne_result* result);

typedef uint64_t ne_thread_mode;
static const ne_thread_mode ne_thread_mode_resume = 0;
static const ne_thread_mode ne_thread_mode_suspend = 1;
static const ne_thread_mode ne_thread_mode_count = 2;

//    ne_result_invalid_parameter:
//      Parameter 'routine' is null or does not point at a valid function.
ne_thread* ne_thread_create(ne_result* result, ne_thread_routine routine, void* data, ne_thread_mode mode, const char* name);

typedef uint64_t ne_thread_wait_time;
static const ne_thread_wait_time ne_thread_wait_time_none = 0;
static const ne_thread_wait_time ne_thread_wait_time_infinite = (ne_thread_wait_time)-1;
ne_bool ne_thread_wait(ne_result* result, ne_thread* thread, ne_thread_wait_time nanoseconds);

// Waits on the thread to be completed and frees the resource.
int64_t ne_thread_free(ne_result* result, ne_thread* thread);

const char* ne_thread_get_name(ne_result* result, ne_thread* thread);

void ne_thread_suspend(ne_result* result, ne_thread* thread);

void ne_thread_resume(ne_result* result, ne_thread* thread);

typedef uint64_t ne_thread_priority;
static const ne_thread_priority ne_thread_priority_low = 0;
static const ne_thread_priority ne_thread_priority_medium = 1;
static const ne_thread_priority ne_thread_priority_high = 2;
static const ne_thread_priority ne_thread_priority_count = 3;
void ne_thread_set_priority(ne_result* result, ne_thread* thread, ne_thread_priority level);

typedef uint64_t ne_thread_storage_id;

// Gets a thread storage value by id. The id can be any value determined by the user.
// It is common to use the address of a static variable reinterpreted as a uint64_t.
// The address is fixed and globally unique.
void* ne_thread_storage_get(ne_result* result, ne_thread_storage_id id);
void ne_thread_storage_set(ne_result* result, ne_thread_storage_id id, void* data);

// Get the number of threads currently running.
uint64_t ne_thread_get_count(ne_result* result);

// Iterate through all the existing threads in order of creation. Passing null will return the first thread.
ne_thread* ne_thread_get_next(ne_result* result, ne_thread* thread);

typedef struct ne_thread_mutex ne_thread_mutex;

// Creates a mutex that can be locked and unlocked for the current process only.
// The mutex allows recursive locks on the same thread.
ne_thread_mutex* ne_thread_mutex_create(ne_result* result);

// Locks the mutex or waits on the mutex to be locked.
// Always returns 'ne_true' when it aquires a lock.
// If the mutex is already locked by the same calling thread,
// this will immediately return 'ne_false'.
//    ne_result_invalid_parameter:
//      The pointer did not point to a valid mutex.
ne_bool ne_thread_mutex_lock(ne_result* result, ne_thread_mutex* mutex, ne_thread_wait_time nanoseconds);

//    ne_result_mutex_locked:
//      The mutex was locked by another thread.
//    ne_result_invalid_parameter:
//      The pointer did not point to a valid mutex.
void ne_thread_mutex_unlock(ne_result* result, ne_thread_mutex* mutex);

// If the mutex is locked by the current thread, it will unlock it and free the resource.
// Once a mutex is freed the pointer becomes invalid.
//    ne_result_mutex_locked:
//      The mutex was locked by another thread.
//    ne_result_invalid_parameter:
//      The pointer did not point to a valid mutex.
void ne_thread_mutex_free(ne_result* result, ne_thread_mutex* mutex);


typedef struct ne_thread_event ne_thread_event;

// Creates an event that multiple threads can wait upon.
// When signalled, automatic reset events will allow a single
// thread through and turn back to the non-signalled state.
// When signalled, manual reset events will allow as many threads
// through until the event is purposely set to the non-signalled state.
void ne_thread_event_create(ne_result* result, ne_bool signaled, ne_bool auto_reset);

// Setting an automatic reset event to 'ne_false' has no effect.
//    ne_result_invalid_parameter:
//      The pointer did not point to a valid event.
void ne_thread_event_set_signalled(ne_result* result, ne_thread_event* event, ne_bool signaled);

//    ne_result_invalid_parameter:
//      The pointer did not point to a valid event.
void ne_thread_event_free(ne_result* result, ne_thread_event* event);

/// Atomic:

/// Intrinsics:

// memset
// sin/cos, etc

/// Events:

/// Packages:

struct ne_package_info
{
  const char* name;
  uint64_t major;
  uint64_t minor;
};

typedef struct ne_package_info ne_package_info;

typedef struct ne_package ne_package;


ne_package* ne_package_enumerate_next(ne_result* result, ne_package* current);

// Requesting a package will result in an event that lets you know when a package is loaded.
void ne_package_request(ne_result* result, ne_package_info* info);

// Checks if a package exists locally (if not, then the package must be requested).
ne_bool ne_package_exists(ne_result* result, ne_package_info* info);

// Loads a package and blocks upon the request if the package does not exist locally.
// If the package fails to load or a request fails, null will be returned.
ne_package* ne_package_load(ne_result* result, ne_package_info* info);

ne_package* ne_package_load_memory(ne_result* result, ne_package_info* info, const uint8_t* memory, uint64_t size);

// Get the package that was first loaded where we started execution.
ne_package* ne_package_get_executable(ne_result* result);

// Get the sandbox package, a special package provided explicitly by the sandbox application.
// Files and functions defined within this package are entirely up to the sandbox and have no standardization.
// This is how extensions are created for applications.
// Sandboxes may even allow third parties to extend the sandbox with their own custom way of loading addons.
ne_package* ne_package_get_sandbox(ne_result* result);

// All files are relative to the package root.
ne_file* ne_package_open_file(ne_result* result, ne_package* package, const char* filename);

void* ne_package_find_symbol(ne_result* result, ne_package* package, const char* name);

void ne_package_close(ne_result* result, ne_package* package);


/// Clipboard:

ne_bool ne_clipboard_supported(ne_result* result);

ne_bool ne_clipboard_request_permission(ne_result* result);

// Common text types that should be handled by this every platform.
// If a particular text type is not supported it should return null.
static const char* const ne_clipboard_texttype_plain = "plain";
static const char* const ne_clipboard_texttype_rtf = "rtf";
static const char* const ne_clipboard_texttype_html = "html";
static const char* const ne_clipboard_texttype_url = "url";

// Each file or directory name will be separated by a single '\n' (no '\r' on any platform).
static const char* const ne_clipboard_texttype_files = "files";

// Returns null if the clipboard does not store this text type. Memory is *platform-owned*.
// May result in 'ne_result_clipboard_texttype_not_supported'.
const char* ne_clipboard_get_text(ne_result* result, const char* texttype);

// May result in 'ne_result_clipboard_texttype_not_supported'.
void ne_clipboard_set_text(ne_result* result, const char* text, const char* texttype);

// Grab an image off the clipboard.
void ne_clipboard_get_image_rgba32f(ne_result* result, ne_image* image);

void ne_clipboard_set_image_rgba32f(ne_result* result, ne_image* image);


/// Window:

/// Keyboard:

ne_bool ne_keyboard_supported(ne_result* result);

ne_bool ne_keyboard_request_permission(ne_result* result);

typedef uint64_t ne_keyboard_index;
static const ne_keyboard_index ne_keyboard_index_a = 0;
static const ne_keyboard_index ne_keyboard_index_b = 1;
static const ne_keyboard_index ne_keyboard_index_c = 2;
static const ne_keyboard_index ne_keyboard_index_count = 3;

// Returns the state of a keyboard key at the exact time of call.
// Note that events don't need to be pumped for this value to change.
// Returning 'ne_true' means the key is down and 'ne_false' means up.
// This represents the physical keyboard, which means it may change
// independent of any application focus.
ne_bool ne_keyboard_is_down(ne_result* result, ne_keyboard_index index);

/// Mouse:

ne_bool ne_mouse_supported(ne_result* result);

ne_bool ne_mouse_request_permission(ne_result* result);

ne_bool ne_mouse_is_touch_emulated(ne_result* result);

typedef uint64_t ne_mouse_index;
static const ne_mouse_index ne_mouse_index_left = 0;
static const ne_mouse_index ne_mouse_index_right = 1;
static const ne_mouse_index ne_mouse_index_middle = 2;
static const ne_mouse_index ne_mouse_index_x1_back = 3;
static const ne_mouse_index ne_mouse_index_x2_forward = 4;
static const ne_mouse_index ne_mouse_index_count = 5;

// Returns the state of a mouse button at the exact time of call.
// Note that events don't need to be pumped for this value to change.
// Returning 'ne_true' means the button is down and 'ne_false' means up.
// This represents the physical mouse, which means it may change
// independent of any application focus.
ne_bool ne_mouse_is_down(ne_result* result, ne_keyboard_index index);

ne_int64_2 ne_mouse_get_position_screen(ne_result* result);

/// Touch:

struct ne_touch_finger
{
  ne_int64_2 position_screen;
  uint64_t pressure;
};
typedef struct ne_touch_finger ne_touch_finger;

ne_bool ne_touch_supported(ne_result* result);

ne_bool ne_touch_is_mouse_emulated(ne_result* result);

uint64_t ne_touch_device_count(ne_result* result);

uint64_t ne_touch_finger_count(ne_result* result, uint64_t device);

void ne_touch_get_finger(ne_result* result, uint64_t device, uint64_t finger, ne_touch_finger* finger_out);

/// Joystick:

/// Gamepad:

/// Haptic:

/// Audio:

/// Printing:

/// CPU Features / Byte Order:

ne_bool ne_cpu_is_little_endian(ne_result* result);
uint64_t ne_cpu_get_cache_line_size(ne_result* result);
uint64_t ne_cpu_get_logical_count(ne_result* result);
uint64_t ne_cpu_get_memory_size(ne_result* result);

/// Power Management:

typedef uint64_t ne_power_state;
static const ne_power_state ne_power_state_powered = 0;
static const ne_power_state ne_power_state_draining = 1;
static const ne_power_state ne_power_state_charging = 2;
static const ne_power_state ne_power_state_charged = 3;
static const ne_power_state ne_power_state_count = 4;

ne_power_state ne_power_get_state(ne_result* result, int64_t* seconds, int64_t* percent);

/// Parallel ports (or other kinds):

/// Socket:

ne_bool ne_socket_supported(ne_result* result);

ne_bool ne_socket_request_permission(ne_result* result);

/// Sandbox:

// Attempt to trigger breakpoint with the sandbox application.
// The purpose of this to allow code to be debugged at a certain point.
// If a debugger is not attached or cannot attach on the fly for the given platform
// then the sandbox may log information and continue execution.
void ne_sandbox_break(ne_result* result);

// Comunicate a custom message with the sandbox.
// This function is entirely sandbox defined and has no explicit behavior.
void ne_sandbox_communicate(ne_result* result, void* buffer, uint64_t size);

// Signal a message to the sandbox application.
void ne_sandbox_message(ne_result* result, const char* message);

// This is the first function that should be called to test if functionality is working.
// The sandbox will display a Hello World message in a console or as a message box.
// This is also used as an introductory function to demonstrate that an application is working.
void ne_sandbox_hello_world();

/// Vulcan:

ne_bool ne_vulcan_supported(ne_result* result);

/// LLVM:

#ifdef __cplusplus
}
#endif
