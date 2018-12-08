// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_core/test_core.h"

#define TEST_CORE_RANDOM_A 1103515245ULL
#define TEST_CORE_RANDOM_C 12345ULL
#define TEST_CORE_RANDOM_M 18446744071562067968ULL
#define TEST_CORE_RANDOM_SEED 123456789ULL

static int32_t callback_counter = 0;

ne_core_bool test_core_validate(ne_core_bool value,
                                const char *file,
                                int64_t line,
                                const char *message)
{
  if (value != NE_CORE_TRUE)
  {
    ne_core_error(nullptr, file, line, message);
  }

  // Return the value right back out so we can use it in an or statement.
  return value;
}

uint64_t test_core_string_length(const char *string)
{
  if (string == nullptr)
  {
    return 0;
  }

  const char *iterator = string;
  while (*iterator != '\0')
  {
    ++iterator;
  }
  return static_cast<uint64_t>(iterator - string);
}

int64_t test_core_string_compare(const char *a, const char *b)
{
  if (a == b)
  {
    return 0;
  }
  if (a == nullptr)
  {
    return -1;
  }
  if (b == nullptr)
  {
    return +1;
  }

  for (;;)
  {
    if (*a != *b)
    {
      return *a - *b;
    }

    // We only need to check 'a' here since we already verified they are the
    // same above.
    if (*a == 0)
    {
      return 0;
    }

    ++a;
    ++b;
  }
}

int64_t test_core_memory_compare_value(void *memory,
                                       uint8_t value,
                                       uint64_t size)
{
  for (uint64_t i = 0; i < size; ++i)
  {
    uint8_t byte = (static_cast<uint8_t *>(memory))[i];
    if (byte != value)
    {
      return byte - value;
    }
  }
  return 0;
}

void test_core_random_initialize(void *memory, uint64_t size)
{
  uint64_t seed = TEST_CORE_RANDOM_SEED;
  for (uint64_t i = 0; i < size; ++i)
  {
    seed =
        (TEST_CORE_RANDOM_A * seed + TEST_CORE_RANDOM_C) % TEST_CORE_RANDOM_M;

    static_cast<uint8_t *>(memory)[i] = static_cast<uint8_t>(seed);
  }
}

int64_t test_core_random_compare(void *memory, uint64_t size)
{
  uint64_t seed = TEST_CORE_RANDOM_SEED;
  for (uint64_t i = 0; i < size; ++i)
  {
    seed =
        (TEST_CORE_RANDOM_A * seed + TEST_CORE_RANDOM_C) % TEST_CORE_RANDOM_M;

    uint8_t byte = static_cast<uint8_t *>(memory)[i];
    if (byte != static_cast<uint8_t>(seed))
    {
      return byte - static_cast<uint8_t>(seed);
    }
  }
  return 0;
}

void test_core_stream(ne_core_stream *stream,
                      ne_core_bool free_stream,
                      test_core_table *table)
{
  // If we're in a simulated environment, we want to check blocking operations.
  // For completness, first check non-blocking operations.
  // We also do NOT want to free the stream here, because we're going to
  // continue testing it.
  if (table->simulated_environment != NE_CORE_FALSE)
  {
    table->simulated_environment = NE_CORE_FALSE;
    test_core_stream(stream, NE_CORE_FALSE, table);
    table->simulated_environment = NE_CORE_TRUE;
  }

  ne_core_bool blocking = table->simulated_environment;

  // void (*seek)(uint64_t *result, ne_core_stream *stream,
  //              ne_core_stream_seek_origin origin, uint64_t position);

  // void (*free)(uint64_t *result, ne_core_stream *stream);

  uint8_t buffer1[TEST_CORE_SIMULATED_SIZE] = {0};
  uint8_t buffer2[TEST_CORE_SIMULATED_SIZE] = {0};

  // Test is_terminated.
  if (stream->is_terminated != nullptr)
  {
    // We expect the stream not to be terminated from the start.
    TEST_CORE_CLEAR_RESULT();
    ne_core_bool terminated = stream->is_terminated(table->result, stream);
    TEST_CORE_EXPECT_RESULT(table->expected_result);
    TEST_CORE_EXPECT(!terminated);
  }

  // Test get_position.
  if (stream->get_position != nullptr)
  {
    // Get the position of the stream to verify that it starts at 0.
    TEST_CORE_CLEAR_RESULT();
    uint64_t position = stream->get_position(table->result, stream);
    TEST_CORE_EXPECT_RESULT(table->expected_result);
    TEST_CORE_EXPECT(position == 0);
  }

  // Test read.
  if (stream->read != nullptr)
  {
    // Get the starting point.
    uint64_t position = 0;
    if (stream->get_position != nullptr)
    {
      TEST_CORE_CLEAR_RESULT();
      position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
    }

    // Initialize with random noise and read.
    test_core_random_initialize(buffer1, TEST_CORE_SIMULATED_SIZE);
    TEST_CORE_CLEAR_RESULT();
    uint64_t amount1 = stream->read(table->result, stream, buffer1,
                                    TEST_CORE_SIMULATED_SIZE, blocking);
    TEST_CORE_EXPECT_RESULT(table->expected_result);

    // Make sure some of the random data was overwritten.
    if (amount1 != 0)
    {
      TEST_CORE_EXPECT(test_core_random_compare(buffer1, amount1) != 0);
    }

    // Expect to read exact string in simulated environment.
    if (table->simulated_environment != NE_CORE_FALSE)
    {
      TEST_CORE_EXPECT(amount1 == TEST_CORE_SIMULATED_SIZE);
      TEST_CORE_EXPECT(
          ne_intrinsic_memory_compare(buffer1, TEST_CORE_SIMULATED_STREAM,
                                      TEST_CORE_SIMULATED_SIZE) == 0);
    }

    // Verify that the position was advanced as far as the read said.
    if (stream->get_position != nullptr)
    {
      TEST_CORE_CLEAR_RESULT();
      uint64_t new_position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
      TEST_CORE_EXPECT((new_position - position) == amount1);
    }

    // If we support seeking, go back to the beginning and read again.
    if (stream->seek != nullptr)
    {
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin,
                   position);
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      // Verify that the position is the same.
      if (stream->get_position != nullptr)
      {
        TEST_CORE_CLEAR_RESULT();
        uint64_t new_position = stream->get_position(table->result, stream);
        TEST_CORE_EXPECT_RESULT(table->expected_result);
        TEST_CORE_EXPECT(new_position == position);
      }

      // Read again into the second buffer.
      test_core_random_initialize(buffer2, TEST_CORE_SIMULATED_SIZE);
      TEST_CORE_CLEAR_RESULT();
      uint64_t amount2 = stream->read(table->result, stream, buffer2,
                                      TEST_CORE_SIMULATED_SIZE, blocking);
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      // We should have read the exact same thing.
      TEST_CORE_EXPECT(amount1 == amount2);
      TEST_CORE_EXPECT(ne_intrinsic_memory_compare(buffer1, buffer2, amount2) ==
                       0);

      // Go back to the beginning.
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin,
                   position);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
    }
  }

  // Test write.
  if (stream->write != nullptr)
  {
    // Get the starting point.
    uint64_t position = 0;
    if (stream->get_position != nullptr)
    {
      TEST_CORE_CLEAR_RESULT();
      position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
    }

    // Write data to the stream.
    TEST_CORE_CLEAR_RESULT();
    uint64_t amount1 =
        stream->write(table->result, stream, TEST_CORE_SIMULATED_STREAM,
                      TEST_CORE_SIMULATED_SIZE, blocking);
    TEST_CORE_EXPECT_RESULT(table->expected_result);

    // If we have the ability to flush, call it now.
    if (stream->flush != nullptr)
    {
      TEST_CORE_CLEAR_RESULT();
      stream->flush(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
    }

    // Verify that the position was advanced as far as the write said.
    if (stream->get_position != nullptr)
    {
      TEST_CORE_CLEAR_RESULT();
      uint64_t new_position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
      TEST_CORE_EXPECT((new_position - position) == amount1);
    }

    // If we support seeking, go back to the beginning and read what was
    // written.
    if (stream->seek != nullptr)
    {
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin,
                   position);
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      // Verify that the position is the same.
      if (stream->get_position != nullptr)
      {
        TEST_CORE_CLEAR_RESULT();
        uint64_t new_position = stream->get_position(table->result, stream);
        TEST_CORE_EXPECT_RESULT(table->expected_result);
        TEST_CORE_EXPECT(new_position == position);
      }

      // Read the written contents into the buffer.
      test_core_random_initialize(buffer2, TEST_CORE_SIMULATED_SIZE);
      TEST_CORE_CLEAR_RESULT();
      uint64_t amount2 = stream->read(table->result, stream, buffer2,
                                      TEST_CORE_SIMULATED_SIZE, blocking);
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      // We should have read the exact same thing.
      TEST_CORE_EXPECT(amount1 == amount2);
      TEST_CORE_EXPECT(ne_intrinsic_memory_compare(TEST_CORE_SIMULATED_STREAM,
                                                   buffer2, amount2) == 0);

      // Go back to the beginning.
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin,
                   position);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
    }
  }

  // Test flush.
  if (stream->flush != nullptr)
  {
    // We can't easily verify that flush succeeded, but we can call it for
    // coverage.
    TEST_CORE_CLEAR_RESULT();
    stream->flush(table->result, stream);
    TEST_CORE_EXPECT_RESULT(table->expected_result);
  }

  // Test get_size.
  if (stream->get_size != nullptr)
  {
    TEST_CORE_CLEAR_RESULT();
    uint64_t size = stream->get_size(table->result, stream);
    TEST_CORE_EXPECT_RESULT(table->expected_result);

    if (table->simulated_environment != NE_CORE_FALSE)
    {
      TEST_CORE_EXPECT(size == TEST_CORE_SIMULATED_SIZE);
    }
  }

  // Test seek.
  if (stream->seek != nullptr)
  {
    // Call these for coverage. We'll test them below if we have get_position.
    TEST_CORE_CLEAR_RESULT();
    stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, 0);
    TEST_CORE_EXPECT_RESULT(table->expected_result);
    TEST_CORE_CLEAR_RESULT();
    stream->seek(table->result, stream, ne_core_stream_seek_origin_current, 1);
    TEST_CORE_EXPECT_RESULT(table->expected_result);
    TEST_CORE_CLEAR_RESULT();
    stream->seek(table->result, stream, ne_core_stream_seek_origin_end, 0);
    TEST_CORE_EXPECT_RESULT(table->expected_result);

    // If we have get_position, we can do a lot more to verift the streams.
    if (stream->get_position != nullptr)
    {
      // Beginning.
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, 0);
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      TEST_CORE_CLEAR_RESULT();
      uint64_t new_position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
      TEST_CORE_EXPECT(new_position == 0);

      // Beginning with positive offset.
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, 1);
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      TEST_CORE_CLEAR_RESULT();
      new_position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
      TEST_CORE_EXPECT(new_position == 1);

      // Beginning with negative offset (clamped).
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin,
                   static_cast<uint64_t>(-1));
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      TEST_CORE_CLEAR_RESULT();
      new_position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
      TEST_CORE_EXPECT(new_position == 0);

      // Current with positive offset.
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_current,
                   1);
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      TEST_CORE_CLEAR_RESULT();
      new_position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
      TEST_CORE_EXPECT(new_position == 1);

      // Current with negative offset.
      TEST_CORE_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_current,
                   static_cast<uint64_t>(-1));
      TEST_CORE_EXPECT_RESULT(table->expected_result);

      TEST_CORE_CLEAR_RESULT();
      new_position = stream->get_position(table->result, stream);
      TEST_CORE_EXPECT_RESULT(table->expected_result);
      TEST_CORE_EXPECT(new_position == 0);

      // We can only test the end properly if we have get_size.
      if (stream->get_size != nullptr)
      {
        TEST_CORE_CLEAR_RESULT();
        uint64_t size = stream->get_size(table->result, stream);
        TEST_CORE_EXPECT_RESULT(table->expected_result);

        // End.
        TEST_CORE_CLEAR_RESULT();
        stream->seek(table->result, stream, ne_core_stream_seek_origin_end, 0);
        TEST_CORE_EXPECT_RESULT(table->expected_result);

        TEST_CORE_CLEAR_RESULT();
        new_position = stream->get_position(table->result, stream);
        TEST_CORE_EXPECT_RESULT(table->expected_result);
        TEST_CORE_EXPECT(new_position == size);

        // End with negative offset.
        if (size != 0)
        {
          TEST_CORE_CLEAR_RESULT();
          stream->seek(table->result, stream, ne_core_stream_seek_origin_end,
                       static_cast<uint64_t>(-1));
          TEST_CORE_EXPECT_RESULT(table->expected_result);

          TEST_CORE_CLEAR_RESULT();
          new_position = stream->get_position(table->result, stream);
          TEST_CORE_EXPECT_RESULT(table->expected_result);

          TEST_CORE_EXPECT(new_position == (size - 1));
        }

        // End with positive offset (clamped).
        TEST_CORE_CLEAR_RESULT();
        stream->seek(table->result, stream, ne_core_stream_seek_origin_end, 1);
        TEST_CORE_EXPECT_RESULT(table->expected_result);

        TEST_CORE_CLEAR_RESULT();
        new_position = stream->get_position(table->result, stream);
        TEST_CORE_EXPECT_RESULT(table->expected_result);
        TEST_CORE_EXPECT(new_position == size);
      }
    }

    // Finally, seek back to the beginning for future tests.
    // Note that this only happens if we support seek.
    TEST_CORE_CLEAR_RESULT();
    stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, 0);
    TEST_CORE_EXPECT_RESULT(table->expected_result);
  }

  // Test free (optionally).
  if (stream->free != nullptr && free_stream != NE_CORE_FALSE)
  {
    TEST_CORE_CLEAR_RESULT();
    stream->free(table->result, stream);
    TEST_CORE_EXPECT_RESULT(table->expected_result);
  }
}

static void test_core_full(test_core_table *table)
{
  TEST_CORE_CLEAR_RESULT();
  table->full_tests(table);
  TEST_CORE_CLEAR_RESULT();
  table->shared_tests(table);
}

static void test_core_null(test_core_table *table)
{
  TEST_CORE_CLEAR_RESULT();
  table->null_tests(table);
  TEST_CORE_CLEAR_RESULT();
  table->shared_tests(table);
}

static void test_core_query_permission_granted_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_core_table *>(const_cast<void *>(user_data));
  TEST_CORE_EXPECT(event->permission == table->permission);
  TEST_CORE_EXPECT(event->current_state == ne_core_permission_state_granted);
  TEST_CORE_EXPECT(event->previous_state == ne_core_permission_state_prompt);

  // We got permission so run both full and shared tests.
  TEST_CORE_CLEAR_RESULT();
  table->expected_result = NE_CORE_RESULT_SUCCESS;
  test_core_full(table);
}

static void test_core_query_permission_denied_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_core_table *>(const_cast<void *>(user_data));
  TEST_CORE_EXPECT(event->permission == table->permission);
  TEST_CORE_EXPECT(event->current_state == ne_core_permission_state_denied);
  TEST_CORE_EXPECT(event->previous_state == ne_core_permission_state_prompt);

  // We don't have permission, so check that all tests return null/0 as well
  // as the permission denied result.
  TEST_CORE_CLEAR_RESULT();
  table->expected_result = NE_CORE_RESULT_PERMISSION_DENIED;
  test_core_null(table);
}

static void test_core_request_permission_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_core_table *>(const_cast<void *>(user_data));
  TEST_CORE_EXPECT(event->permission == table->permission);
  TEST_CORE_EXPECT(event->current_state != ne_core_permission_state_prompt);
  TEST_CORE_EXPECT(event->previous_state == ne_core_permission_state_prompt);

  if (event->current_state == ne_core_permission_state_granted)
  {
    // We SHOULD have permission.
    TEST_CORE_CLEAR_RESULT();
    ne_core_query_permission(table->result, table->permission,
                             &test_core_query_permission_granted_callback,
                             table);
    TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
  }
  else
  {
    TEST_CORE_EXPECT(event->current_state == ne_core_permission_state_denied);

    // We SHOULD NOT have permission.
    TEST_CORE_CLEAR_RESULT();
    ne_core_query_permission(table->result, table->permission,
                             &test_core_query_permission_denied_callback,
                             table);
    TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
  }
}

static void test_core_query_permission_prompt_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_core_table *>(const_cast<void *>(user_data));
  TEST_CORE_EXPECT(event->permission == table->permission);
  TEST_CORE_EXPECT(event->current_state == ne_core_permission_state_prompt);
  TEST_CORE_EXPECT(event->previous_state == ne_core_permission_state_prompt);
}

static void test_core_query_permission_invalid_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_core_table *>(const_cast<void *>(user_data));
  TEST_CORE_EXPECT(event->permission == table->permission);
  TEST_CORE_EXPECT(event->current_state == ne_core_permission_state_invalid);
  TEST_CORE_EXPECT(event->previous_state == ne_core_permission_state_invalid);
}

static void test_core_all(test_core_table *table)
{
  TEST_CORE_CLEAR_RESULT();
  ne_core_bool supported = table->supported(table->result);
  TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);

  if (supported != NE_CORE_FALSE)
  {
    if (table->permission != NE_CORE_INVALID_PERMISSION)
    {
      // To test permissions, the core must be supported.
      TEST_CORE_EXPECT(ne_core_supported(nullptr) == NE_CORE_TRUE);

      // Permissions should currently be 'prompt'.
      TEST_CORE_CLEAR_RESULT();
      ne_core_query_permission(table->result, table->permission,
                               &test_core_query_permission_prompt_callback,
                               table);
      TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);

      // Request permission and wait for the granted/denied callback.
      TEST_CORE_CLEAR_RESULT();
      ne_core_request_permission(table->result, &table->permission, 1, nullptr,
                                 &test_core_request_permission_callback, table);
      TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
    }
    else
    {
      // No permission check existed so we run both full and shared tests.
      TEST_CORE_CLEAR_RESULT();
      table->expected_result = NE_CORE_RESULT_SUCCESS;
      test_core_full(table);
    }
  }
  else
  {
    if (table->permission != NE_CORE_INVALID_PERMISSION)
    {
      // To test permissions, the core must be supported.
      TEST_CORE_EXPECT(ne_core_supported(nullptr) == NE_CORE_TRUE);

      TEST_CORE_CLEAR_RESULT();
      ne_core_query_permission(table->result, table->permission,
                               &test_core_query_permission_invalid_callback,
                               table);
      TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);

      TEST_CORE_CLEAR_RESULT();
      ne_core_request_permission(table->result, &table->permission, 1, nullptr,
                                 &test_core_query_permission_invalid_callback,
                                 table);
      TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
    }

    TEST_CORE_CLEAR_RESULT();
    table->expected_result = NE_CORE_RESULT_NOT_SUPPORTED;
    test_core_null(table);
  }
}

static void test_core_on_exit(const ne_core_exit_event *event,
                              const void *user_data)
{
  (void)event;
  auto table = static_cast<test_core_table *>(const_cast<void *>(user_data));
  TEST_CORE_CLEAR_RESULT();
  table->exit_tests(table);
}

void test_core_run(test_core_table *table)
{
  table->success = NE_CORE_TRUE;
  table->is_final_run = NE_CORE_FALSE;
  uint64_t result = NE_CORE_RESULT_NOT_SET;
  table->result = &result;

  // Test everything with a valid result pointer.
  test_core_all(table);

  // Test everything with a null result pointer.
  table->is_final_run = NE_CORE_TRUE;
  table->result = nullptr;
  test_core_all(table);

  // Because of events, the test may continue well past this point so we can't
  // return a success or failure here, so we run the exit tests below.
  if (ne_core_supported(nullptr) != NE_CORE_FALSE)
  {
    TEST_CORE_CLEAR_RESULT();
    ne_core_on_exit(table->result, &test_core_on_exit, table);
    TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
  }
  else
  {
    TEST_CORE_CLEAR_RESULT();
    table->exit_tests(table);
  }
}

static void test_frame_callback(const ne_core_frame_event *event,
                                const void *user_data)
{
  (void)event;
  (void)user_data;
  ++callback_counter;
}

static void full_tests(test_core_table *table)
{
  TEST_CORE_CLEAR_RESULT();
  void *allocation = ne_core_allocate(table->result, 1);
  TEST_CORE_EXPECT_RESULT(table->expected_result);
  TEST_CORE_EXPECT(allocation != nullptr);

  // Write a single byte to the allocated memory to ensure it's writable.
  if (allocation != nullptr)
  {
    *static_cast<uint8_t *>(allocation) = 0xFF;
  }

  TEST_CORE_CLEAR_RESULT();
  ne_core_free(table->result, allocation);
  TEST_CORE_EXPECT_RESULT(table->expected_result);

  // Allocate something so large that we expect it to fail.
  TEST_CORE_CLEAR_RESULT();
  TEST_CORE_EXPECT(ne_core_allocate(table->result, 0xFFFFFFFFFFFFFFFF) ==
                   nullptr);
  TEST_CORE_EXPECT_RESULT(NE_CORE_RESULT_ALLOCATION_FAILED);

  // Test the unit test functions too!
  TEST_CORE_EXPECT(test_core_string_length(nullptr) == 0);
  TEST_CORE_EXPECT(test_core_string_length("") == 0);
  TEST_CORE_EXPECT(test_core_string_length("hello") == 5);

  TEST_CORE_EXPECT(test_core_string_compare("hello", "world") < 0);
  TEST_CORE_EXPECT(test_core_string_compare("world", "hello") > 0);
  TEST_CORE_EXPECT(test_core_string_compare("hello", "hello") == 0);
  char string_buffer[] = "cello";
  string_buffer[0] = 'h';
  TEST_CORE_EXPECT(test_core_string_compare("hello", string_buffer) == 0);

  uint8_t buffer1[32] = {127};
  uint8_t buffer2[32] = {0};

  TEST_CORE_EXPECT(test_core_memory_compare_value(buffer1, 0, sizeof(buffer1)) >
                   0);
  TEST_CORE_EXPECT(
      test_core_memory_compare_value(buffer1, 255, sizeof(buffer1)) < 0);
  TEST_CORE_EXPECT(
      test_core_memory_compare_value(buffer2, 0, sizeof(buffer1)) == 0);

  TEST_CORE_CLEAR_RESULT();
  ne_core_request_frame(table->result, &test_frame_callback, table);
  TEST_CORE_EXPECT_RESULT(table->expected_result);
}

static void null_tests(test_core_table *table)
{
  TEST_CORE_CLEAR_RESULT();
  TEST_CORE_EXPECT(ne_core_allocate(table->result, 1) == nullptr);
  TEST_CORE_EXPECT_RESULT(table->expected_result);

  TEST_CORE_CLEAR_RESULT();
  ne_core_free(table->result, nullptr);
  TEST_CORE_EXPECT_RESULT(table->expected_result);
}

static void shared_tests(test_core_table *table)
{
  TEST_CORE_CLEAR_RESULT();
  ne_core_hello_world(table->result);
  TEST_CORE_EXPECT_RESULT(table->expected_result);
}

static void exit_tests(test_core_table *table)
{
  TEST_CORE_EXPECT(callback_counter == 2);
}

void test_core(ne_core_bool simulated_environment)
{
  TEST_CORE_RUN(ne_core_supported, NE_CORE_INVALID_PERMISSION);
}

int32_t ne_core_main(int32_t argc, char *argv[])
{
  auto simulated_environment = static_cast<ne_core_bool>(
      argc >= 2 &&
      test_core_string_compare(argv[1], "--simulated_environment") == 0);

  test_core(simulated_environment);
  // test_io(simulated_environment);

  // We want to return 0 for success, and 1 for error.
  return 0;
}
