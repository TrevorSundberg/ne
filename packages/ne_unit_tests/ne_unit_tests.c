#include <stdint.h>
#include "../../include/ne.h"

static void validate(ne_bool value, const char* message)
{
  if (!value)
    ne_sandbox_error(ne_null, message);
}

static uint64_t strlen(const char* string)
{
  const char* iterator = string;
  while (*iterator++ != '\0');

  return iterator - string;
}

static ne_bool is_non_empty_string(const char* string)
{
  if (string == ne_null)
    return ne_false;

  return strlen(string) != 0;
}

uint64_t global = 1234;

void ne_unit_tests()
{
  ne_result result = ne_result_core_not_set;

  validate(ne_empty_string != ne_null && *ne_empty_string == '\0',
    "Calling 'ne_empty_string' should return valid string that contains only a null terminator.");

  // For some function calls we don't need to check the result because we
  // check that the sub-api is supported and request permission where possible,
  // and there may be no other possible errors.
  
  // The comment 'Coverage' means we cannot validate the functions, but we can at least call them.

  result = ne_result_core_not_set;
  if (ne_version_supported(&result))
  {
    validate(result == ne_result_core_success,
      "Expected 'ne_version_supported' to set the result to 'ne_result_core_success'.");

    validate(ne_version_api_major(ne_null) == ne_version_header_major,
      "Expected 'ne_version_major' to match 'ne_version_header_major'.");

    validate(ne_version_api_minor(ne_null) >= ne_version_header_minor,
      "Expected 'ne_version_minor' to be the same or greater than 'ne_version_header_minor'.");

    validate(is_non_empty_string(ne_version_sandbox(ne_null)),
      "Expected a non-empty string out of 'ne_version_host'.");

    // Coverage.
    ne_version_sandbox_major(ne_null);
    ne_version_sandbox_minor(ne_null);

    validate(is_non_empty_string(ne_version_platform(ne_null)),
      "Expected a non-empty string out of 'ne_version_platform'.");
  }
  else
  {
    validate(result == ne_result_core_not_supported,
      "Expected 'ne_version_supported' to set the result to 'ne_result_core_not_supported'.");
  }

  result = ne_result_core_not_set;
  if (ne_memory_supported(&result))
  {
    void* allocated = ne_memory_allocate(ne_null, 1);
    validate(allocated != ne_null,
      "Expected an allocation of size 1 to succeed in 'ne_memory_allocate'.");
  }

  //void* allocated = ne_memory_allocate(ne_null, 1);
  //validate(allocated != ne_null,
  //  "Expected an allocation of size 1 to succeed in 'ne_memory_allocate'.");
  //
  //validate(ne_memory_query(allocated) == ne_memory_access_readwrite,
  //  "Expected 'ne_memory_query' to return that the allocated memory supports read/write.");
  //// Technically this unit test could be flawed due to however the allocator puts memory together...
  //// Since we've only allocated once then the only way this could be invalid is if it hit the static
  //// section of memory.
  //validate(ne_memory_query((uint8_t*)allocated + 1) == ne_memory_access_invalid,
  //  "Expected 'ne_memory_query' to return that the offset memory was invalid.");
  //validate(ne_memory_query((uint8_t*)allocated - 1) == ne_memory_access_invalid,
  //  "Expected 'ne_memory_query' to return that the offset memory was invalid.");
  //
  //ne_memory_free(allocated);
  //validate(ne_memory_query(allocated) == ne_memory_access_invalid,
  //  "Expected 'ne_memory_query' to return that the deallocatged memory was invalid.");
  //
  //validate(ne_memory_query(ne_null) == ne_memory_access_invalid,
  //  "Expected 'ne_memory_query' to return that 'ne_null' was invalid.");
  //
  //validate(ne_memory_query(&global) == ne_memory_access_readwrite,
  //  "Expected 'ne_memory_query' to return that a global variable address supports read/write.");
  //
  //uint64_t local = 1234;
  //validate(ne_memory_query(&local) == ne_memory_access_readwrite,
  //  "Expected 'ne_memory_query' to return that a local variable address supports read/write.");
  //
  //const char* string = "1234";
  //validate(ne_memory_query(string) == ne_memory_access_read,
  //  "Expected 'ne_memory_query' to return that a string constant address is read only.");

  result = ne_result_core_not_set;
  if (ne_timer_supported(&result))
  {
    validate(result == ne_result_core_success,
      "Expected 'ne_timer_supported' to set the result to 'ne_result_core_success'.");

    validate(ne_timer_count(ne_null) >= 1,
      "Expected 'ne_timer_count' to return at least one timer.");

    for (uint32_t timer = 0; timer < ne_timer_count(ne_null); ++timer)
    {
      validate(ne_timer_frequency(ne_null, timer) != 0,
        "Expected 'ne_timer_frequency' to return a non-zero frequency.");

      uint64_t initial = ne_timer_ticks(ne_null, timer);

      // Verify that it increments, otherwise it will freeze here
      while (ne_timer_ticks(ne_null, timer) <= initial);
    }
  }
  else
  {
    validate(result == ne_result_core_not_supported,
      "Expected 'ne_timer_supported' to set the result to 'ne_result_core_not_supported'.");
  }


  result = ne_result_core_not_set;
  if (ne_io_supported(&result))
  {
    validate(result == ne_result_core_success,
      "Expected 'ne_io_supported' to set the result to 'ne_result_core_success'.");

    ne_file* input = ne_io_get_input(ne_null);
    validate(input != ne_null,
      "Expected 'ne_io_get_input' to return non null file.");

    ne_file* output = ne_io_get_output(ne_null);
    validate(output != ne_null,
      "Expected 'ne_io_get_output' to return a valid file.");

    if (ne_file_supported(ne_null))
    {
      // We use non-blocking file read/write because we don't want the unit test to hang.
      uint8_t buffer[32] = { 0 };

      result = ne_result_core_not_set;
      validate(ne_file_write(&result, input, &buffer, sizeof(buffer), ne_false) == 0,
        "Expected 'ne_file_write' to 'ne_io_get_input' to return 0 bytes written.");

      validate(result == ne_result_core_invalid_parameter,
        "Expected 'ne_file_write' to 'ne_io_get_input' to set the result to 'ne_result_core_invalid_parameter'.");

      result = ne_result_core_not_set;
      validate(ne_file_read(&result, output, &buffer, sizeof(buffer), ne_false) == 0,
        "Expected 'ne_file_read' to 'ne_io_get_output' to return 0 bytes read.");

      validate(result == ne_result_core_invalid_parameter,
        "Expected 'ne_file_read' to 'ne_io_get_output' to set the result to 'ne_result_core_invalid_parameter'.");

      // Coverage.
      ne_file_read(ne_null, input, &buffer, sizeof(buffer), ne_false);
      ne_file_write(ne_null, output, &buffer, sizeof(buffer), ne_false);
    }
  }
  else
  {
    validate(result == ne_result_core_not_supported,
      "Expected 'ne_io_supported' to set the result to 'ne_result_core_not_supported'");
  }

  // Check for special error codes

}
