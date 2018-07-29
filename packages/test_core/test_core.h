// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(test_core, 0, 0);

ne_core_bool test_validate(ne_core_bool value, const char* message);

uint64_t test_string_length(const char* string);

void test_memory_set(void* memory, uint8_t value, uint64_t size);

int64_t test_memory_compare(void* a, void* b, uint64_t size);

int64_t test_memory_compare_value(void* memory, uint8_t value, uint64_t size);

struct test_table
{
  ne_core_bool (*supported)(uint64_t* result);
  ne_core_bool (*request_permission)(uint64_t* result);
  uint64_t (*version_linked_major)(uint64_t* result);
  uint64_t (*version_linked_minor)(uint64_t* result);
  uint64_t version_header_major;
  uint64_t version_header_minor;
  
  // This is called when we know the API is both supported and we have permission.
  void (*full_tests)(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data);

  // These are tests we run if the API is not supported or permission is denied.
  // All calls are expected to return zero/null, and output the expected result.
  void(*null_tests)(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data);

  // These tests don't rely on the return value and therefore can be called during both full/null phases.
  void(*shared_tests)(ne_core_bool* is_success_out, uint64_t* result, uint64_t expected_result, void* user_data);

  void* user_data;
};
typedef struct test_table test_table;

ne_core_bool test_run(test_table* functions);

ne_core_bool test_core(void);

#define TEST_RUN(name)\
  test_table table;\
  table.user_data = ne_core_null;\
  table.full_tests = &name##_full_tests;\
  table.null_tests = &name##_null_tests;\
  table.shared_tests = &name##_shared_tests;\
  table.supported = name##_supported;\
  table.request_permission = name##_request_permission;\
  table.version_linked_major = name##_version_linked_major;\
  table.version_linked_minor = name##_version_linked_minor;\
  table.version_header_major = name##_version_header_major;\
  table.version_header_minor = name##_version_header_minor;\
  return test_run(&table)

#define TEST_EXPECT(name, expression)\
  (*is_success_out &= test_validate((expression), "In '" name "' expected '" #expression "' to be true."))

#define TEST_SAFE_ENCLOSURE(code)\
  do { code } while (ne_core_true)

#define TEST_CLEAR_RESULT()\
  TEST_SAFE_ENCLOSURE(if (result) *result = ne_core_result_not_set;)

#define TEST_EXPECT_RESULT(name, expected_result)\
  TEST_SAFE_ENCLOSURE(if (result) *is_success_out &= test_validate(*result == expected_result, "In '" name "' expected result '" #expected_result "'.");)

#define TEST_IGNORE_UNUSED_PARAMETERS()\
  (void)is_success_out;\
  (void)result;\
  (void)expected_result;\
  (void)user_data

NE_END
