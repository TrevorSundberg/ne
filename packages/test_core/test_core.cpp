// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_core/test_core.h"
NE_BEGIN

NE_DEFINE_PACKAGE(test_core);

ne_core_bool test_core_validate(ne_core_bool value, const char *message) {
  if (!value)
    ne_core_error(ne_core_null, message);

  // Return the value right back out so we can use it in an or statement.
  return value;
}

uint64_t test_core_string_length(const char *string) {
  if (!string)
    return 0;

  const char *iterator = string;
  while (*iterator != '\0')
    ++iterator;
  return iterator - string;
}

int64_t test_core_string_compare(const char *a, const char *b) {
  if (a == b)
    return 0;
  if (!a)
    return -1;
  if (!b)
    return +1;

  for (;;) {
    if (*a != *b)
      return *a - *b;

    // We only need to check 'a' here since we already verified they are the
    // same above.
    if (*a == 0)
      return 0;

    ++a;
    ++b;
  }
}

int64_t test_core_memory_compare_value(void *memory, uint8_t value,
                                       uint64_t size) {
  for (uint64_t i = 0; i < size; ++i) {
    uint8_t byte = ((uint8_t *)memory)[i];
    if (byte != value)
      return byte - value;
  }
  return 0;
}

static void test_core_full(ne_core_bool *is_success_out, uint64_t *result,
                           uint64_t expected_result, test_core_table *table) {
  TEST_CORE_CLEAR_RESULT();
  table->full_tests(is_success_out, result, expected_result, table->user_data);
  TEST_CORE_CLEAR_RESULT();
  table->shared_tests(is_success_out, result, expected_result,
                      table->user_data);
}

static void test_core_null(ne_core_bool *is_success_out, uint64_t *result,
                           uint64_t expected_result, test_core_table *table) {
  TEST_CORE_CLEAR_RESULT();
  table->null_tests(is_success_out, result, expected_result, table->user_data);
  TEST_CORE_CLEAR_RESULT();
  table->shared_tests(is_success_out, result, expected_result,
                      table->user_data);
}

static void test_core_all(ne_core_bool *is_success_out, uint64_t *result,
                          test_core_table *table) {
  TEST_CORE_CLEAR_RESULT();
  if (table->supported(result)) {
    TEST_CORE_EXPECT_RESULT("supported", ne_core_result_success);

    // We can always call major/minor version checks.
    TEST_CORE_CLEAR_RESULT();
    TEST_CORE_EXPECT("version_linked_major",
                     table->version_linked_major(result) >=
                         table->version_header_major);
    TEST_CORE_EXPECT_RESULT("version_linked_major", ne_core_result_success);
    TEST_CORE_CLEAR_RESULT();
    TEST_CORE_EXPECT("version_linked_minor",
                     table->version_linked_minor(result) >=
                         table->version_header_minor);
    TEST_CORE_EXPECT_RESULT("version_linked_minor", ne_core_result_success);

    TEST_CORE_CLEAR_RESULT();
    if (table->request_permission(result)) {
      TEST_CORE_EXPECT_RESULT("request_permission", ne_core_result_success);

      // We got permission so run both full and shared tests.
      TEST_CORE_CLEAR_RESULT();
      test_core_full(is_success_out, result, ne_core_result_success, table);
    } else {
      TEST_CORE_EXPECT_RESULT("request_permission",
                              ne_core_result_permission_denied);

      // We don't have permission, so check that all tests return null/0 as well
      // as the permission denied result.
      TEST_CORE_CLEAR_RESULT();
      test_core_null(is_success_out, result, ne_core_result_permission_denied,
                     table);
    }
  } else {
    TEST_CORE_EXPECT_RESULT("supported", ne_core_result_not_supported);

    // Verify that the major and minor return not supported.
    TEST_CORE_CLEAR_RESULT();
    TEST_CORE_EXPECT("version_linked_major",
                     table->version_linked_major(result) == 0);
    TEST_CORE_EXPECT_RESULT("version_linked_major",
                            ne_core_result_not_supported);
    TEST_CORE_CLEAR_RESULT();
    TEST_CORE_EXPECT("version_linked_minor",
                     table->version_linked_minor(result) == 0);
    TEST_CORE_EXPECT_RESULT("version_linked_minor",
                            ne_core_result_not_supported);

    // Requesting permission should also return not-supported.
    TEST_CORE_CLEAR_RESULT();
    table->request_permission(result);
    TEST_CORE_EXPECT_RESULT("request_permission", ne_core_result_not_supported);

    TEST_CORE_CLEAR_RESULT();
    test_core_null(is_success_out, result, ne_core_result_not_supported, table);
  }
}

