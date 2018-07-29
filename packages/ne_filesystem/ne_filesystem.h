// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_filesystem, 0, 0);

// All file system paths start with '/' and use '/' as a separator between all directories.
// This way is chosen to keep code platform agnostic. To translate between the current operating system use
// the translate functions.

// We need to extend code to deal with symbolic links and permissions.

typedef uint64_t ne_filesystem_mode;
static const ne_filesystem_mode ne_filesystem_mode_read = 0;
static const ne_filesystem_mode ne_filesystem_mode_write = 1;
static const ne_filesystem_mode ne_filesystem_mode_append = 2;
static const ne_filesystem_mode ne_filesystem_mode_count = 3;

NE_API const char* (*ne_filesystem_translate_os_to_ne)(const char* path);
NE_API const char* (*ne_filesystem_translate_ne_to_os)(const char* path);

NE_API void (*ne_filesystem_open_file)(uint64_t* result, ne_filesystem_mode mode, const char* filename, ne_core_stream* stream_out);

NE_API const char* (*ne_filesystem_get_path_separator)(uint64_t* result);

// Get the current working directory (all relative paths are relative to this location).
NE_API const char* (*ne_filesystem_get_working_directory)(uint64_t* result);

// Set the current working directory (all relative paths are relative to this location).
NE_API const char* (*ne_filesystem_set_working_directory)(uint64_t* result, const char* directory);

// Get the directory that the executable lives in.
NE_API const char* (*ne_filesystem_get_executable_directory)(uint64_t* result);

// Get a directory that your application will have permission to write to.
NE_API const char* (*ne_filesystem_get_documents_directory)(uint64_t* result);

// Get the temporary directory which your application may write temporary files to.
// These files may be periodically cleared.
NE_API const char* (*ne_filesystem_get_temporary_directory)(uint64_t* result);

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
  ne_core_bool multi_select;
  ne_filesystem_dialog_mode mode;
  ne_filesystem_dialog_type type;
};
typedef struct ne_filesystem_dialog_config ne_filesystem_dialog_config;

// Show a file dialog for opening or saving files.
NE_API const char* (*ne_filesystem_dialog)(uint64_t* result, const ne_filesystem_dialog_config* config);

NE_END
