// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
#include "ne_file.h"
NE_BEGIN

ne_bool NE_API ne_filesystem_supported(ne_result* result);

ne_bool NE_API ne_filesystem_request_permission(ne_result* result);

typedef uint64_t ne_filesystem_mode;
static const ne_filesystem_mode ne_filesystem_mode_read = 0;
static const ne_filesystem_mode ne_filesystem_mode_write = 1;
static const ne_filesystem_mode ne_filesystem_mode_append = 2;
static const ne_filesystem_mode ne_filesystem_mode_count = 3;

ne_file* NE_API ne_filesystem_open_file(ne_result* result, ne_filesystem_mode mode, const char* filename);

const char* NE_API ne_filesystem_get_path_separator(ne_result* result);

// Get the current working directory (all relative paths are relative to this location).
const char* NE_API ne_filesystem_get_working_directory(ne_result* result);

// Set the current working directory (all relative paths are relative to this location).
const char* NE_API ne_filesystem_set_working_directory(ne_result* result, const char* directory);

// Get the directory that the executable lives in.
const char* NE_API ne_filesystem_get_executable_directory(ne_result* result);

// Get a directory that your application will have permission to write to.
const char* NE_API ne_filesystem_get_documents_directory(ne_result* result);

// Get the temporary directory which your application may write temporary files to.
// These files may be periodically cleared.
const char* NE_API ne_filesystem_get_temporary_directory(ne_result* result);

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
const char* NE_API ne_filesystem_dialog(ne_result* result, const ne_filesystem_dialog_config* config);

NE_END
