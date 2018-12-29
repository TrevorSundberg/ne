/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_filesystem/test_filesystem.h"
/*
// Test writing to a file multiple times.
// Test appending to a file multiple times.
// Verify both outputs above by reading the file back.
// Verify shares by opening multiple files at once and doing operations.
// Verify create always/if_none/never.
*/

static char get_last_char(const char *string)
{
  uint64_t length = test_string_length(string);
  if (length != 0)
  {
    return string[length - 1];
  }

  return '\0';
}

static bool validate_any_path(const char *path)
{
  if (path == nullptr || *path == '\0')
  {
    return false;
  }

  // This also helps to validate that the string has a null terminator and
  // doesn't run off the end of memory.
  char last = get_last_char(path);
  return last != '/' && last != '\\';
}

static bool validate_universal_absolute_path(const char *path)
{
  return validate_any_path(path) && path[0] == '/';
}

static bool validate_universal_canonical_path(const char *path)
{
  if (!validate_universal_absolute_path(path))
  {
    return false;
  }

  int32_t state = 0;

  for (;;)
  {
    char c = *path;

    if (c == '/' || c == '\0')
    {
      if (state == 2 || state == 3)
      {
        // We found a '/./' or '/../'.
        return false;
      }

      state = 1;
    }
    else if (c == '.')
    {
      if (state == 1 || state == 2)
      {
        ++state;
      }
    }
    else
    {
      state = 0;
    }

    if (c == '\0')
    {
      return true;
    }

    ++path;
  }
}

// Validate path
// Validate absolute path
// Validate canonical path

static void test_translate_paths(test_table *table,
                                 const char *universal,
                                 const char *os)
{
  TEST_CLEAR_RESULT();
  char *test_os =
      ne_filesystem_translate_universal_to_os(table->result, universal);
  TEST_EXPECT_TABLE_RESULT();
  TEST_EXPECT(test_string_compare(test_os, os) == 0);
  TEST_EXPECT(validate_any_path(test_os));

  TEST_CLEAR_RESULT();
  ne_core_free(table->result, test_os);
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  char *test_universal =
      ne_filesystem_translate_os_to_universal(table->result, os);
  TEST_EXPECT_TABLE_RESULT();
  TEST_EXPECT(test_string_compare(test_universal, universal) == 0);
  TEST_EXPECT(validate_universal_canonical_path(test_universal));

  TEST_CLEAR_RESULT();
  ne_core_free(table->result, test_universal);
  TEST_EXPECT_TABLE_RESULT();
}

