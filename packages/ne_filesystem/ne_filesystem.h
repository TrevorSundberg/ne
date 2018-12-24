/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

/// Major version of the library (bumped for breaking changes).
#define NE_FILESYSTEM_MAJOR 0
/// Minor version of the library (bumped for added features).
#define NE_FILESYSTEM_MINOR 0

/// By default the user does NOT have permission to read, write, or enumerate
/// the filesystem, except within directories returned by calling
/// #ne_filesystem_get_directory with the following:
/// - #ne_filesystem_directory_private.
/// - #ne_filesystem_directory_temporary.
#define NE_FILESYSTEM_PERMISSION 0x1d34f6658d725730

/// Determines if this package is fully supported on this platform.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
/// @return
///   #NE_CORE_TRUE if supported, #NE_CORE_FALSE otherwise.
NE_CORE_API ne_core_bool (*ne_filesystem_supported)(uint64_t *result);

/// Standard name for the Posix scheme which uses '/' and the root starts at
/// '/'.
#define NE_FILESYSTEM_SCHEME_POSIX "Posix" NE_CORE_NULL_PADDING

/// Standard name for the Windows scheme which uses '\\' and the root starts at
/// a drive (e.g. 'C:').
#define NE_FILESYSTEM_SCHEME_WINDOWS "Windows" NE_CORE_NULL_PADDING

/// Returns the name of the used filesystem path scheme. Each name is guaranteed
/// to be at least 8 bytes long, and the first 8 bytes are guaranteed to be
/// unqiue per scheme. This allows you to reinterpret the string as if it were a
/// uint64_t for efficient platform checks. It is recommended to avoid using
/// this function if possible to maximize platform independence. Do not free the
/// returned memory.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
/// @return
///   The name of the scheme that the filesystem uses, or nullptr on error.
///   - #ne_core_tag_platform_owned.
NE_CORE_API const char *(*ne_filesystem_get_scheme)(uint64_t *result);

/// All file system paths start with '/' and use '/' as a separator between all
/// directories (UNIX format). This way is chosen to keep code platform
/// agnostic. For example, on Windows the path 'C:\\Program Files' would be
/// '/C:/Program Files/'. Not starting with '/' indicates that the path is
/// relative to the working directory. Redundant '/////' will be treated as a
/// single '/'. Paths may optionally end with '/', however all paths returned
/// from ne_filesystem APIs will NOT include the trailing '/'. Not all
/// characters for filenames are valid on all platforms. The following is a list
/// of characters to avoid because they may not work on all platforms:
/// - <>:"/\|?*^
/// - Any ASCII character from 0 to 31 (decimal).
typedef struct ne_filesystem_tag_universal_path
    ne_filesystem_tag_universal_path;

/// A file system path that is in the specific operating system format. Note
/// that this format will not be accepted by any ne_filesystem API call. It
/// typically should only be used to display the path to the user of that
/// operating system.
typedef struct ne_filesystem_tag_os_path ne_filesystem_tag_os_path;

// We need to extend code to deal with symbolic links and permissions.

/// The file system entry (file, directory, etc.) could not be opened, created,
/// or accessed for a number of reasons: the entry was locked by another
/// operation, the entry is marked read only and we're doing a modifying/write
/// operation, a directory or file of the same name exists, a disk error
/// occurred, file or directory permissions denied the creation, etc.
#define NE_FILESYSTEM_RESULT_ERROR 0x3e9e5f2301e6affc

/// The option #ne_filesystem_if_file_exists_error was specified and a file of
/// the same path existed.
#define NE_FILESYSTEM_RESULT_FILE_EXISTS_ERROR 0xafa525b9a749e711

/// The option #ne_filesystem_if_none_exists_error was specified and a file did
/// not exist under the specified path.
#define NE_FILESYSTEM_RESULT_NONE_EXISTS_ERROR 0x313766bcddc425e3

/// A directory had children (files or sub-directories).
#define NE_FILESYSTEM_RESULT_DIRECTORY_NOT_EMPTY 0xef556522991e9089

