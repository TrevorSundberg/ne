/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_core/test_core.h"

static int32_t frame_counter = 0;
static int32_t exit_counter = 0;

static void test_frame_callback(const ne_core_frame_event *event,
                                const void *user_data)
{
  (void)event;
  (void)user_data;
  ++frame_counter;
}

static void test_exit_callback(const ne_core_exit_event *event,
                               const void *user_data)
{
  (void)event;
  auto table = static_cast<test_table *>(const_cast<void *>(user_data));
  TEST_EXPECT(frame_counter == 2);
  ++exit_counter;
}

static void full_tests(test_table *table)
{
  TEST_CLEAR_RESULT();
  const char *platform = ne_core_get_platform_name(table->result);
  TEST_EXPECT(platform != nullptr && *platform != '\0');
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  const char *guid = ne_core_get_application_guid(table->result);
  TEST_EXPECT(guid != nullptr && *guid != '\0');
  TEST_EXPECT(
      test_string_characters("abcdefghijklmnopqrstuvwxyz0123456789-", guid));
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  void *allocation = ne_core_allocate(table->result, 1);
  TEST_EXPECT_TABLE_RESULT();
  TEST_EXPECT(allocation != nullptr);

  // Write a single byte to the allocated memory to ensure it's writable.
  if (allocation != nullptr)
  {
    *static_cast<uint8_t *>(allocation) = 0xFF;
  }

  TEST_CLEAR_RESULT();
  ne_core_free(table->result, allocation);
  TEST_EXPECT_TABLE_RESULT();

  // Allocate something so large that we expect it to fail.
  TEST_CLEAR_RESULT();
  TEST_EXPECT(ne_core_allocate(table->result, 0xFFFFFFFFFFFFFFFF) == nullptr);
  TEST_EXPECT_RESULT(NE_CORE_RESULT_ALLOCATION_FAILED);

  // Test the unit test functions too!
  TEST_EXPECT(test_string_length(nullptr) == 0);
  TEST_EXPECT(test_string_length("") == 0);
  TEST_EXPECT(test_string_length("hello") == 5);

  TEST_EXPECT(test_string_compare("hello", "world") < 0);
  TEST_EXPECT(test_string_compare("world", "hello") > 0);
  TEST_EXPECT(test_string_compare("hello", "hello") == 0);
  char string_buffer[] = "cello";
  string_buffer[0] = 'h';
  TEST_EXPECT(test_string_compare("hello", string_buffer) == 0);

  TEST_EXPECT(test_string_characters("abc", "babacaca") == NE_CORE_TRUE);
  TEST_EXPECT(test_string_characters("abc", "babacafa") == NE_CORE_FALSE);

  uint8_t buffer1[32] = {127};
  uint8_t buffer2[32] = {0};

  TEST_EXPECT(test_memory_compare_value(buffer1, 0, sizeof(buffer1)) > 0);
  TEST_EXPECT(test_memory_compare_value(buffer1, 255, sizeof(buffer1)) < 0);
  TEST_EXPECT(test_memory_compare_value(buffer2, 0, sizeof(buffer1)) == 0);
}

static void null_tests(test_table *table)
{
  TEST_CLEAR_RESULT();
  TEST_EXPECT(ne_core_get_platform_name(table->result) == nullptr);
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  TEST_EXPECT(ne_core_allocate(table->result, 1) == nullptr);
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  ne_core_free(table->result, nullptr);
  TEST_EXPECT_TABLE_RESULT();
}

static void shared_tests(test_table *table)
{
  TEST_CLEAR_RESULT();
  ne_core_hello_world(table->result);
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  ne_core_on_exit(table->result, &test_exit_callback, table);
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  ne_core_request_frame(table->result, &test_frame_callback, table);
  TEST_EXPECT_TABLE_RESULT();

  // We can't test these functions or call them for coverage because doing so
  // would cause the tests to fail (or exit prematurely).
  TEST_EXPECT(ne_core_exit != nullptr);
  TEST_EXPECT(ne_core_error != nullptr);
}

static void exit_tests(test_table *table)
{
  TEST_EXPECT(frame_counter == 2);
  TEST_EXPECT(exit_counter == 2);
}

void test_core(ne_core_bool simulated_environment)
{
  TEST_RUN(ne_core_supported, NE_CORE_PERMISSION_INVALID);
}
