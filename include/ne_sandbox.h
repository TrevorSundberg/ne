// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_sandbox_supported(ne_result* result);

// Comunicate a custom message with the sandbox.
// This function is entirely sandbox defined and has no explicit behavior.
// The return value is also sandbox defined.
uint64_t NE_API ne_sandbox_communicate(ne_result* result, void* buffer, uint64_t size);

// Signal a message to the sandbox application.
void NE_API ne_sandbox_message(ne_result* result, const char* message);

// Signal an error to the sandbox application.
void NE_API ne_sandbox_error(ne_result* result, const char* message);

// Terminates the application at the current location. The code and the message are optional.
void NE_API ne_sandbox_terminate(ne_result* result, uint64_t code, const char* message);

// This is the first function that should be called to test if functionality is working.
// The sandbox will display a Hello World message in a console or as a message box.
// This is also used as an introductory function to demonstrate that an application is working.
void NE_API ne_sandbox_hello_world(ne_result* result);

NE_END
