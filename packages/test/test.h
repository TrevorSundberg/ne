/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
#include "../ne_core/ne_intrinsic.h"

#define TEST_SIMULATED_STREAM "<This is a test>"
#define TEST_SIMULATED_SIZE (sizeof(TEST_SIMULATED_STREAM) - 1)

typedef struct test_table test_table;
/// This table contains all the test functions and is automatically
/// built using #TEST_RUN.
struct test_table
{
  ne_core_bool (*supported)(uint64_t *result);
  uint64_t permission;
  ne_core_bool simulated_environment;
  ne_core_bool is_final_run;

  // This is called when we know the API is both supported and we have
  // permission. Called twice with valid pointer for result and with null.
  void (*full_tests)(test_table *table);

  // These are tests we run if the API is not supported or permission is denied.
  // All calls are expected to return zero/null, and output the expected result.
  // Called twice with valid pointer for result and with null.
  void (*null_tests)(test_table *table);

  // These tests don't rely on the return value and therefore can be called
  // during both full/null phases.
  // Called twice with valid pointer for result and with null.
  void (*shared_tests)(test_table *table);

  // Tests that run at the end of all tests completing. This typically involves
  // verifying that all asynchronous callbacks were invoked.
  // Only called once at exit if ne_core is supported.
  void (*exit_tests)(test_table *table);

  // The result we expect out of the tests that are running.
  uint64_t expected_result;

  // A pointer to the result that we give to each call.
  uint64_t *result;

  // Indicates whether all operations were a success (default NE_CORE_TRUE).
  ne_core_bool success;

  // Any data we want to pass to the tests.
  void *user_data;
};

ne_core_bool test_validate(ne_core_bool value,
                           uint64_t error_result,
                           const char *file,
                           int64_t line,
                           const char *message);

uint64_t test_string_length(const char *string);

int64_t test_string_compare(const char *a, const char *b);

int64_t test_memory_compare_value(void *memory, uint8_t value, uint64_t size);

// Initializes a buffer with psuedo-random memory.
void test_random_initialize(void *memory, uint64_t size);

int64_t test_random_compare(void *memory, uint64_t size);

// This will attempt to test all non-null functions inside the stream interface.
// When 'simulated_environment' is false:
//  - Non-blocking operations only.
//  - No specific data is expected to be read.
// When 'simulated_environment' is true:
//  - Non-blocking and blocking operations are performed.
//  - A specific string TEST_SIMULATED_STREAM is expected to be read.
void test_stream(ne_core_stream *stream,
                 ne_core_bool free_stream,
                 test_table *table);

void test_run(test_table *table);

#define TEST_RUN(library_supported, library_permission)                        \
  static test_table table;                                                     \
  table.user_data = NE_CORE_NULL;                                              \
  table.full_tests = &full_tests;                                              \
  table.null_tests = &null_tests;                                              \
  table.shared_tests = &shared_tests;                                          \
  table.exit_tests = &exit_tests;                                              \
  table.supported = library_supported;                                         \
  table.permission = library_permission;                                       \
  table.simulated_environment = simulated_environment;                         \
  table.expected_result = NE_CORE_RESULT_INVALID;                              \
  table.is_final_run = NE_CORE_FALSE;                                          \
  table.result = NE_CORE_NULL;                                                 \
  table.success = NE_CORE_TRUE;                                                \
  test_run(&table)

#define TEST_EXPECT(expression)                                                \
  NE_CORE_ENCLOSURE(table->success &= test_validate(                           \
                        (expression), NE_CORE_RESULT_INVALID, __FILE__,        \
                        __LINE__, "TEST_EXPECT(" #expression ") failed\n");)

#define TEST_CLEAR_RESULT()                                                    \
  NE_CORE_ENCLOSURE(if (table->result) *table->result =                        \
                        NE_CORE_RESULT_INVALID;);

#define TEST_EXPECT_RESULT(expected_result)                                    \
  NE_CORE_ENCLOSURE(if (table->result) {                                       \
    table->success &= test_validate(                                           \
        *table->result == (expected_result), *table->result, __FILE__,         \
        __LINE__, "TEST_EXPECT_RESULT(" #expected_result ") failed\n");        \
  })

#define TEST_EXPECT_TABLE_RESULT() TEST_EXPECT_RESULT(table->expected_result)

#define TEST_DECLARE_PARAMETERS                                                \
  ne_core_bool *is_success_out, uint64_t *result, uint64_t expected_result,    \
      void *user_data

#define TEST_PASS_PARAMETERS is_success_out, result, expected_result, user_data
