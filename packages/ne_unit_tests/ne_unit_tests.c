#include <stdint.h>
#include "ne.h"

void validate(ne_bool value, const char* message)
{
  if (!value)
  {
    ne_host_error(message);
    ne_host_break();
  }
}

uint64_t strlen(const char* string)
{
  const char* iterator = string;
  while (*iterator++ != '\0');

  return iterator - string;
}

ne_bool is_non_empty_string(const char* string)
{
  if (string == ne_null)
    return ne_false;

  return strlen(string) != 0;
}

uint64_t global = 1234;

void ne_unit_tests()
{
  validate(ne_empty_string() != ne_null && *ne_empty_string() == '\0',
    "Calling 'ne_empty_string' should return valid string that contains only a null terminator.");

  validate(ne_empty_string() == ne_empty_string(),
    "Calling 'ne_empty_string' should always return the same value.");

  validate(ne_result_get() == ne_result_success,
    "The first call to 'ne_result_get' should return 'ne_result_success'.");

  ne_result_set(ne_result_invalid_parameter);
  validate(ne_result_get() == ne_result_invalid_parameter,
    "The call to 'ne_result_get' should return 'ne_result_invalid_parameter'.");

  // Validate that all error messages contain text
  for (ne_result i = 0; i < ne_result_count; ++i)
  {
    validate(is_non_empty_string(ne_result_to_text(ne_result_success)),
      "Expected a non-empty string out of 'ne_result_to_text'.");
  }

  validate(ne_result_to_text(ne_result_count) == ne_empty_string(),
    "Expected 'ne_empty_string' from 'ne_result_to_text' when an invalid value is passed in.");

  validate(ne_result_to_text((uint32_t)-1) == ne_empty_string(),
    "Expected 'ne_empty_string' from 'ne_result_to_text' when an invalid value is passed in.");

  validate(ne_version_major() == ne_version_header_major,
    "Expected 'ne_version_major' to match 'ne_version_header_major'");

  validate(ne_version_minor() >= ne_version_header_minor,
    "Expected 'ne_version_minor' to be the same or greater than 'ne_version_header_minor'");

  validate(is_non_empty_string(ne_version_platform()),
    "Expected a non-empty string out of 'ne_version_platform'.");

  validate(is_non_empty_string(ne_version_host()),
    "Expected a non-empty string out of 'ne_version_host'.");

  void* allocated = ne_memory_allocate(1);
  validate(allocated != ne_null,
    "Expected an allocation of size 1 to succeed in 'ne_memory_allocate'.");

  validate(allocated != ne_null,
    "Expected an allocation of size 1 to succeed in 'ne_memory_allocate'.");

  validate(ne_memory_query(allocated) == ne_memory_access_readwrite,
    "Expected 'ne_memory_query' to return that the allocated memory supports read/write.");

  // Technically this unit test could be flawed due to however the allocator puts memory together...
  // Since we've only allocated once then the only way this could be invalid is if it hit the static
  // section of memory.
  validate(ne_memory_query((uint8_t*)allocated + 1) == ne_memory_access_invalid,
    "Expected 'ne_memory_query' to return that the offset memory was invalid.");
  validate(ne_memory_query((uint8_t*)allocated - 1) == ne_memory_access_invalid,
    "Expected 'ne_memory_query' to return that the offset memory was invalid.");

  ne_memory_free(allocated);
  validate(ne_memory_query(allocated) == ne_memory_access_invalid,
    "Expected 'ne_memory_query' to return that the deallocatged memory was invalid.");

  validate(ne_memory_query(ne_null) == ne_memory_access_invalid,
    "Expected 'ne_memory_query' to return that 'ne_null' was invalid.");

  validate(ne_memory_query(&global) == ne_memory_access_readwrite,
    "Expected 'ne_memory_query' to return that a global variable address supports read/write.");

  uint64_t local = 1234;
  validate(ne_memory_query(&local) == ne_memory_access_readwrite,
    "Expected 'ne_memory_query' to return that a local variable address supports read/write.");

  const char* string = "1234";
  validate(ne_memory_query(string) == ne_memory_access_read,
    "Expected 'ne_memory_query' to return that a string constant address is read only.");

  validate(ne_timer_count() >= 1,
    "Expected 'ne_timer_count' to return at least one timer.");
  
  for (uint32_t timer = 0; timer < ne_timer_count(); ++timer)
  {
    validate(ne_timer_frequency(timer) != 0,
      "Expected 'ne_timer_frequency' to return a non-zero frequency.");

    uint64_t initial = ne_timer_ticks(timer);

    // Verify that it increments, otherwise it will freeze here
    while (ne_timer_ticks(timer) <= initial);
  }

  ne_file* input = ne_io_get_input();
  validate(input != ne_null,
    "Expected 'ne_io_get_input' to return non null file.");

  ne_file* output = ne_io_get_output();
  validate(output != ne_null,
    "Expected 'ne_io_get_output' to return a valid file.");

  uint8_t buffer[32];

  // Call these for coverage (can't verify them).
  ne_file_read(input, &buffer, sizeof(buffer));
  ne_file_write(output, &buffer, sizeof(buffer));

  // Check for special error codes

}
