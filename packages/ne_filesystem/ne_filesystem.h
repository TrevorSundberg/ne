// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

NE_CORE_BEGIN

#define NE_FILESYSTEM_MAJOR 0
#define NE_FILESYSTEM_MINOR 0
#define NE_FILESYSTEM_PERMISSION 0x9f96d1ab5ae25942

// All file system paths start with '/' and use '/' as a separator between all
// directories. This way is chosen to keep code platform agnostic. To translate
// between the current operating system format use the translate functions.

// If a path contains invalid characters for the platform, then they must be
// escaped in a determanistic way. When a path is escaped it may overlap with a
// non-escaped name, therefore a platform may add uncommon symbols to avoid
// overlap. This is to ensure that all paths work on all platforms (users may
// elect to avoid special symbols, but they will still work).

// We need to extend code to deal with symbolic links and permissions.

#define NE_FILESYSTEM_RESULT_CREATE_ERROR 0xafa525b9a749e711
#define NE_FILESYSTEM_RESULT_FILE_DOES_NOT_EXIST 0xafa525b9a749e711
#define NE_FILESYSTEM_RESULT_DIRECTORY_DOES_NOT_EXIST 0xcdb20618efd664ed
#define NE_FILESYSTEM_RESULT_FILE_ERROR 0x3e9e5f2301e6affc
#define NE_FILESYSTEM_RESULT_DIRECTORY_NOT_EMPTY 0xef556522991e9089

typedef enum ne_filesystem_io NE_CORE_ENUM
{
  ne_filesystem_io_read = 0,
  ne_filesystem_io_write = 1,
  ne_filesystem_io_read_write = 2,
  ne_filesystem_io_append = 3,
  ne_filesystem_io_max = 4,
  ne_filesystem_io_force_size = 0x7FFFFFFF
} ne_filesystem_io;

typedef enum ne_filesystem_create NE_CORE_ENUM
{
  /// A file will never be created (it must already exist).
  ne_filesystem_create_never = 0,
  /// A file will only be created if non exists in its place.
  ne_filesystem_create_if_none = 1,
  /// A new empty file is always created. If a file exists it will be cleared if
  /// possible.
  ne_filesystem_create_always = 2,
  ne_filesystem_create_max = 3,
  ne_filesystem_create_force_size = 0x7FFFFFFF
} ne_filesystem_create;

typedef enum ne_filesystem_share_flags NE_CORE_ENUM
{
  // Others may simultaneously open this file if they are reading.
  ne_filesystem_share_flags_read = 1,
  // Others may simultaneously open this file if they are writing (includes
  // appending).
  ne_filesystem_share_flags_write = 2,
  // Others may delete this file while we have it open (resulting in an error
  // for stream functions).
  ne_filesystem_share_flags_delete = 4,
  ne_filesystem_share_flags_max = 5,
  ne_filesystem_share_flags_force_size = 0x7FFFFFFF
} ne_filesystem_share_flags;

// All files are always opened for binary (there is no text translation mode).
// If specifying 'ne_filesystem_create_if_none' or 'ne_filesystem_create_always'
// this call will implicitly create the directory structure leading up to the
// file.
//    NE_FILESYSTEM_RESULT_CREATE_ERROR:
//      The file could not be created. This can be for a number of reasons: the
//      file already exists and was locked, a directory of the same name exists,
//      a disk error occurred, permissions denied the creation, etc.
//    NE_FILESYSTEM_RESULT_FILE_DOES_NOT_EXIST:
//      When the file does not exist and 'ne_filesystem_create_never' was
//      specified.
//    NE_FILESYSTEM_RESULT_FILE_ERROR:
//      The file exists, but could not be opened for a number of reasons: the
//      file was locked by another operation, the file is marked read only and
//      we're doing a write operation, a disk error occurred, permissions denied
//      the creation, etc.
NE_CORE_API void (*ne_filesystem_open_file)(uint64_t *result,
                                            const char *path,
                                            ne_filesystem_io io,
                                            ne_filesystem_create create,
                                            ne_filesystem_share_flags share,
                                            ne_core_stream *stream_out);

typedef enum ne_filesystem_entry_type NE_CORE_ENUM
{
  ne_filesystem_entry_type_not_found = 0,
  ne_filesystem_entry_type_file = 1,
  ne_filesystem_entry_type_directory = 2,
  ne_filesystem_entry_type_symlink = 3,
  ne_filesystem_entry_type_block = 4,
  ne_filesystem_entry_type_character = 5,
  ne_filesystem_entry_type_fifo = 6,
  ne_filesystem_entry_type_socket = 7,
  ne_filesystem_entry_type_unknown = 8,
  ne_filesystem_entry_type_count = 9,

  ne_filesystem_entry_type_max = 10,
  ne_filesystem_entry_type_force_size = 0x7FFFFFFF
} ne_filesystem_entry_type;

NE_CORE_API ne_filesystem_entry_type (*ne_filesystem_get_type)(
    uint64_t *result, const char *path);