/// Describes how we would like to interact with a file: input or output?
typedef enum ne_filesystem_io NE_CORE_ENUM
{
  /// Opens a file for reading. Supported stream operations:
  ///   - \ref ne_core_stream.read.
  ///   - \ref ne_core_stream.get_position.
  ///   - \ref ne_core_stream.get_size.
  ///   - \ref ne_core_stream.seek.
  ///   - \ref ne_core_stream.is_terminated.
  ///   - \ref ne_core_stream.free.
  ne_filesystem_io_read = 0,

  /// Opens a file for writing. Supported stream operations:
  ///   - \ref ne_core_stream.write.
  ///   - \ref ne_core_stream.flush.
  ///   - \ref ne_core_stream.get_position.
  ///   - \ref ne_core_stream.get_size.
  ///   - \ref ne_core_stream.seek.
  ///   - \ref ne_core_stream.is_terminated.
  ///   - \ref ne_core_stream.free.
  ne_filesystem_io_write = 1,

  /// Opens a file for both reading and writing. Supported stream operations:
  ///   - \ref ne_core_stream.read.
  ///   - \ref ne_core_stream.write.
  ///   - \ref ne_core_stream.get_position.
  ///   - \ref ne_core_stream.get_size.
  ///   - \ref ne_core_stream.seek.
  ///   - \ref ne_core_stream.is_terminated.
  ///   - \ref ne_core_stream.flush.
  ///   - \ref ne_core_stream.free.
  ne_filesystem_io_read_write = 2,

  /// Opens a file for writing in a special mode that always writes to the end
  /// of the stream (no seeking). Supported stream operations:
  ///   - \ref ne_core_stream.write.
  ///   - \ref ne_core_stream.flush.
  ///   - \ref ne_core_stream.get_position.
  ///   - \ref ne_core_stream.get_size.
  ///   - \ref ne_core_stream.is_terminated.
  ///   - \ref ne_core_stream.free.
  ne_filesystem_io_append = 3,

  /// Opens a file for writing in a special mode that always writes to the end
  /// of the stream. Seeking is supported, however writes will always occur at
  /// the end and reads will always occur at the seek position. Supported stream
  /// operations:
  ///   - \ref ne_core_stream.read.
  ///   - \ref ne_core_stream.write.
  ///   - \ref ne_core_stream.get_position.
  ///   - \ref ne_core_stream.get_size.
  ///   - \ref ne_core_stream.seek.
  ///   - \ref ne_core_stream.is_terminated.
  ///   - \ref ne_core_stream.flush.
  ///   - \ref ne_core_stream.free.
  ne_filesystem_io_read_append = 4,

  /// Enum entry count.
  ne_filesystem_io_max = 5,

  /// Force enums to be 32-bit.
  ne_filesystem_io_force_size = 0x7FFFFFFF
} ne_filesystem_io;

/// The action to take when opening and a file exists.
typedef enum ne_filesystem_if_file_exists NE_CORE_ENUM
{
  /// The existing file will be opened.
  ne_filesystem_if_file_exists_open = 0,

  /// The operation will result in #NE_FILESYSTEM_RESULT_FILE_EXISTS_ERROR.
  ne_filesystem_if_file_exists_error = 1,

  /// The existing file will be truncated to size 0 (an empty file).
  ne_filesystem_if_file_exists_truncate = 2,

  /// Enum entry count.
  ne_filesystem_if_file_exists_max = 3,

  /// Force enums to be 32-bit.
  ne_filesystem_if_file_exists_force_size = 0x7FFFFFFF
} ne_filesystem_if_file_exists;

/// The action to take when opening and a file does not exist.
typedef enum ne_filesystem_if_none_exists NE_CORE_ENUM
{
  /// None exists so a file will be created and opened in it's place.
  ne_filesystem_if_none_exists_create = 0,

  /// The operation will result in #NE_FILESYSTEM_RESULT_NONE_EXISTS_ERROR.
  ne_filesystem_if_none_exists_error = 1,

  /// Enum entry count.
  ne_filesystem_if_none_exists_max = 2,

  /// Force enums to be 32-bit.
  ne_filesystem_if_none_exists_force_size = 0x7FFFFFFF
} ne_filesystem_if_none_exists;

/// By default all restrictions are placed upon a file once it is opened. This
/// controls what restrictions we lift and allow others to access the same file.
typedef enum ne_filesystem_share_flags NE_CORE_ENUM
{
  /// Others may simultaneously open this file if they are reading.
  ne_filesystem_share_flags_read = 1,

  /// Others may simultaneously open this file if they are writing (includes
  /// appending).
  ne_filesystem_share_flags_write = 2,

  /// Others may delete this file while we have it open. This resultes in an
  /// error for stream functions and \ref ne_core_stream.is_terminated will be
  /// #NE_CORE_TRUE.
  ne_filesystem_share_flags_delete = 4,

  /// Flag max value.
  ne_filesystem_share_flags_max = 5,

  /// Force enums to be 32-bit.
  ne_filesystem_share_flags_force_size = 0x7FFFFFFF
} ne_filesystem_share_flags;

