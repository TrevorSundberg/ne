// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
//#include "../test_clipboard/test_clipboard.h"
#include "../test_core/test_core.h"
#include "../test_io/test_io.h"
//#include "../test_cpu/test_cpu.h"
//#include "../test_debug/test_debug.h"
//#include "../test_display/test_display.h"
//#include "../test_filesystem/test_filesystem.h"
//#include "../test_keyboard/test_keyboard.h"
//#include "../test_mouse/test_mouse.h"
//#include "../test_package/test_package.h"
//#include "../test_power/test_power.h"
//#include "../test_sandbox/test_sandbox.h"
//#include "../test_socket/test_socket.h"
//#include "../test_thread/test_thread.h"
//#include "../test_timer/test_timer.h"
//#include "../test_touch/test_touch.h"
//#include "../test_vulkan/test_vulkan.h"

// These are APIs that are planned:
//#include "../test_atomic/test_atomic.h"
//#include "../test_audio/test_audio.h"
//#include "../test_gamepad/test_gamepad.h"
//#include "../test_haptic/test_haptic.h"
//#include "../test_intrinsic/test_intrinsic.h"
//#include "../test_joystick/test_joystick.h"
//#include "../test_port/test_port.h"
//#include "../test_print/test_print.h"
//#include "../test_window/test_window.h"
NE_BEGIN

NE_DECLARE_PACKAGE(test, 0, 0);

// Runs all the tests for all standard ne-libraries.
int32_t ne_core_main(int32_t argc, char *argv[]);

NE_END
