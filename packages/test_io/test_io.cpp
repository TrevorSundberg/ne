/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_io/test_io.h"

static void full_tests(test_table *table)
{
  ne_core_stream input;
  ne_intrinsic_memory_set(&input, 1, sizeof(input));
  TEST_CLEAR_RESULT();
  ne_io_get_input(table->result, &input);
  TEST_EXPECT_RESULT(table->expected_result);

  TEST_EXPECT(input.read != NE_CORE_NULL);
  TEST_EXPECT(input.write == NE_CORE_NULL);
  TEST_EXPECT(input.flush == NE_CORE_NULL);
  TEST_EXPECT(input.get_position == NE_CORE_NULL);
  TEST_EXPECT(input.get_size == NE_CORE_NULL);
  TEST_EXPECT(input.is_terminated == NE_CORE_NULL);
  TEST_EXPECT(input.seek == NE_CORE_NULL);
  TEST_EXPECT(input.free != NE_CORE_NULL);

  test_stream(&input, table->is_final_run, table);

  ne_core_stream output;
  ne_intrinsic_memory_set(&output, 1, sizeof(output));
  TEST_CLEAR_RESULT();
  ne_io_get_output(table->result, &output);
  TEST_EXPECT_RESULT(table->expected_result);

  TEST_EXPECT(output.read == NE_CORE_NULL);
  TEST_EXPECT(output.write != NE_CORE_NULL);
  TEST_EXPECT(output.flush != NE_CORE_NULL);
  TEST_EXPECT(output.get_position == NE_CORE_NULL);
  TEST_EXPECT(output.get_size == NE_CORE_NULL);
  TEST_EXPECT(output.is_terminated == NE_CORE_NULL);
  TEST_EXPECT(output.seek == NE_CORE_NULL);
  TEST_EXPECT(output.free != NE_CORE_NULL);

  test_stream(&output, table->is_final_run, table);

  ne_core_stream error;
  ne_intrinsic_memory_set(&error, 1, sizeof(error));
  TEST_CLEAR_RESULT();
  ne_io_get_error(table->result, &error);
  TEST_EXPECT_RESULT(table->expected_result);

  TEST_EXPECT(error.read == NE_CORE_NULL);
  TEST_EXPECT(error.write != NE_CORE_NULL);
  TEST_EXPECT(error.flush != NE_CORE_NULL);
  TEST_EXPECT(error.get_position == NE_CORE_NULL);
  TEST_EXPECT(error.get_size == NE_CORE_NULL);
  TEST_EXPECT(error.is_terminated == NE_CORE_NULL);
  TEST_EXPECT(error.seek == NE_CORE_NULL);
  TEST_EXPECT(error.free != NE_CORE_NULL);

  test_stream(&error, table->is_final_run, table);
}

static void null_tests(test_table *table)
{
  ne_core_stream input;
  ne_intrinsic_memory_set(&input, 1, sizeof(input));
  TEST_CLEAR_RESULT();
  ne_io_get_input(table->result, &input);
  TEST_EXPECT_RESULT(table->expected_result);
  TEST_EXPECT(test_memory_compare_value(&input, 1, sizeof(input)) == 0);

  ne_core_stream output;
  ne_intrinsic_memory_set(&output, 1, sizeof(output));
  TEST_CLEAR_RESULT();
  ne_io_get_output(table->result, &output);
  TEST_EXPECT_RESULT(table->expected_result);
  TEST_EXPECT(test_memory_compare_value(&input, 1, sizeof(input)) == 0);

  ne_core_stream error;
  ne_intrinsic_memory_set(&error, 1, sizeof(error));
  TEST_CLEAR_RESULT();
  ne_io_get_error(table->result, &error);
  TEST_EXPECT_RESULT(table->expected_result);
  TEST_EXPECT(test_memory_compare_value(&input, 1, sizeof(input)) == 0);
}

static void shared_tests(test_table *table) { (void)table; }

static void exit_tests(test_table *table) { (void)table; }

void test_io(ne_core_bool simulated_environment)
{
  TEST_RUN(ne_io_supported, NE_CORE_INVALID_PERMISSION);
}