/// Forward declaration and alias.
typedef struct ne_filesystem_open_info ne_filesystem_open_info;
/// Encapsulates all options when opening a file with #ne_filesystem_open_file.
struct ne_filesystem_open_info
{
  /// The path to the file we would like to open in universal format.
  ///   - #ne_filesystem_tag_universal_path.
  const char *universal_path;

  /// Whether we're performing a reads, writes, or combination operations. This
  /// option controls the function pointers that will be filled out on the
  /// #ne_core_stream.
  ne_filesystem_io io;

  /// The action to take if a file already exists at the #universal_path.
  ne_filesystem_if_file_exists if_file_exists;

  /// The action to take if a file does not exist at the #universal_path.
  ne_filesystem_if_none_exists if_none_exists;

  /// When we open the file, how would we like to share it with other external
  /// processes or internal calls to
  ne_filesystem_share_flags share_flags;

  /// If we want this file to always read directly from the disk or source
  /// without going through any operating system caching mechanisms. This is
  /// useful for when the underlying file is being modified and the most up to
  /// date version is requested. This parameter has serious performance
  /// ramifications.
  ne_core_bool bypass_cache;
};

/// All files are always opened for binary (there is no text translation mode).
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p info.path required #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_FILE_EXISTS_ERROR:
///     If a file already existed and #ne_filesystem_if_file_exists_error was
///     specified.
///   - #NE_FILESYSTEM_RESULT_NONE_EXISTS_ERROR:
///     If a file did not exist and #ne_filesystem_if_none_exists_error was
///     specified.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     The file could not be opened or created.
/// @param info
///   A struct that describes the path to open, if we want to
///   read/write/append, what we want to do if the file exists or doesn't exist,
///   and any other special settings.
/// @param stream_out
///   Outputs the created stream. The operations available on the stream are
///   dependent upon #ne_filesystem_open_info.io given in parameter \p info.
NE_CORE_API void (*ne_filesystem_open_file)(uint64_t *result,
                                            const ne_filesystem_open_info *info,
                                            ne_core_stream *stream_out);

/// Indicates the type of an entry in the file system such as a file,
/// directory, or special contstruct.
typedef enum ne_filesystem_entry_type NE_CORE_ENUM
{
  /// Returned in the case of an error.
  ne_filesystem_entry_type_none = 0,
  /// The path did not resolve to an existing entry.
  ne_filesystem_entry_type_not_found = 1,
  /// The entry is a directory which may contain children.
  ne_filesystem_entry_type_directory = 2,
  /// The entry is a regular file in the file system.
  ne_filesystem_entry_type_regular = 3,
  /// The entry is a symbolic link to another file or directory.
  ne_filesystem_entry_type_symbolic_link = 4,
  /// The entry is a block device that allows random access.
  ne_filesystem_entry_type_block = 5,
  /// The entry is a character device that supports a serial stream.
  ne_filesystem_entry_type_character = 6,
  /// The entry is a named pipe.
  ne_filesystem_entry_type_pipe = 7,
  /// The entry is a socket.
  ne_filesystem_entry_type_socket = 8,
  /// The entry exists but is not of a known type.
  ne_filesystem_entry_type_unknown = 9,

  /// Enum entry count.
  ne_filesystem_entry_type_max = 10,
  /// Force enums to be 32-bit.
  ne_filesystem_entry_type_force_size = 0x7FFFFFFF
} ne_filesystem_entry_type;

/// Determine the type of an entry in the file system.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p path required #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     An error occurred or the path did not resolve to a file system entry.
/// @param universal_path
///   The path to the entry in the universal format.
///   - #ne_filesystem_tag_universal_path.
/// @return
///   The type of entry, or #ne_filesystem_entry_type_none if an error occurs.
NE_CORE_API ne_filesystem_entry_type (*ne_filesystem_get_type)(
    uint64_t *result, const char *universal_path);

/// An entry in a filesystem has 3 types of time assocaited with it: time of
/// creation, last time the entry was modified, and the last time the entry was
/// accessed.
typedef enum ne_filesystem_time_type NE_CORE_ENUM
{
  /// Represents the time that the entry was created.
  ne_filesystem_time_type_created = 0,
  /// Represents the last time that the entry was modified or changed.
  ne_filesystem_time_type_modified = 1,
  /// Represents the last time that the entry was accessed.
  ne_filesystem_time_type_accessed = 2,

  /// Enum entry count.
  ne_filesystem_time_type_max = 3,
  /// Force enums to be 32-bit.
  ne_filesystem_time_type_force_size = 0x7FFFFFFF
} ne_filesystem_time_type;