NE_CORE_API uint64_t (*ne_filesystem_get_last_write_time)(uint64_t *result,
                                                          const char *path);
NE_CORE_API void (*ne_filesystem_set_last_write_time)(uint64_t *result,
                                                      const char *path,
                                                      uint64_t time);

NE_CORE_API const char *(*ne_filesystem_read_symlink)(uint64_t *result,
                                                      const char *path);

// Returns the size of a file in bytes.
//    NE_FILESYSTEM_RESULT_FILE_DOES_NOT_EXIST:
//      The path specified does not point at a valid file.
NE_CORE_API uint64_t (*ne_filesystem_file_size)(uint64_t *result,
                                                const char *path);

// Removes a file or empty directory.
//    NE_FILESYSTEM_RESULT_DIRECTORY_NOT_EMPTY:
//      We attempted to delete a directory but it was not empty.
//      Use a recursive algorithm to clear out the contents of the directory and
//      all sub-directories first.
NE_CORE_API void (*ne_filesystem_delete)(uint64_t *result, const char *path);

// Moves a file from one path to another path. If the filename is different,
// this renames the file.
NE_CORE_API void (*ne_filesystem_move_rename)(uint64_t *result,
                                              const char *from_path,
                                              const char *to_path);

// If the given path ends in '/' this will return the first child of that
// directory. If the path does not end in '/' it will return the next sibling of
// that file or directory. If no more siblings exist, this will return null.

// List the child file nodes under the given directory.
// Enumerator dereference takes 'const char**'.
//    NE_FILESYSTEM_RESULT_DIRECTORY_DOES_NOT_EXIST:
//      The given path was not a path to a directory.
NE_CORE_API void (*ne_filesystem_enumerator)(
    uint64_t *result, const char *path, ne_core_enumerator *enumerator_out);

// This function should only be used for displaying a string to the user.
// This conversion is one to one with 'ne_filesystem_translate_os_to_ne'.
NE_CORE_API const char *(*ne_filesystem_translate_ne_to_os)(uint64_t *result,
                                                            const char *path);

// This function should only be used for parsing a file path provided by the
// user. This conversion is one to one with 'ne_filesystem_translate_ne_to_os'.
NE_CORE_API const char *(*ne_filesystem_translate_os_to_ne)(uint64_t *result,
                                                            const char *path);

// Get the current working directory (all relative paths are relative to this
// location). Unless otherwise specified by the sandbox, this directory will
// start at the executable directory.
NE_CORE_API const char *(*ne_filesystem_get_working_directory)(
    uint64_t *result);

// Set the current working directory (all relative paths are relative to this
// location).
NE_CORE_API const char *(*ne_filesystem_set_working_directory)(
    uint64_t *result, const char *path);

// Get the directory that the executable lives in.
NE_CORE_API const char *(*ne_filesystem_get_executable_directory)(
    uint64_t *result);

// Get a directory that your application will have permission to write to.
// This is generally the home or documents directory, specific to a user if
// possible.
NE_CORE_API const char *(*ne_filesystem_get_public_user_writable_directory)(
    uint64_t *result);

// Get a directory that your application will have permission to write to.
// This is generally a hidden or local directory, specific to a user if
// possible.
NE_CORE_API const char *(*ne_filesystem_get_private_user_writable_directory)(
    uint64_t *result);

// Get the temporary directory which your application may write temporary files
// to. These files may be periodically cleared.
NE_CORE_API const char *(*ne_filesystem_get_temporary_directory)(
    uint64_t *result);

typedef enum ne_filesystem_dialog_mode NE_CORE_ENUM
{
  ne_filesystem_dialog_mode_save = 0,
  ne_filesystem_dialog_mode_open = 1,
  ne_filesystem_dialog_mode_count = 2,

  ne_filesystem_dialog_mode_max = 3,
  ne_filesystem_dialog_mode_force_size = 0x7FFFFFFF
} ne_filesystem_dialog_mode;

typedef enum ne_filesystem_dialog_type NE_CORE_ENUM
{
  ne_filesystem_dialog_type_file = 0,
  ne_filesystem_dialog_type_directory = 1,
  ne_filesystem_dialog_type_count = 2,

  ne_filesystem_dialog_type_max = 3,
  ne_filesystem_dialog_type_force_size = 0x7FFFFFFF
} ne_filesystem_dialog_type;

typedef struct ne_filesystem_dialog_config ne_filesystem_dialog_config;
/// Configures an operating system specific dialog to allow users to select
/// files or directories.
struct ne_filesystem_dialog_config
{
  // Uses the '*' wildcard notation as well as separating '\n' for multiple
  // types.
  const char *filter;

  // .
  const char *starting_path;
  ne_core_bool multi_select;
  ne_filesystem_dialog_mode mode;
  ne_filesystem_dialog_type type;
};

// Show a file dialog for opening or saving files.
NE_CORE_API const char *(*ne_filesystem_dialog)(
    uint64_t *result, const ne_filesystem_dialog_config *config);

NE_CORE_END
