// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_sandbox, 0, 0);

// Get the major version of the API. Between major versions breaking changes are
// applied.
NE_API uint64_t (*ne_sandbox_api_major)(uint64_t *result);

// Get the minor version of the API. Between minor versions features are added.
NE_API uint64_t (*ne_sandbox_api_minor)(uint64_t *result);

// Note that the 'api' does not have a patch version, because it is a
// header/interface not an implementation.

// The name of the current sandbox application.
// This is akin to a browser, such as "Chrome", "Firefox", "Opera", "Edge",
// "IE", "Safari", etc. This can be used to detect specific sandboxes when using
// sandbox targeted functions such as 'ne_package_get_sandbox' or
// 'ne_sandbox_communicate'.
NE_API const char *(*ne_sandbox_name)(uint64_t *result);

// Get the major version of the Sandbox. Between major versions breaking changes
// are applied.
NE_API uint64_t (*ne_sandbox_major)(uint64_t *result);

// Get the minor version of the Sandbox. Between minor versions
// features/interfaces are added.
NE_API uint64_t (*ne_sandbox_minor)(uint64_t *result);

// Get the patch version of the Sandbox. Between patch versions bugs are fixed
// (no interface changes).
NE_API uint64_t (*ne_sandbox_patch)(uint64_t *result);

// The name of the current platform.
// "Windows", "Linux", "Android", "iOS", "Mac OS X", etc.
NE_API const char *(*ne_sandbox_platform)(uint64_t *result);

// Comunicate a custom message with the sandbox.
// This function is entirely sandbox defined and has no explicit behavior.
// The return value is also sandbox defined.
NE_API uint64_t (*ne_sandbox_communicate)(uint64_t *result, void *buffer,
                                          uint64_t size);

// Signal a message to the sandbox application.
NE_API void (*ne_sandbox_message)(uint64_t *result, const char *message);

// Terminates the application at the current location. The code and the message
// are optional.
NE_API void (*ne_sandbox_terminate)(uint64_t *result, uint64_t code,
                                    const char *message);

NE_END
