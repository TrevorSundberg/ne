// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test/test.h"
NE_CORE_BEGIN

NE_CORE_DEFINE_PACKAGE(test);

int32_t ne_core_main(int32_t argc, char *argv[]) {
  ne_core_bool result = NE_CORE_TRUE;
  result &= test_core();
  result &= test_io();

  // We want to return 0 for success, and 1 for error.
  return (int32_t)!result;
}

NE_CORE_END