ne_core_bool test_core_run(test_core_table *table) {
  ne_core_bool is_success = ne_core_true;
  uint64_t last_result = ne_core_result_not_set;

  // Test everything with a valid result pointer.
  test_core_all(&is_success, &last_result, table);

  // Test everything with a null result pointer.
  test_core_all(&is_success, ne_core_null, table);

  return is_success;
}

static void ne_core_full_tests(ne_core_bool *is_success_out, uint64_t *result,
                               uint64_t expected_result, void *user_data) {
  TEST_CORE_IGNORE_UNUSED_PARAMETERS();

  TEST_CORE_CLEAR_RESULT();
  void *allocation = ne_core_allocate(result, 1);
  TEST_CORE_EXPECT_RESULT("ne_core_allocate", expected_result);
  TEST_CORE_EXPECT("ne_core_allocate", allocation != ne_core_null);

  // Write a single byte to the allocated memory to ensure it's writable.
  if (allocation)
    *(uint8_t *)allocation = 0xFF;

  TEST_CORE_CLEAR_RESULT();
  ne_core_free(result, allocation);
  TEST_CORE_EXPECT_RESULT("ne_core_free", expected_result);

  // Allocate something so large that we expect it to fail.
  TEST_CORE_CLEAR_RESULT();
  TEST_CORE_EXPECT("ne_core_allocate",
                   ne_core_allocate(result, 0xFFFFFFFFFFFFFFFF) ==
                       ne_core_null);
  TEST_CORE_EXPECT_RESULT("ne_core_allocate", ne_core_result_allocation_failed);

  // Test the unit test functions too!
  TEST_CORE_EXPECT("test_string_length",
                   test_core_string_length(ne_core_null) == 0);
  TEST_CORE_EXPECT("test_string_length", test_core_string_length("") == 0);
  TEST_CORE_EXPECT("test_string_length", test_core_string_length("hello") == 5);

  TEST_CORE_EXPECT("test_string_compare",
                   test_core_string_compare("hello", "world") < 0);
  TEST_CORE_EXPECT("test_string_compare",
                   test_core_string_compare("world", "hello") > 0);
  TEST_CORE_EXPECT("test_string_compare",
                   test_core_string_compare("hello", "hello") == 0);
  char string_buffer[] = "cello";
  string_buffer[0] = 'h';
  TEST_CORE_EXPECT("test_string_compare",
                   test_core_string_compare("hello", string_buffer) == 0);

  uint8_t buffer1[32] = {127};
  uint8_t buffer2[32] = {0};

  TEST_CORE_EXPECT("test_core_memory_compare_value",
                   test_core_memory_compare_value(buffer1, 0, sizeof(buffer1)) >
                       0);
  TEST_CORE_EXPECT("test_core_memory_compare_value",
                   test_core_memory_compare_value(buffer1, 255, sizeof(buffer1)) <
                       0);
  TEST_CORE_EXPECT(
      "test_core_memory_compare_value",
      test_core_memory_compare_value(buffer2, 0, sizeof(buffer1)) == 0);
}

static void ne_core_null_tests(ne_core_bool *is_success_out, uint64_t *result,
                               uint64_t expected_result, void *user_data) {
  TEST_CORE_IGNORE_UNUSED_PARAMETERS();

  TEST_CORE_CLEAR_RESULT();
  TEST_CORE_EXPECT("ne_core_allocate",
                   ne_core_allocate(result, 1) == ne_core_null);
  TEST_CORE_EXPECT_RESULT("ne_core_allocate", expected_result);

  TEST_CORE_CLEAR_RESULT();
  ne_core_free(result, ne_core_null);
  TEST_CORE_EXPECT_RESULT("ne_core_free", expected_result);
}

static void ne_core_shared_tests(ne_core_bool *is_success_out, uint64_t *result,
                                 uint64_t expected_result, void *user_data) {
  TEST_CORE_IGNORE_UNUSED_PARAMETERS();

  ne_core_hello_world(result);
  TEST_CORE_EXPECT_RESULT("ne_core_hello_world", expected_result);
}

ne_core_bool test_core(void) { TEST_CORE_RUN(ne_core); }

NE_END
