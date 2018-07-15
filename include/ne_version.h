// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_version_supported(ne_result* result);

static const uint64_t ne_version_header_major = 1;
static const uint64_t ne_version_header_minor = 0;

// Get the major version of the API. Between major versions breaking changes are applied.
uint64_t NE_API ne_version_api_major(ne_result* result);

// Get the minor version of the API. Between minor versions features are added.
uint64_t NE_API ne_version_api_minor(ne_result* result);

// Note that the 'api' does not have a patch version, because it is a header/interface not an implementation.

// The name of the current sandbox application.
// This is akin to a browser, such as "Chrome", "Firefox", "Opera", "Edge", "IE", "Safari", etc.
// This can be used to detect specific sandboxes when using sandbox targeted functions such as
// 'ne_package_get_sandbox' or 'ne_sandbox_communicate'.
const char* NE_API ne_version_sandbox(ne_result* result);

// Get the major version of the Sandbox. Between major versions breaking changes are applied.
uint64_t NE_API ne_version_sandbox_major(ne_result* result);

// Get the minor version of the Sandbox. Between minor versions features/interfaces are added.
uint64_t NE_API ne_version_sandbox_minor(ne_result* result);

// Get the patch version of the Sandbox. Between patch versions bugs are fixed (no interface changes).
uint64_t NE_API ne_version_sandbox_patch(ne_result* result);

// The name of the current platform.
// "Windows", "Linux", "Android", "iOS", "Mac OS X", etc.
const char* NE_API ne_version_platform(ne_result* result);

NE_END
