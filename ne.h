#pragma once

// This header is considered tenative and is a work in progress. It contains the API
// as well as notes about how the API should work. Some of these notes may eventually
// be transcribed into a standards document.

// Compile once. Run Everywhere.

// All strings are UTF8. Assume platform is 64 bit little endian.

// Definitions:
//
//   invalid: Every call to the API functions will return null,
//            0, ne_false, or the binary 0 equivalent.
//
//   sub-api: All functions labeled ne_xxxxx where xxxxx is a name.

// If a 'request_permission' function exists then it must be called and must return
// ne_true, otherwise all calls are *invalid* and return 'ne_result_permission_denied'.

// A 'supported' call will indicate that a particular sub-api
// may not exist on some platforms, e.g. 'ne_thread_exists'.
// If a sub-api does not exist, all calls to it are *invalid* and return 'ne_result_not_supported'.
// This also includes a 'request_permission' function if applicable.
// It is safe to call functions without checking existance as long as return values/results are checked.

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

// The goal is to make behavior as consistent as possible.

// Memory types:
// application-owned
// platform-owned
// user-owned

// NE will also look for the standard forms of 'main' (whether it returns a value or not, or takes argc/argv).

typedef uint8_t ne_bool;

static const ne_bool ne_true = 1;
static const ne_bool ne_false = 0;
static void* const ne_null = 0;

// Return an empty string that can be pointer compared.
const char* ne_empty_string(void);

struct ne_image
{
  uint8_t* buffer;
  uint64_t size_x;
  uint64_t size_y;
};
typedef struct ne_image ne_image;

/// Errors:

typedef uint32_t ne_result;

// Enumeration of status codes.
static const ne_result ne_result_success = 0;
static const ne_result ne_result_invalid_parameter = 1;
static const ne_result ne_result_permission_denied = 2;
static const ne_result ne_result_not_supported = 3;
static const ne_result ne_result_io_error = 4;
static const ne_result ne_result_clipboard_texttype_not_supported = 5;
static const ne_result ne_result_count = 6;

// Get the last error that occurred on this thread (errors are specific to each thread).
// Unless otherwise noted, every function in defined in NE will set the
// state to 'ne_result_success' unless the function fails.
// Does not set 'ne_result' to 'ne_result_success'.
ne_result ne_result_get(void);

// Does not set 'ne_result' to 'ne_result_success'.
void ne_result_set(ne_result code);

// Returns a human readable string from an error code. Memory is *application-owned*.
// If an invalid status code is passed in it will return
// 'ne_empty_string' and the result will be 'ne_result_invalid_parameter'.
// The contents of the string may change per platform and should not be relied upon.
// The memory returned is owned by the platform and will exist for the lifetime of the executable.
const char* ne_result_to_text(ne_result code);


/// Version:

static const uint32_t ne_version_header_major = 1;
static const uint32_t ne_version_header_minor = 0;

// Get the major version of the API. Between major versions breaking changes are applied.
uint32_t ne_version_major(void);

// Get the minor version of the API. Between minor versions features are added.
uint32_t ne_version_minor(void);

// Returns a the name of the current platform.
// "Windows", "Linux", "Android", "iOS", "Mac OS X", etc.
const char* ne_version_platform(void);

// Returns a the name of the current sandbox application.
// This can be used to detect specific sandboxes when using sandbox targeted functions such as
// 'ne_package_get_sandbox' or 'ne_sandbox_communicate'.
const char* ne_version_sandbox(void);

/// Memory:

void* ne_memory_allocate(uint64_t sizeBytes);

void ne_memory_free(void* memory);

typedef uint32_t ne_memory_access;
static const ne_memory_access ne_memory_access_invalid = 0;
static const ne_memory_access ne_memory_access_read = 1;
static const ne_memory_access ne_memory_access_write = 2;
static const ne_memory_access ne_memory_access_readwrite = 3;
static const ne_memory_access ne_memory_access_count = 4;
ne_memory_access ne_memory_query(const void* memory);


/// Timers:

// A platform may support multiple types of timers, ordered from highest to lowest frequency.
// The minimum frequency is a timer that increments every second.
// The value returned will always be at least 1.
uint32_t ne_timer_count(void);

// Get how many ticks have passed for a given timer.
// At the start of the application the ticks will be will be 0.
// The valid timer index is from [0, ne_timer_count), ordered from highest to lowest frequency.
uint64_t ne_timer_ticks(uint32_t timer);

// Get the frequency of a specific timer (how many ticks per second, non-zero).
// The valid timer index is from [0, ne_timer_count), ordered from highest to lowest frequency.
uint64_t ne_timer_frequency(uint32_t timer);


/// File IO:

typedef struct ne_file ne_file;

// This operation is non blocking and will return how much was read.
//    ne_result_invalid_parameter:
//      If the file was a write only file.
//    ne_result_io_error:
//      If an error occurred on reading the file.
uint64_t ne_file_read(ne_file* file, void* buffer, uint64_t size);

