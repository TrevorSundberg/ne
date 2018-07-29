// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_core/test_core.h"
NE_BEGIN

NE_DEFINE_PACKAGE(test_core);

ne_core_bool test_validate(ne_core_bool value, const char* message)
{
  if (!value)
    ne_core_error(ne_core_null, message);

  // Return the value right back out so we can use it in an or statement.
  return value;
}

uint64_t test_string_length(const char* string)
{
  if (!string)
    return 0;

  const char* iterator = string;
  while (*iterator++ != '\0');
  return iterator - string;
}

void test_memory_set(void* memory, uint8_t value, uint64_t size)
{
  for (uint64_t i = 0; i < size; ++i)
    ((uint8_t*)memory)[i] = value;
}

int64_t test_memory_compare(void* a, void* b, uint64_t size)
{
  for (uint64_t i = 0; i < size; ++i)
  {
    uint8_t a_byte = ((uint8_t*)a)[i];
    uint8_t b_byte = ((uint8_t*)b)[i];
    if (a_byte != b_byte)
      return a_byte - b_byte;
  }
  return 0;
}

int64_t test_memory_compare_value(void* memory, uint8_t value, uint64_t size)
{
  for (uint64_t i = 0; i < size; ++i)
  {
    uint8_t byte = ((uint8_t*)memory)[i];
    if (byte != value)
      return byte - value;
  }
  return 0;
}

static void test_full(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, test_table* table)
{
  TEST_CLEAR_RESULT();
  table->full_tests(is_success_out, result, expected_result, table->user_data);
  TEST_CLEAR_RESULT();
  table->shared_tests(is_success_out, result, expected_result, table->user_data);
}

static void test_null(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, test_table* table)
{
  TEST_CLEAR_RESULT();
  table->null_tests(is_success_out, result, expected_result, table->user_data);
  TEST_CLEAR_RESULT();
  table->shared_tests(is_success_out, result, expected_result, table->user_data);
}

static void test_all(ne_core_bool* is_success_out, uint64_t* result, test_table* table)
{
  TEST_CLEAR_RESULT();
  if (table->supported(result))
  {
    TEST_EXPECT_RESULT("supported", ne_core_result_success);

    // We can always call major/minor version checks.
    TEST_CLEAR_RESULT();
    TEST_EXPECT("version_linked_major", table->version_linked_major(result) >= table->version_header_major);
    TEST_EXPECT_RESULT("version_linked_major", ne_core_result_success);
    TEST_CLEAR_RESULT();
    TEST_EXPECT("version_linked_minor", table->version_linked_minor(result) >= table->version_header_minor);
    TEST_EXPECT_RESULT("version_linked_minor", ne_core_result_success);

    TEST_CLEAR_RESULT();
    if (table->request_permission(result))
    {
      TEST_EXPECT_RESULT("request_permission", ne_core_result_success);

      // We got permission so run both full and shared tests.
      TEST_CLEAR_RESULT();
      test_full(is_success_out, result, ne_core_result_success, table);
    }
    else
    {
      TEST_EXPECT_RESULT("request_permission", ne_core_result_permission_denied);

      // We don't have permission, so check that all tests return null/0 as well as the permission denied result.
      TEST_CLEAR_RESULT();
      test_null(is_success_out, result, ne_core_result_permission_denied, table);
    }
  }
  else
  {
    TEST_EXPECT_RESULT("supported", ne_core_result_not_supported);

    // Verify that the major and minor return not supported.
    TEST_CLEAR_RESULT();
    TEST_EXPECT("version_linked_major", table->version_linked_major(result) == 0);
    TEST_EXPECT_RESULT("version_linked_major", ne_core_result_not_supported);
    TEST_CLEAR_RESULT();
    TEST_EXPECT("version_linked_minor", table->version_linked_minor(result) == 0);
    TEST_EXPECT_RESULT("version_linked_minor", ne_core_result_not_supported);

    // Requesting permission should also return not-supported.
    TEST_CLEAR_RESULT();
    table->request_permission(result);
    TEST_EXPECT_RESULT("request_permission", ne_core_result_not_supported);

    TEST_CLEAR_RESULT();
    test_null(is_success_out, result, ne_core_result_not_supported, table);
  }
}

ne_core_bool test_run(test_table* table)
{
  ne_core_bool is_success = ne_core_true;
  uint64_t last_result = ne_core_result_not_set;

  // Test everything with a valid result pointer.
  test_all(&is_success, &last_result, table);

  // Test everything with a null result pointer.
  test_all(&is_success, ne_core_null, table);

  return is_success;
}

static void ne_core_full_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();

  TEST_EXPECT("ne_core_empty_string", ne_core_empty_string != ne_core_null && *ne_core_empty_string == '\0');

  // Test the unit test functions too!
  TEST_EXPECT("test_string_length", test_string_length(ne_core_null) == 0);
  TEST_EXPECT("test_string_length", test_string_length("") == 0);
  TEST_EXPECT("test_string_length", test_string_length("hello") == 5);

  uint8_t buffer1[32] = { 1 };
  uint8_t buffer2[32] = { 0 };

  TEST_EXPECT("test_memory_compare", test_memory_compare(buffer1, buffer2, sizeof(buffer1)) > 0);
  TEST_EXPECT("test_memory_compare", test_memory_compare(buffer2, buffer1, sizeof(buffer1)) < 0);

  TEST_EXPECT("test_memory_compare_value", test_memory_compare_value(buffer1, 0, sizeof(buffer1)) > 0);
  TEST_EXPECT("test_memory_compare_value", test_memory_compare_value(buffer2, 0, sizeof(buffer1)) == 0);

  test_memory_set(buffer1, 0, sizeof(buffer1));
  TEST_EXPECT("test_memory_compare", test_memory_compare(buffer1, buffer2, sizeof(buffer1)) == 0);

  test_memory_set(buffer1, 1, sizeof(buffer1));
  TEST_EXPECT("test_memory_compare", test_memory_compare(buffer1, buffer2, sizeof(buffer1)) > 0);
  test_memory_set(buffer2, 1, sizeof(buffer2));
  TEST_EXPECT("test_memory_compare", test_memory_compare(buffer1, buffer2, sizeof(buffer1)) == 0);
}

static void ne_core_null_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();

  TEST_EXPECT("ne_core_empty_string", ne_core_empty_string == ne_core_null);
}

static void ne_core_shared_tests(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data)
{
  TEST_IGNORE_UNUSED_PARAMETERS();

  ne_core_hello_world(result);
  TEST_EXPECT_RESULT("ne_core_hello_world", expected_result);
}

ne_core_bool test_core()
{
  TEST_RUN(ne_core);
}

NE_END
