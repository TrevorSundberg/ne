// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_filesystem/test_filesystem.h"
NE_BEGIN

NE_DEFINE_PACKAGE(test_filesystem);

// Test writing to a file multiple times.
// Test appending to a file multiple times.
// Verify both outputs above by reading the file back.
// Verify shares by opening multiple files at once and doing operations.
// Verify create always/if_none/never.

static void test_translate(const char *ne_path, TEST_DECLARE_PARAMETERS) {
  TEST_IGNORE_UNUSED_PARAMETERS();

  TEST_CLEAR_RESULT();
  const char *os_translated = ne_filesystem_translate_ne_to_os(result, ne_path);
  TEST_EXPECT("ne_filesystem_translate_ne_to_os",
              os_translated != ne_core_null);
  TEST_EXPECT_RESULT("ne_filesystem_translate_ne_to_os", expected_result);

  TEST_CLEAR_RESULT();
  const char *ne_translated =
      ne_filesystem_translate_os_to_ne(result, os_translated);
  TEST_EXPECT("ne_filesystem_translate_os_to_ne",
              ne_translated != ne_core_null);
  TEST_EXPECT("ne_filesystem_translate_os_to_ne",
              test_string_compare(ne_translated, ne_path) == 0);
  TEST_EXPECT_RESULT("ne_filesystem_translate_os_to_ne", expected_result);
}

static void test_open_file(const char *path, ne_filesystem_io io,
                           ne_filesystem_create create,
                           ne_filesystem_share_flags share,
                           ne_core_stream *stream_out,
                           TEST_DECLARE_PARAMETERS) {
  test_memory_set(stream_out, 1, sizeof(*stream_out));
  TEST_CLEAR_RESULT();
  ne_filesystem_open_file(result, path, io, ne_filesystem_create_if_none, 0,
                          stream_out);
  TEST_EXPECT_RESULT("ne_filesystem_open_file", expected_result);
}

static void test_write(ne_core_stream *stream_out, TEST_DECLARE_PARAMETERS) {
  test_memory_set(stream_out, 1, sizeof(*stream_out));
  TEST_CLEAR_RESULT();
  ne_filesystem_open_file(result, path, io, ne_filesystem_create_if_none, 0,
                          stream_out);
  TEST_EXPECT_RESULT("ne_filesystem_open_file", expected_result);

  TEST_EXPECT("read", stream_out->read == ne_core_null);

  TEST_CLEAR_RESULT();
  stream_out->write(result, stream_out, buffer, sizeof(buffer), ne_core_false);
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

static void ne_filesystem_full_tests(ne_core_bool *is_success_out,
                                     uint64_t *result, uint64_t expected_result,
                                     void *user_data) {
  TEST_IGNORE_UNUSED_PARAMETERS();

  ne_filesystem_translate_test("/", is_success_out, result, expected_result,
                               user_data);
}

static void ne_filesystem_null_tests(ne_core_bool *is_success_out,
                                     uint64_t *result, uint64_t expected_result,
                                     void *user_data) {
  TEST_IGNORE_UNUSED_PARAMETERS();
}

static void ne_filesystem_shared_tests(ne_core_bool *is_success_out,
                                       uint64_t *result,
                                       uint64_t expected_result,
                                       void *user_data) {
  TEST_IGNORE_UNUSED_PARAMETERS();
}

ne_core_bool test_filesystem() { TEST_RUN(ne_filesystem); }

NE_END