// This operation is non blocking and will return how much was written.
//    ne_result_invalid_parameter:
//      If the file was a read only file.
//    ne_result_io_error:
//      If an error occurred on writing the file.
uint64_t ne_file_write(ne_file* file, const void* buffer, uint64_t size);
void ne_file_flush(ne_file* file);
uint64_t ne_file_get_position(ne_file* file);


typedef uint32_t ne_file_seek_origin;
static const ne_file_seek_origin ne_file_seek_origin_begin = 0;
static const ne_file_seek_origin ne_file_seek_origin_current = 1;
static const ne_file_seek_origin ne_file_seek_origin_end = 2;
static const ne_file_seek_origin ne_file_seek_origin_count = 3;

void ne_file_seek(ne_file* file, ne_file_seek_origin origin, uint64_t position);

void ne_file_close(ne_file* file);

/// Input / Output:

ne_file* ne_io_get_input(void);
ne_file* ne_io_get_output(void);

/// File System:

ne_bool ne_filesystem_supported(void);

ne_bool ne_filesystem_request_permission(void);

typedef uint32_t ne_filesystem_mode;
static const ne_filesystem_mode ne_filesystem_mode_read = 0;
static const ne_filesystem_mode ne_filesystem_mode_write = 1;
static const ne_filesystem_mode ne_filesystem_mode_append = 2;
static const ne_filesystem_mode ne_filesystem_mode_count = 3;

ne_file* ne_filesystem_open_file(ne_filesystem_mode mode, const char* filename);

const char* ne_filesystem_get_path_separator(void);

// Get the current working directory (all relative paths are relative to this location).
const char* ne_filesystem_get_working_directory(void);

// Set the current working directory (all relative paths are relative to this location).
const char* ne_filesystem_set_working_directory(const char* directory);

// Get the directory that the executable lives in.
const char* ne_filesystem_get_executable_directory(void);

// Get a directory that your application will have permission to write to.
const char* ne_filesystem_get_documents_directory(void);

// Get the temporary directory which your application may write temporary files to.
// These files may be periodically cleared.
const char* ne_filesystem_get_temporary_directory(void);

typedef uint32_t ne_filesystem_dialog_mode;
static const ne_filesystem_dialog_mode ne_filesystem_dialog_mode_save = 0;
static const ne_filesystem_dialog_mode ne_filesystem_dialog_mode_open = 1;
static const ne_filesystem_dialog_mode ne_filesystem_dialog_mode_count = 2;

typedef uint32_t ne_filesystem_dialog_type;
static const ne_filesystem_dialog_type ne_filesystem_dialog_type_file = 0;
static const ne_filesystem_dialog_type ne_filesystem_dialog_type_directory = 1;
static const ne_filesystem_dialog_type ne_filesystem_dialog_type_count = 2;

struct ne_filesystem_dialog_config
{
  // Uses the '*' wildcard notation as well as separating '\n' for multiple types.
  const char* filter;
  
  // .
  const char* starting_file_name;
  ne_bool allow_multiple;
  ne_filesystem_dialog_mode mode;
  ne_filesystem_dialog_type type;
};

typedef struct ne_filesystem_dialog_config ne_filesystem_dialog_config;

// Show a file dialog for opening or saving files.
const char* ne_filesystem_dialog(const ne_filesystem_dialog_config* config);

/// Thread:

ne_bool ne_thread_supported(void);

typedef struct ne_thread ne_thread;

typedef int64_t(*ne_thread_routine)(void*);

// Pass in null to get the first thread.
ne_thread* ne_thread_enumerate_next(ne_thread* current);

ne_thread* ne_thread_get_current(void);

ne_thread* ne_thread_get_main(void);

typedef uint32_t ne_thread_mode;
static const ne_thread_mode ne_thread_mode_resume = 0;
static const ne_thread_mode ne_thread_mode_suspend = 1;
static const ne_thread_mode ne_thread_mode_count = 2;

ne_thread* ne_thread_create(ne_thread_routine routine, void* data, ne_thread_mode mode, const char* name);

static const uint64_t ne_thread_wait_none = 0;
static const uint64_t ne_thread_wait_infinite = (uint64_t)-1;
ne_bool ne_thread_wait(ne_thread* thread, uint64_t milliseconds);

int64_t ne_thread_close_and_wait(ne_thread* thread);

const char* ne_thread_get_name(ne_thread* thread);

void ne_thread_suspend(ne_thread* thread);

void ne_thread_resume(ne_thread* thread);

typedef uint32_t ne_thread_priority;
static const ne_thread_priority ne_thread_priority_low = 0;
static const ne_thread_priority ne_thread_priority_medium = 1;
static const ne_thread_priority ne_thread_priority_high = 2;
static const ne_thread_priority ne_thread_priority_count = 3;
void ne_thread_set_priority(ne_thread* thread, ne_thread_priority level);