/// Retrieve a time in nanoseconds for a file entry. Note that the actual
/// precision of the time is operating system dependent.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p path required #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     An error occurred or the path did not resolve to a file system entry.
/// @param universal_path
///   The path to the entry in the universal format.
///   - #ne_filesystem_tag_universal_path.
/// @param type
///   The type of time (created, modified, or accessed).
/// @return
///   The time of the file in nanoseconds, or 0 if an error occurs.
NE_CORE_API uint64_t (*ne_filesystem_get_time)(uint64_t *result,
                                               const char *universal_path,
                                               ne_filesystem_time_type type);

/// Set a time in nanoseconds for a file entry. Note that the actual
/// precision of the time is operating system dependent. This means that if you
/// were to call #ne_filesystem_get_time with the same path immediately after,
/// it may not return the same as \p time.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p path required #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     An error occurred or the path did not resolve to a file system entry.
/// @param path
///   The path to the entry in the universal format.
///   - #ne_filesystem_tag_universal_path.
/// @param type
///   The type of time (created, modified, or accessed).
/// @param time
///   The time of the file in nanoseconds.
NE_CORE_API void (*ne_filesystem_set_time)(uint64_t *result,
                                           const char *path,
                                           ne_filesystem_time_type type,
                                           uint64_t time_nanoseconds);

/// Reads the destination of a symbolic link. The memory returned must be freed.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p path required #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     An error occurred or the path did not resolve to a file system entry.
///   - #NE_CORE_RESULT_ALLOCATION_FAILED:
///     Not enough system memory or address space, or other system error.
/// @param universal_path
///   The path to the entry in the universal format.
///   - #ne_filesystem_tag_universal_path.
/// @return
///   The destination path of the symbolic link in universal format, or
///   #NE_CORE_NULL if an error occurs.
///   - #ne_core_tag_user_owned.
///   - #ne_filesystem_tag_universal_path.
NE_CORE_API const char *(*ne_filesystem_read_symlink)(
    uint64_t *result, const char *universal_path);

/// Returns the size of a file in bytes.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p path required #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     An error occurred or the path did not resolve to a file system entry.
/// @param path
///   The path to the entry in the universal format.
///   - #ne_filesystem_tag_universal_path.
/// @return
///   The number of bytes within the file, or 0 if an error occurs.
NE_CORE_API uint64_t (*ne_filesystem_file_size)(uint64_t *result,
                                                const char *path);

/// Removes a file or empty directory.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p path required #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_DIRECTORY_NOT_EMPTY:
///     We attempted to delete a directory but it was not empty.
///     Use a recursive algorithm to clear out the contents of the directory and
///     all sub-directories first.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     An error occurred or the path did not resolve to a file system entry.
/// @param universal_path
///   The path to the entry in the universal format.
///   - #ne_filesystem_tag_universal_path.
NE_CORE_API void (*ne_filesystem_delete)(uint64_t *result,
                                         const char *universal_path);

/// Moves a file from one path to another path. This operating can also be
/// conceptually thought of as rennaming if only the filename part of the path
/// changes.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p from_universal_path or \p to_universal_path required
///     #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     An error occurred or the path did not resolve to a file system entry.
/// @param from_universal_path
///   The source universal path to the entry we want to move/rename.
///   - #ne_filesystem_tag_universal_path.
/// @param to_universal_path
///   The destination universal path to where we want the entry after the
///   move/rename completes.
///   - #ne_filesystem_tag_universal_path.
NE_CORE_API void (*ne_filesystem_move_rename)(uint64_t *result,
                                              const char *from_universal_path,
                                              const char *to_universal_path);

/// Outputs an enumerator that walks over the child entries of the given
/// directory and outputs the name of each child entry. Note that dereferncing
/// the enumerator does NOT output paths.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_PERMISSION_DENIED:
///     The \p directory_path required #NE_FILESYSTEM_PERMISSION.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     An error occurred or the path did not resolve to a directory.
/// @param directory_universal_path
///   The path to the directory in the universal format.
///   - #ne_filesystem_tag_universal_path.
/// @param enumerator_out
///   Outputs the created enumerator.
///   #ne_core_enumerator.dereference takes 'const char **' for 'value_out' and
///   outputs the name of each child (NOT paths).
NE_CORE_API void (*ne_filesystem_enumerator)(
    uint64_t *result,
    const char *directory_universal_path,
    ne_core_enumerator *enumerator_out);

