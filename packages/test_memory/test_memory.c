// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_memory/test_memory.h"
NE_BEGIN

NE_DEFINE_PACKAGE(test_memory);

static void ne_memory_full_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();

  TEST_CLEAR_RESULT();
  void* allocation = ne_memory_allocate(result, 1);
  TEST_EXPECT_RESULT("ne_memory_allocate", expected_result);
  TEST_EXPECT("ne_memory_allocate", allocation != ne_core_null);

  TEST_CLEAR_RESULT();
  ne_memory_free(result, allocation);
  TEST_EXPECT_RESULT("ne_memory_free", expected_result);

  // Allocate something so large that we expect it to fail.
  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_memory_allocate", ne_memory_allocate(result, 0xFFFFFFFFFFFFFFFF) == ne_core_null);
  TEST_EXPECT_RESULT("ne_memory_allocate", ne_memory_result_allocation_failed);
}

static void ne_memory_null_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();

  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_memory_allocate", ne_memory_allocate(result, 1) == ne_core_null);
  TEST_EXPECT_RESULT("ne_memory_allocate", expected_result);

  TEST_CLEAR_RESULT();
  ne_memory_free(result, ne_core_null);
  TEST_EXPECT_RESULT("ne_memory_free", expected_result);
}

static void ne_memory_shared_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();
}

ne_core_bool test_memory()
{
  TEST_RUN(ne_memory);
}

NE_END
