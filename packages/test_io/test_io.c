// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_io/test_io.h"
NE_BEGIN

NE_DEFINE_PACKAGE(test_io);

static void ne_io_full_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();

  uint8_t buffer[32] = { 0 };

  ne_core_stream input;
  test_memory_set(&input, 0, sizeof(input));
  TEST_CLEAR_RESULT();
  ne_io_get_input(result, &input);
  TEST_EXPECT_RESULT("ne_io_get_input", expected_result);

  TEST_CLEAR_RESULT();
  input.read(result, &input, buffer, sizeof(buffer), ne_core_false);
  TEST_EXPECT_RESULT("read", expected_result);

  TEST_EXPECT("write", input.write == ne_core_null);
  TEST_EXPECT("flush", input.flush == ne_core_null);

  TEST_CLEAR_RESULT();
  input.get_position(result, &input);
  TEST_EXPECT_RESULT("get_position", expected_result);

  TEST_CLEAR_RESULT();
  input.get_length(result, &input);
  TEST_EXPECT_RESULT("get_length", expected_result);

  TEST_CLEAR_RESULT();
  input.is_eof(result, &input);
  TEST_EXPECT_RESULT("is_eof", expected_result);

  TEST_EXPECT("seek", input.seek == ne_core_null);
  TEST_EXPECT("free", input.free == ne_core_null);


  ne_core_stream output;
  test_memory_set(&output, 0, sizeof(output));
  TEST_CLEAR_RESULT();
  ne_io_get_output(result, &output);
  TEST_EXPECT_RESULT("ne_io_get_output", expected_result);

  TEST_EXPECT("read", output.read == ne_core_null);

  TEST_CLEAR_RESULT();
  output.write(result, &output, buffer, sizeof(buffer), ne_core_false);
  TEST_EXPECT_RESULT("write", expected_result);

  TEST_CLEAR_RESULT();
  output.flush(result, &output);
  TEST_EXPECT_RESULT("flush", expected_result);

  TEST_CLEAR_RESULT();
  output.get_position(result, &output);
  TEST_EXPECT_RESULT("get_position", expected_result);

  TEST_CLEAR_RESULT();
  output.get_length(result, &output);
  TEST_EXPECT_RESULT("get_length", expected_result);

  TEST_EXPECT("is_eof", output.is_eof == ne_core_null);
  TEST_EXPECT("seek", output.seek == ne_core_null);
  TEST_EXPECT("free", output.free == ne_core_null);
}

static void ne_io_null_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();

  ne_core_stream input;
  test_memory_set(&input, 0, sizeof(input));
  TEST_CLEAR_RESULT();
  ne_io_get_input(result, &input);
  TEST_EXPECT_RESULT("ne_io_get_input", expected_result);
  TEST_EXPECT("ne_io_get_input", test_memory_compare_value(&input, 0, sizeof(input)) == 0);

  ne_core_stream output;
  test_memory_set(&output, 0, sizeof(output));
  TEST_CLEAR_RESULT();
  ne_io_get_output(result, &output);
  TEST_EXPECT_RESULT("ne_io_get_output", expected_result);
  TEST_EXPECT("ne_io_get_output", test_memory_compare_value(&input, 0, sizeof(input)) == 0);
}

static void ne_io_shared_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();
}

ne_core_bool test_io()
{
  TEST_RUN(ne_io);
}

NE_END