/// Converts an operating system specific path to an absolute universal path.
/// This function should typically only be used for converting a user written
/// string or for interoping directly with the operating system. Note that
/// unsupported symbols are not changed in any way. The path is made absolute by
/// utilizing the current working drive/directory. The reason that the path is
/// always converted to absolute is that there are os path constructs that
/// cannot be represented with the universal path (such as 'C:test.txt' on
/// Windows). Conversion to absolute allows us to avoid these unsupported
/// constructs. All strings and paths are in UTF8, regardless of operating
/// system. The memory returned must be freed.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     The path was unable to be translated.
///   - #NE_CORE_RESULT_ALLOCATION_FAILED:
///     Not enough system memory or address space, or other system error.
/// @param universal_path
///   The path to the entry in the universal format.
///   - #ne_filesystem_tag_universal_path.
/// @return
///   The translated path in os format, or #NE_CORE_NULL if an error occurs.
///   - #ne_core_tag_user_owned.
///   - #ne_filesystem_tag_os_path.
NE_CORE_API char *(*ne_filesystem_translate_universal_to_os)(
    uint64_t *result, const char *universal_path);

/// Converts an operating system specific path to an absolute canonicalized
/// universal path. This function should typically only be used for converting a
/// user written string or for interoping directly with the operating system.
/// Note that unsupported symbols are not changed in any way. The path is made
/// absolute by utilizing the current working drive/directory. The reason that
/// the path is always converted to absolute is that there are os path
/// constructs that cannot be represented with the universal path (such as
/// 'C:test.txt' on Windows). Conversion to absolute allows us to avoid these
/// unsupported constructs. All strings and paths are in UTF8, regardless of
/// operating system. The memory returned must be freed.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_FILESYSTEM_RESULT_ERROR:
///     The path was unable to be translated.
///   - #NE_CORE_RESULT_ALLOCATION_FAILED:
///     Not enough system memory or address space, or other system error.
/// @param os_path
///   The path to the entry in the operating system specific format.
///   - #ne_filesystem_tag_os_path.
/// @return
///   The translated path in universal format, or #NE_CORE_NULL if an error
///   occurs.
///   - #ne_core_tag_user_owned.
///   - #ne_filesystem_tag_universal_path.
NE_CORE_API char *(*ne_filesystem_translate_os_to_universal)(
    uint64_t *result, const char *os_path);

/// Set the current working directory. All relative paths are relative to this
/// location.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
/// @param universal_path
///   The path to the entry in the universal format.
///   - #ne_filesystem_tag_universal_path.
NE_CORE_API void (*ne_filesystem_set_working_directory)(
    uint64_t *result, const char *universal_path);

/// Special directories we may request.
typedef enum ne_filesystem_directory NE_CORE_ENUM
{
  /// A directory to read/write files that are not visible to the public or
  /// other applications. This directory is specific to machine, user, and
  /// application. #NE_FILESYSTEM_PERMISSION is NOT required to mutate files
  /// within this directory.
  ne_filesystem_directory_private = 0,

  /// A directory to read/write temporary or cached files to. These files may be
  /// periodically cleared by users or the operating system. It is recommended
  /// that your application clears its own files when they are not needed.
  /// #NE_FILESYSTEM_PERMISSION is NOT required to mutate files within this
  /// directory.
  ne_filesystem_directory_temporary = 1,

  /// A public directory that is visible by all applications and to the user.
  ne_filesystem_directory_public = 2,

  /// All relative paths are relative to this location. This directory typically
  /// is set to the executable directory, however users and other applications
  /// may set the working directory when launching your application.
  ne_filesystem_directory_working = 3,

  /// Get the directory that the executable lives in. This directory should not
  /// be written to any may be read only on many platforms.
  ne_filesystem_directory_executable = 4,

  /// Enum entry count.
  ne_filesystem_directory_max = 3,

  /// Force enums to be 32-bit.
  ne_filesystem_directory_force_size = 0x7FFFFFFF
} ne_filesystem_directory;

/// Retrieve a special directory for the current platform.
/// @param result
///   - #NE_CORE_RESULT_SUCCESS:
///     The operation completed successfully.
///   - #NE_CORE_RESULT_NOT_SUPPORTED:
///     The package is not supported.
///   - #NE_CORE_RESULT_ALLOCATION_FAILED:
///     Not enough system memory or address space, or other system error.
/// @param directory
///   Which special directory is being requested.
/// @return
///   A path to the directory requested in universal format or #NE_CORE_NULL if
///   an error occurs.
///   - #ne_core_tag_user_owned.
///   - #ne_filesystem_tag_universal_path.
NE_CORE_API const char *(*ne_filesystem_get_directory)(
    uint64_t *result, ne_filesystem_directory directory);
