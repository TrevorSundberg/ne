// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_package, 0, 0);

struct ne_package_info
{
  const char* name;
  uint64_t major;
  uint64_t minor;
};

typedef struct ne_package_info ne_package_info;

typedef struct ne_package ne_package;

NE_API ne_package* (*ne_package_enumerate_next)(uint64_t* result, ne_package* current);

// Requesting a package will result in an event that lets you know when a package is loaded.
NE_API void (*ne_package_request)(uint64_t* result, ne_package_info* info);

// Checks if a package exists locally (if not, then the package must be requested).
NE_API ne_core_bool (*ne_package_exists)(uint64_t* result, ne_package_info* info);

// Loads a package and blocks upon the request if the package does not exist locally.
// If the package fails to load or a request fails, null will be returned.
NE_API ne_package* (*ne_package_load)(uint64_t* result, ne_package_info* info);

NE_API ne_package* (*ne_package_load_memory)(uint64_t* result, ne_package_info* info, const uint8_t* memory, uint64_t size);

// Get the package that was first loaded where we started execution.
NE_API ne_package* (*ne_package_get_executable)(uint64_t* result);

// Get the sandbox package, a special package provided explicitly by the sandbox application.
// Files and functions defined within this package are entirely up to the sandbox and have no standardization.
// This is how extensions are created for applications.
// Sandboxes may even allow third parties to extend the sandbox with their own custom way of loading addons.
NE_API ne_package* (*ne_package_get_sandbox)(uint64_t* result);

// All files are relative to the package root.
NE_API void (*ne_package_open_file)(uint64_t* result, ne_package* package, const char* filename, ne_core_stream* stream_out);

NE_API void* (*ne_package_find_symbol)(uint64_t* result, ne_package* package, const char* name);

NE_API void (*ne_package_close)(uint64_t* result, ne_package* package);

NE_END