typedef uint64_t ne_thread_storage_id;

// Gets a thread storage value by id. The id can be any value determined by the user.
// It is common to use the address of a static variable reinterpreted as a uint64_t.
// The address is fixed and globally unique.
void* ne_thread_storage_get(ne_thread_storage_id id);
void ne_thread_storage_set(ne_thread_storage_id id, void* data);

// Get the number of threads currently running.
uint64_t ne_thread_get_count(void);

// Iterate through all the existing threads in order of creation. Passing null will return the first thread.
ne_thread* ne_thread_get_next(ne_thread* thread);

/// Thread Sync:

/// Atomic:

/// Intrinsics:

// memset
// sin/cos, etc

/// Events:

/// Packages:

struct ne_package_info
{
  const char* name;
  uint32_t major;
  uint32_t minor;
};

typedef struct ne_package_info ne_package_info;

typedef struct ne_package ne_package;


ne_package* ne_package_enumerate_next(ne_package* current);

// Requesting a package will result in an event that lets you know when a package is loaded.
void ne_package_request(ne_package_info* info);

// Checks if a package exists locally (if not, then the package must be requested).
ne_bool ne_package_exists(ne_package_info* info);

// Loads a package and blocks upon the request if the package does not exist locally.
// If the package fails to load or a request fails, null will be returned.
ne_package* ne_package_load(ne_package_info* info);

ne_package* ne_package_load_memory(ne_package_info* info, const uint8_t* memory, uint64_t size);

// Get the package that was first loaded where we started execution.
ne_package* ne_package_get_executable(void);

// Get the sandbox package, a special package provided explicitly by the sandbox application.
// Files and functions defined within this package are entirely up to the sandbox and have no standardization.
// This is how extensions are created for applications.
// Sandboxes may even allow third parties to extend the sandbox with their own custom way of loading addons.
ne_package* ne_package_get_sandbox(void);

// All files are relative to the package root.
ne_file* ne_package_open_file(ne_package* package, const char* filename);

void* ne_package_find_symbol(ne_package* package, const char* name);

void ne_package_close(ne_package* package);


/// Clipboard:

ne_bool ne_clipboard_supported(void);

ne_bool ne_clipboard_request_permission(void);

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
const char* ne_clipboard_get_text(const char* texttype);

// May result in 'ne_result_clipboard_texttype_not_supported'.
void ne_clipboard_set_text(const char* text, const char* texttype);

// Grab an image off the clipboard.
void ne_clipboard_get_image_rgba32f(ne_image* image);

void ne_clipboard_set_image_rgba32f(ne_image* image);


/// Window:

/// Keyboard:

ne_bool ne_keyboard_supported(void);

void ne_keyboard_get_state(void);

/// Mouse:

ne_bool ne_mouse_supported(void);

ne_bool ne_mouse_is_touch_emulated(void);

/// Touch:

ne_bool ne_touch_supported(void);

ne_bool ne_touch_is_mouse_emulated(void);

/// Joystick:

/// Gamepad:

/// Haptic:

/// Audio:

/// Printing:

/// CPU Features / Byte Order:

ne_bool ne_cpu_is_little_endian(void);
uint64_t ne_cpu_get_cache_line_size(void);
uint64_t ne_cpu_get_logical_count(void);
uint64_t ne_cpu_get_memory_size(void);

/// Power Management:

typedef uint32_t ne_power_state;
static const ne_power_state ne_power_state_powered = 0;
static const ne_power_state ne_power_state_draining = 1;
static const ne_power_state ne_power_state_charging = 2;
static const ne_power_state ne_power_state_charged = 3;
static const ne_power_state ne_power_state_count = 4;

ne_power_state ne_power_get_state(int64_t* seconds, int64_t* percent);

/// Parallel ports (or other kinds):

/// Socket:

ne_bool ne_socket_supported(void);

ne_bool ne_socket_request_permission(void);

/// Sandbox:

// Attempt to trigger breakpoint with the sandbox application.
// The purpose of this to allow code to be debugged at a certain point.
// If a debugger is not attached or cannot attach on the fly for the given platform
// then the sandbox may log information and continue execution.
void ne_sandbox_break(void);

// Comunicate a custom message with the sandbox.
// This function is entirely sandbox defined and has no explicit behavior.
void ne_sandbox_communicate(void* buffer, uint64_t size);

// Signal a message to the sandbox application.
void ne_sandbox_message(const char* message);

// This is the first function that should be called to test if functionality is working.
// The sandbox will display a Hello World message in a console or as a message box.
// This is also used as an introductory function to demonstrate that an application is working.
void ne_sandbox_hello_world();

/// Vulkan:

ne_bool ne_vulkan_supported(void);

/// LLVM:
