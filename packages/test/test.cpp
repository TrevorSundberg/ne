/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test/test.h"

#define TEST_RANDOM_A 1103515245ULL
#define TEST_RANDOM_C 12345ULL
#define TEST_RANDOM_M 18446744071562067968ULL
#define TEST_RANDOM_SEED 123456789ULL

ne_core_bool test_validate(ne_core_bool value,
                           uint64_t error_result,
                           const char *file,
                           int64_t line,
                           const char *message)
{
  if (value != NE_CORE_TRUE)
  {
    ne_core_error(nullptr, error_result, file, line, message);
  }

  // Return the value right back out so we can use it in an or statement.
  return value;
}

uint64_t test_string_length(const char *string)
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

int64_t test_string_compare(const char *a, const char *b)
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

int64_t test_memory_compare_value(const void *memory,
                                  uint8_t value,
                                  uint64_t size)
{
  for (uint64_t i = 0; i < size; ++i)
  {
    uint8_t byte = (static_cast<const uint8_t *>(memory))[i];
    if (byte != value)
    {
      return byte - value;
    }
  }
  return 0;
}

void test_random_initialize(void *memory, uint64_t size)
{
  uint64_t seed = TEST_RANDOM_SEED;
  for (uint64_t i = 0; i < size; ++i)
  {
    seed = (TEST_RANDOM_A * seed + TEST_RANDOM_C) % TEST_RANDOM_M;

    static_cast<uint8_t *>(memory)[i] = static_cast<uint8_t>(seed);
  }
}

int64_t test_random_compare(void *memory, uint64_t size)
{
  uint64_t seed = TEST_RANDOM_SEED;
  for (uint64_t i = 0; i < size; ++i)
  {
    seed = (TEST_RANDOM_A * seed + TEST_RANDOM_C) % TEST_RANDOM_M;

    uint8_t byte = static_cast<uint8_t *>(memory)[i];
    if (byte != static_cast<uint8_t>(seed))
    {
      return byte - static_cast<uint8_t>(seed);
    }
  }
  return 0;
}

ne_core_bool test_string_characters(const char *expected, const char *input)
{
  for (;;)
  {
    char c = *input;

    if (c == '\0')
    {
      return NE_CORE_TRUE;
    }

    bool valid = false;
    for (const char *it = expected; *it != '\0'; ++it)
    {
      if (c == *it)
      {
        valid = true;
        break;
      }
    }

    if (!valid)
    {
      return NE_CORE_FALSE;
    }

    ++input;
  }
}

char *test_concatenate_allocate(const char *prefix, const char *postfix)
{
  uint64_t prefix_size = test_string_length(prefix);
  uint64_t postfix_size = test_string_length(postfix) + 1;
  uint64_t length = prefix_size + postfix_size;
  uint8_t *memory = ne_core_allocate(nullptr, length);
  ne_intrinsic_memory_copy(memory, prefix, prefix_size);
  ne_intrinsic_memory_copy(memory + prefix_size, postfix, postfix_size);
  return reinterpret_cast<char *>(memory);
}

void test_stream(ne_core_stream *stream,
                 ne_core_bool free_stream,
                 test_table *table)
{
  // If we're in a simulated environment, we want to check blocking operations.
  // For completness, first check non-blocking operations.
  // We also do NOT want to free the stream here, because we're going to
  // continue testing it.
  if (table->simulated_environment != NE_CORE_FALSE)
  {
    table->simulated_environment = NE_CORE_FALSE;
    test_stream(stream, NE_CORE_FALSE, table);
    table->simulated_environment = NE_CORE_TRUE;
  }

  ne_core_bool blocking = table->simulated_environment;

  uint8_t buffer1[TEST_SIMULATED_SIZE] = {0};
  uint8_t buffer2[TEST_SIMULATED_SIZE] = {0};

  // Test is_valid.
  if (stream->is_valid != nullptr)
  {
    // We expect the stream to be valid from the start.
    TEST_CLEAR_RESULT();
    ne_core_bool valid = stream->is_valid(table->result, stream);
    TEST_EXPECT_TABLE_RESULT();
    TEST_EXPECT(valid);
  }

  // Test get_position.
  if (stream->get_position != nullptr)
  {
    // Get the position of the stream to verify that it starts at 0.
    TEST_CLEAR_RESULT();
    uint64_t position = stream->get_position(table->result, stream);
    TEST_EXPECT_TABLE_RESULT();
    TEST_EXPECT(position == 0);
  }

  // Test read.
  if (stream->read != nullptr)
  {
    // Get the starting point.
    uint64_t position = 0;
    if (stream->get_position != nullptr)
    {
      TEST_CLEAR_RESULT();
      position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
    }

    // Initialize with random noise and read.
    test_random_initialize(buffer1, TEST_SIMULATED_SIZE);
    TEST_CLEAR_RESULT();
    uint64_t amount1 = stream->read(
        table->result, stream, buffer1, TEST_SIMULATED_SIZE, blocking);
    TEST_EXPECT_TABLE_RESULT();

    // Make sure some of the random data was overwritten.
    if (amount1 != 0)
    {
      TEST_EXPECT(test_random_compare(buffer1, amount1) != 0);
    }

    // Expect to read exact string in simulated environment.
    if (table->simulated_environment != NE_CORE_FALSE)
    {
      TEST_EXPECT(amount1 == TEST_SIMULATED_SIZE);
      TEST_EXPECT(ne_intrinsic_memory_compare(buffer1,
                                              TEST_SIMULATED_STREAM,
                                              TEST_SIMULATED_SIZE) == 0);
    }

    // Verify that the position was advanced as far as the read said.
    if (stream->get_position != nullptr)
    {
      TEST_CLEAR_RESULT();
      uint64_t new_position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
      TEST_EXPECT((new_position - position) == amount1);
    }

    // If we support seeking, go back to the beginning and read again.
    if (stream->seek != nullptr)
    {
      TEST_CLEAR_RESULT();
      stream->seek(table->result,
                   stream,
                   ne_core_stream_seek_origin_begin,
                   static_cast<int64_t>(position));
      TEST_EXPECT_TABLE_RESULT();

      // Verify that the position is the same.
      if (stream->get_position != nullptr)
      {
        TEST_CLEAR_RESULT();
        uint64_t new_position = stream->get_position(table->result, stream);
        TEST_EXPECT_TABLE_RESULT();
        TEST_EXPECT(new_position == position);
      }

      // Read again into the second buffer.
      test_random_initialize(buffer2, TEST_SIMULATED_SIZE);
      TEST_CLEAR_RESULT();
      uint64_t amount2 = stream->read(
          table->result, stream, buffer2, TEST_SIMULATED_SIZE, blocking);
      TEST_EXPECT_TABLE_RESULT();

      // We should have read the exact same thing.
      TEST_EXPECT(amount1 == amount2);
      TEST_EXPECT(ne_intrinsic_memory_compare(buffer1, buffer2, amount2) == 0);

      // Go back to the beginning.
      TEST_CLEAR_RESULT();
      stream->seek(table->result,
                   stream,
                   ne_core_stream_seek_origin_begin,
                   static_cast<int64_t>(position));
      TEST_EXPECT_TABLE_RESULT();
    }
  }

  // Test write.
  if (stream->write != nullptr)
  {
    // Get the starting point.
    uint64_t position = 0;
    if (stream->get_position != nullptr)
    {
      TEST_CLEAR_RESULT();
      position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
    }

    // Write data to the stream.
    TEST_CLEAR_RESULT();
    uint64_t amount1 = stream->write(table->result,
                                     stream,
                                     TEST_SIMULATED_STREAM,
                                     TEST_SIMULATED_SIZE,
                                     blocking);
    TEST_EXPECT_TABLE_RESULT();

    // If we have the ability to flush, call it now.
    if (stream->flush != nullptr)
    {
      TEST_CLEAR_RESULT();
      stream->flush(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
    }

    // Verify that the position was advanced as far as the write said.
    if (stream->get_position != nullptr)
    {
      TEST_CLEAR_RESULT();
      uint64_t new_position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
      TEST_EXPECT((new_position - position) == amount1);
    }

    // If we support seeking, go back to the beginning and read what was
    // written.
    if (stream->seek != nullptr)
    {
      TEST_CLEAR_RESULT();
      stream->seek(table->result,
                   stream,
                   ne_core_stream_seek_origin_begin,
                   static_cast<int64_t>(position));
      TEST_EXPECT_TABLE_RESULT();

      // Verify that the position is the same.
      if (stream->get_position != nullptr)
      {
        TEST_CLEAR_RESULT();
        uint64_t new_position = stream->get_position(table->result, stream);
        TEST_EXPECT_TABLE_RESULT();
        TEST_EXPECT(new_position == position);
      }

      // Read the written contents into the buffer.
      test_random_initialize(buffer2, TEST_SIMULATED_SIZE);
      TEST_CLEAR_RESULT();
      uint64_t amount2 = stream->read(
          table->result, stream, buffer2, TEST_SIMULATED_SIZE, blocking);
      TEST_EXPECT_TABLE_RESULT();

      // We should have read the exact same thing.
      TEST_EXPECT(amount1 == amount2);
      TEST_EXPECT(ne_intrinsic_memory_compare(
                      TEST_SIMULATED_STREAM, buffer2, amount2) == 0);

      // Go back to the beginning.
      TEST_CLEAR_RESULT();
      stream->seek(table->result,
                   stream,
                   ne_core_stream_seek_origin_begin,
                   static_cast<int64_t>(position));
      TEST_EXPECT_TABLE_RESULT();
    }
  }

  // Test flush.
  if (stream->flush != nullptr)
  {
    // We can't easily verify that flush succeeded, but we can call it for
    // coverage.
    TEST_CLEAR_RESULT();
    stream->flush(table->result, stream);
    TEST_EXPECT_TABLE_RESULT();
  }

  // Test get_size.
  if (stream->get_size != nullptr)
  {
    TEST_CLEAR_RESULT();
    uint64_t size = stream->get_size(table->result, stream);
    TEST_EXPECT_TABLE_RESULT();

    if (table->simulated_environment != NE_CORE_FALSE)
    {
      TEST_EXPECT(size == TEST_SIMULATED_SIZE);
    }
  }

  // Test seek.
  if (stream->seek != nullptr)
  {
    // Call these for coverage. We'll test them below if we have get_position.
    TEST_CLEAR_RESULT();
    stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, 0);
    TEST_EXPECT_TABLE_RESULT();
    TEST_CLEAR_RESULT();
    stream->seek(table->result, stream, ne_core_stream_seek_origin_current, 1);
    TEST_EXPECT_TABLE_RESULT();
    TEST_CLEAR_RESULT();
    stream->seek(table->result, stream, ne_core_stream_seek_origin_end, 0);
    TEST_EXPECT_TABLE_RESULT();

    // If we have get_position, we can do a lot more to verift the streams.
    if (stream->get_position != nullptr)
    {
      // Beginning.
      TEST_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, 0);
      TEST_EXPECT_TABLE_RESULT();

      TEST_CLEAR_RESULT();
      uint64_t new_position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
      TEST_EXPECT(new_position == 0);

      // Beginning with positive offset.
      TEST_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, 1);
      TEST_EXPECT_TABLE_RESULT();

      TEST_CLEAR_RESULT();
      new_position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
      TEST_EXPECT(new_position == 1);

      // Beginning with negative offset (clamped).
      TEST_CLEAR_RESULT();
      stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, -1);
      TEST_EXPECT_TABLE_RESULT();

      TEST_CLEAR_RESULT();
      new_position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
      TEST_EXPECT(new_position == 0);

      // Current with positive offset.
      TEST_CLEAR_RESULT();
      stream->seek(
          table->result, stream, ne_core_stream_seek_origin_current, 1);
      TEST_EXPECT_TABLE_RESULT();

      TEST_CLEAR_RESULT();
      new_position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
      TEST_EXPECT(new_position == 1);

      // Current with negative offset.
      TEST_CLEAR_RESULT();
      stream->seek(
          table->result, stream, ne_core_stream_seek_origin_current, -1);
      TEST_EXPECT_TABLE_RESULT();

      TEST_CLEAR_RESULT();
      new_position = stream->get_position(table->result, stream);
      TEST_EXPECT_TABLE_RESULT();
      TEST_EXPECT(new_position == 0);

      // We can only test the end properly if we have get_size.
      if (stream->get_size != nullptr)
      {
        TEST_CLEAR_RESULT();
        uint64_t size = stream->get_size(table->result, stream);
        TEST_EXPECT_TABLE_RESULT();

        // End.
        TEST_CLEAR_RESULT();
        stream->seek(table->result, stream, ne_core_stream_seek_origin_end, 0);
        TEST_EXPECT_TABLE_RESULT();

        TEST_CLEAR_RESULT();
        new_position = stream->get_position(table->result, stream);
        TEST_EXPECT_TABLE_RESULT();
        TEST_EXPECT(new_position == size);

        // End with negative offset.
        if (size != 0)
        {
          TEST_CLEAR_RESULT();
          stream->seek(
              table->result, stream, ne_core_stream_seek_origin_end, -1);
          TEST_EXPECT_TABLE_RESULT();

          TEST_CLEAR_RESULT();
          new_position = stream->get_position(table->result, stream);
          TEST_EXPECT_TABLE_RESULT();

          TEST_EXPECT(new_position == (size - 1));
        }

        // End with positive offset (clamped).
        TEST_CLEAR_RESULT();
        stream->seek(table->result, stream, ne_core_stream_seek_origin_end, 1);
        TEST_EXPECT_TABLE_RESULT();

        TEST_CLEAR_RESULT();
        new_position = stream->get_position(table->result, stream);
        TEST_EXPECT_TABLE_RESULT();
        TEST_EXPECT(new_position == size);
      }
    }

    // Finally, seek back to the beginning for future tests.
    // Note that this only happens if we support seek.
    TEST_CLEAR_RESULT();
    stream->seek(table->result, stream, ne_core_stream_seek_origin_begin, 0);
    TEST_EXPECT_TABLE_RESULT();
  }

  // Test free (optionally).
  if (stream->free != nullptr && free_stream != NE_CORE_FALSE)
  {
    TEST_CLEAR_RESULT();
    stream->free(table->result, stream);
    TEST_EXPECT_TABLE_RESULT();
  }
}

static void test_full(test_table *table)
{
  TEST_CLEAR_RESULT();
  table->full_tests(table);
  TEST_CLEAR_RESULT();
  table->shared_tests(table);
}

static void test_null(test_table *table)
{
  TEST_CLEAR_RESULT();
  table->null_tests(table);
  TEST_CLEAR_RESULT();
  table->shared_tests(table);
}

static void test_query_permission_granted_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_table *>(const_cast<void *>(user_data));
  TEST_EXPECT(event->permission == table->permission);
  TEST_EXPECT(event->current_state == ne_core_permission_state_granted);
  TEST_EXPECT(event->previous_state == ne_core_permission_state_prompt);

  // We got permission so run both full and shared tests.
  TEST_CLEAR_RESULT();
  table->expected_result = NE_CORE_RESULT_SUCCESS;
  test_full(table);
}

static void test_query_permission_denied_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_table *>(const_cast<void *>(user_data));
  TEST_EXPECT(event->permission == table->permission);
  TEST_EXPECT(event->current_state == ne_core_permission_state_denied);
  TEST_EXPECT(event->previous_state == ne_core_permission_state_prompt);

  // We don't have permission, so check that all tests return null/0 as well
  // as the permission denied result.
  TEST_CLEAR_RESULT();
  table->expected_result = NE_CORE_RESULT_PERMISSION_DENIED;
  test_null(table);
}

static void test_request_permission_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_table *>(const_cast<void *>(user_data));
  TEST_EXPECT(event->permission == table->permission);
  TEST_EXPECT(event->current_state != ne_core_permission_state_prompt);
  TEST_EXPECT(event->previous_state == ne_core_permission_state_prompt);

  if (event->current_state == ne_core_permission_state_granted)
  {
    // We SHOULD have permission.
    TEST_CLEAR_RESULT();
    ne_core_query_permission(table->result,
                             table->permission,
                             &test_query_permission_granted_callback,
                             table);
    TEST_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
  }
  else
  {
    TEST_EXPECT(event->current_state == ne_core_permission_state_denied);

    // We SHOULD NOT have permission.
    TEST_CLEAR_RESULT();
    ne_core_query_permission(table->result,
                             table->permission,
                             &test_query_permission_denied_callback,
                             table);
    TEST_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
  }
}

static void test_query_permission_prompt_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_table *>(const_cast<void *>(user_data));
  TEST_EXPECT(event->permission == table->permission);
  TEST_EXPECT(event->current_state == ne_core_permission_state_prompt);
  TEST_EXPECT(event->previous_state == ne_core_permission_state_prompt);
}

static void test_query_permission_invalid_callback(
    const ne_core_permission_event *event, const void *user_data)
{
  auto table = static_cast<test_table *>(const_cast<void *>(user_data));
  TEST_EXPECT(event->permission == table->permission);
  TEST_EXPECT(event->current_state == ne_core_permission_state_invalid);
  TEST_EXPECT(event->previous_state == ne_core_permission_state_invalid);
}

static void test_all(test_table *table)
{
  TEST_CLEAR_RESULT();
  ne_core_bool supported = table->supported(table->result);
  TEST_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);

  if (supported != NE_CORE_FALSE)
  {
    if (table->permission != NE_CORE_PERMISSION_INVALID)
    {
      // To test permissions, the core must be supported.
      TEST_EXPECT(ne_core_supported(nullptr) == NE_CORE_TRUE);

      // Permissions should currently be 'prompt'.
      TEST_CLEAR_RESULT();
      ne_core_query_permission(table->result,
                               table->permission,
                               &test_query_permission_prompt_callback,
                               table);
      TEST_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);

      // Request permission and wait for the granted/denied callback.
      TEST_CLEAR_RESULT();
      ne_core_request_permission(table->result,
                                 &table->permission,
                                 1,
                                 nullptr,
                                 &test_request_permission_callback,
                                 table);
      TEST_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
    }
    else
    {
      // No permission check existed so we run both full and shared tests.
      TEST_CLEAR_RESULT();
      table->expected_result = NE_CORE_RESULT_SUCCESS;
      test_full(table);
    }
  }
  else
  {
    if (table->permission != NE_CORE_PERMISSION_INVALID)
    {
      // To test permissions, the core must be supported.
      TEST_EXPECT(ne_core_supported(nullptr) == NE_CORE_TRUE);

      TEST_CLEAR_RESULT();
      ne_core_query_permission(table->result,
                               table->permission,
                               &test_query_permission_invalid_callback,
                               table);
      TEST_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);

      TEST_CLEAR_RESULT();
      ne_core_request_permission(table->result,
                                 &table->permission,
                                 1,
                                 nullptr,
                                 &test_query_permission_invalid_callback,
                                 table);
      TEST_EXPECT_RESULT(NE_CORE_RESULT_SUCCESS);
    }

    TEST_CLEAR_RESULT();
    table->expected_result = NE_CORE_RESULT_NOT_SUPPORTED;
    test_null(table);
  }
}

static void test_on_exit(const ne_core_exit_event *event, const void *user_data)
{
  (void)event;
  auto table = static_cast<test_table *>(const_cast<void *>(user_data));
  TEST_CLEAR_RESULT();
  table->exit_tests(table);
}

void test_run(test_table *table)
{
  // We register exit callbacks here since the callbacks are executed in LIFO
  // order and we want the library to have a change to register it's own exit
  // callbacks that will be called before the test exit.
  ne_core_on_exit(nullptr, &test_on_exit, table);

  table->success = NE_CORE_TRUE;
  table->is_final_run = NE_CORE_FALSE;
  uint64_t result = NE_CORE_RESULT_INVALID;
  table->result = &result;

  // Test everything with a valid result pointer.
  test_all(table);

  // Test everything with a null result pointer.
  table->is_final_run = NE_CORE_TRUE;
  table->result = nullptr;
  test_all(table);

  // Because of events, the test may continue well past this point so we can't
  // return a success or failure here.
}

int32_t ne_core_main(int32_t argc, char *argv[])
{
  auto simulated_environment = static_cast<ne_core_bool>(
      argc >= 2 && test_string_compare(argv[1], "--simulated") == 0);

  // Invalid, but until we get a way for modules to run their own initialize and
  // register tests (dependency upon test) then  we must do this.
  extern void test_core(ne_core_bool simulated_environment);
  extern void test_io(ne_core_bool simulated_environment);
  extern void test_time(ne_core_bool simulated_environment);
  extern void test_filesystem(ne_core_bool simulated_environment);
  test_core(simulated_environment);
  test_io(simulated_environment);
  test_time(simulated_environment);
  test_filesystem(simulated_environment);

  // We can't know whether the test completed by this point do to callbacks.
  return 0;
}
