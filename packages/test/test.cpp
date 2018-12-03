// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test/test.h"

int32_t ne_core_main(int32_t argc, char *argv[])
{
  ne_core_bool simulated_environment =
      argc == 2 &&
      test_core_string_compare(argv[1], "--simulated_environment") == 0;

  ne_core_bool result = NE_CORE_TRUE;
  result &= test_core(simulated_environment);
  result &= test_io(simulated_environment);

  // We want to return 0 for success, and 1 for error.
  return (int32_t)!result;
}
