// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
#include "ne_file.h"
NE_BEGIN

ne_bool NE_API ne_package_supported(ne_result* result);

struct ne_package_info
{
  const char* name;
  uint64_t major;
  uint64_t minor;
};

typedef struct ne_package_info ne_package_info;

typedef struct ne_package ne_package;


ne_package* NE_API ne_package_enumerate_next(ne_result* result, ne_package* current);

// Requesting a package will result in an event that lets you know when a package is loaded.
void NE_API ne_package_request(ne_result* result, ne_package_info* info);

// Checks if a package exists locally (if not, then the package must be requested).
ne_bool NE_API ne_package_exists(ne_result* result, ne_package_info* info);

// Loads a package and blocks upon the request if the package does not exist locally.
// If the package fails to load or a request fails, null will be returned.
ne_package* NE_API ne_package_load(ne_result* result, ne_package_info* info);

ne_package* NE_API ne_package_load_memory(ne_result* result, ne_package_info* info, const uint8_t* memory, uint64_t size);

// Get the package that was first loaded where we started execution.
ne_package* NE_API ne_package_get_executable(ne_result* result);

// Get the sandbox package, a special package provided explicitly by the sandbox application.
// Files and functions defined within this package are entirely up to the sandbox and have no standardization.
// This is how extensions are created for applications.
// Sandboxes may even allow third parties to extend the sandbox with their own custom way of loading addons.
ne_package* NE_API ne_package_get_sandbox(ne_result* result);

// All files are relative to the package root.
ne_file* NE_API ne_package_open_file(ne_result* result, ne_package* package, const char* filename);

void* NE_API ne_package_find_symbol(ne_result* result, ne_package* package, const char* name);

void NE_API ne_package_close(ne_result* result, ne_package* package);

NE_END