static void full_tests(test_table *table)
{
  // TODO(Trevor.Sundberg) We should test relative paths, but we would need to
  // control the working directory. Most likely we'd set POSIX systems to '/'
  // and Windows to the system drive, like 'C:'.

  TEST_CLEAR_RESULT();
  const char *scheme = ne_filesystem_get_scheme(table->result);
  TEST_EXPECT(scheme != nullptr && *scheme != '\0');
  // Read the 8 bytes to help ensure it's readable memory.
  TEST_EXPECT(ne_core_memory_compare(scheme, scheme, sizeof(uint64_t)) == 0);
  // Make sure the string is a reasonable length and doesn't run off the end
  // without a null terminator.
  TEST_EXPECT(test_string_length(scheme) < static_cast<uint16_t>(-1));
  TEST_EXPECT_TABLE_RESULT();

  uint64_t scheme_id = *reinterpret_cast<const uint64_t *>(scheme);

  // All other platforms except Windows use the same path scheme.
  if (scheme_id ==
      *reinterpret_cast<const uint64_t *>(NE_FILESYSTEM_SCHEME_WINDOWS))
  {
    // Absolute paths.
    test_translate_paths(table, R"(/X:/test1/test2)", R"(X:\test1\test2)");
    // Network paths.
    test_translate_paths(table, R"(/\\test1/test2)", R"(\\test1\test2)");
    // UNC paths.
    test_translate_paths(table, R"(/\\?/test1/test2)", R"(\\?\test1\test2)");
    // Device paths.
    test_translate_paths(table, R"(/\\./test1/test2)", R"(\\.\test1\test2)");
    // Drive paths.
    test_translate_paths(table, R"(/C:)", R"(C:)");
  }
  else
  {
    // Absolute paths.
    test_translate_paths(table, R"(/test1/test2)", R"(/test1/test2)");
  }

  for (int32_t i = 0; i != ne_filesystem_special_path_max; ++i)
  {
    TEST_CLEAR_RESULT();
    char *universal_path = ne_filesystem_get_special_path(
        table->result, static_cast<ne_filesystem_special_path>(i));
    TEST_EXPECT(validate_universal_canonical_path(universal_path));
    TEST_EXPECT_TABLE_RESULT();

    TEST_CLEAR_RESULT();
    ne_core_free(table->result, universal_path);
    TEST_EXPECT_TABLE_RESULT();
  }

  TEST_CLEAR_RESULT();
  char *directory = ne_filesystem_get_special_path(
      table->result, ne_filesystem_special_path_directory_temporary);
  TEST_EXPECT(validate_universal_canonical_path(directory));
  TEST_EXPECT_TABLE_RESULT();

  // TODO(Trevor.Sundberg): Must check directory existance or create it.

  char *path = test_concatenate_allocate(directory, "/test.txt");
  TEST_EXPECT(validate_universal_canonical_path(path));

  TEST_CLEAR_RESULT();
  ne_core_free(table->result, directory);
  TEST_EXPECT_TABLE_RESULT();

  ne_filesystem_open_info info;
  ne_core_memory_set(&info, NE_CORE_UNINITIALIZED_BYTE, sizeof(info));

  info.universal_path = path;
  info.io = ne_filesystem_io_write;
  info.if_file_exists = ne_filesystem_if_file_exists_truncate;
  info.if_none_exists = ne_filesystem_if_none_exists_create;
  info.share_flags = ne_filesystem_share_flags_none;
  info.bypass_cache = NE_CORE_FALSE;

  ne_core_stream stream;
  ne_core_memory_set(&stream, NE_CORE_UNINITIALIZED_BYTE, sizeof(stream));
  TEST_CLEAR_RESULT();
  ne_filesystem_open_file(table->result, &info, &stream);
  TEST_EXPECT_TABLE_RESULT();

  TEST_EXPECT(stream.read == NE_CORE_NULL);
  TEST_EXPECT(stream.write != NE_CORE_NULL);
  TEST_EXPECT(stream.flush != NE_CORE_NULL);
  TEST_EXPECT(stream.get_position != NE_CORE_NULL);
  TEST_EXPECT(stream.get_size != NE_CORE_NULL);
  TEST_EXPECT(stream.seek != NE_CORE_NULL);
  TEST_EXPECT(stream.is_valid != NE_CORE_NULL);
  TEST_EXPECT(stream.free != NE_CORE_NULL);

  test_stream(table, &stream, NE_CORE_TRUE);

  TEST_CLEAR_RESULT();
  ne_core_free(table->result, path);
  TEST_EXPECT_TABLE_RESULT();
}

static void null_tests(test_table *table)
{
  TEST_CLEAR_RESULT();
  TEST_EXPECT(ne_filesystem_translate_universal_to_os(table->result, "") ==
              nullptr);
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  TEST_EXPECT(ne_filesystem_translate_os_to_universal(table->result, "") ==
              nullptr);
  TEST_EXPECT_TABLE_RESULT();

  for (int32_t i = 0; i != ne_filesystem_special_path_max; ++i)
  {
    TEST_CLEAR_RESULT();
    TEST_EXPECT(ne_filesystem_get_special_path(
                    table->result,
                    static_cast<ne_filesystem_special_path>(i)) == nullptr);
    TEST_EXPECT_TABLE_RESULT();
  }
}

static void shared_tests(test_table *table) { (void)table; }

static void exit_tests(test_table *table) { (void)table; }

void test_filesystem(ne_core_bool simulated_environment)
{
  TEST_RUN(ne_filesystem_supported, NE_CORE_PERMISSION_INVALID);
}
