// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_CORE_BEGIN

NE_CORE_DECLARE_PACKAGE(ne_package, 0, 0);

typedef struct ne_package_info ne_package_info;
/// Information needed to identify a package.
struct ne_package_info
{
  const char *name;
  uint64_t major;
  uint64_t minor;
};

typedef struct ne_package ne_package;

// List all the packages that are currently loaded.
// Enumerator dereference takes 'ne_package**'.
NE_CORE_API void (*ne_package_symbol_enumerator)(
    uint64_t *result, ne_core_enumerator *enumerator_out);

// Requesting a package will result in an event that lets you know when a
// package is loaded.
NE_CORE_API void (*ne_package_request)(uint64_t *result, ne_package_info *info);

// Checks if a package exists locally (if not, then the package must be
// requested).
NE_CORE_API ne_core_bool (*ne_package_exists)(uint64_t *result,
                                              ne_package_info *info);

// Loads a package and blocks upon the request if the package does not exist
// locally. If the package fails to load or a request fails, null will be
// returned.
NE_CORE_API ne_package *(*ne_package_load)(uint64_t *result,
                                           ne_package_info *info);

NE_CORE_API ne_package *(*ne_package_load_from_stream)(uint64_t *result,
                                                       ne_package_info *info,
                                                       ne_core_stream *stream);

// Get the package that was first loaded where we started execution.
NE_CORE_API ne_package *(*ne_package_get_executable)(uint64_t *result);

// Get the sandbox package, a special package provided explicitly by the sandbox
// application. Files and functions defined within this package are entirely up
// to the sandbox and have no standardization. This is how extensions are
// created for applications. Sandboxes may even allow third parties to extend
// the sandbox with their own custom way of loading addons.
NE_CORE_API ne_package *(*ne_package_get_sandbox)(uint64_t *result);

// All files are relative to the package root.
NE_CORE_API void (*ne_package_open_file)(uint64_t *result,
                                         ne_package *package,
                                         const char *filename,
                                         ne_core_stream *stream_out);

// Note that if this is an 'ne' function, it will be a pointer to the function
// pointer. Returns null if the symbol is not found.
NE_CORE_API void *(*ne_package_symbol_find)(uint64_t *result,
                                            ne_package *package,
                                            const char *name);

typedef struct ne_package_symbol ne_package_symbol;
/// An exported symbol and its address.
struct ne_package_symbol
{
  const char *name;
  void *symbol;
};

// List all the exported symbols within a given package.
// Enumerator dereference takes 'ne_package_symbol*'.
NE_CORE_API void (*ne_package_symbol_enumerator)(
    uint64_t *result, ne_core_enumerator *enumerator_out);

NE_CORE_API void (*ne_package_close)(uint64_t *result, ne_package *package);

NE_CORE_END
