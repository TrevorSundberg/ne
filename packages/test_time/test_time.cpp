/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_time/test_time.h"

static void full_tests(test_table *table)
{
  // We know it's not 1970...
  TEST_CLEAR_RESULT();
  uint64_t initial_system_time = ne_time_system(table->result);
  TEST_EXPECT(initial_system_time > 0);
  TEST_EXPECT_TABLE_RESULT();

  // Ensure the system clock goes up. Technically this clock is allowed to
  // reverse, but we won't be adjusting the system clock on our test machines.
  TEST_CLEAR_RESULT();
  while (ne_time_system(table->result) <= initial_system_time)
  {
    TEST_EXPECT_TABLE_RESULT();
  }

  // Our highest resolution clock may have started at 0, but we know a tick
  // passed on our system time, so we expect that our highest resolution clock
  // has also changed.
  TEST_CLEAR_RESULT();
  uint64_t initial_high_time = ne_time_high_frequency_monotonic(table->result);
  TEST_EXPECT(initial_high_time > 0);
  TEST_EXPECT_TABLE_RESULT();

  // Ensure the monotonic clock goes up.
  TEST_CLEAR_RESULT();
  while (ne_time_high_frequency_monotonic(table->result) <= initial_high_time)
  {
    TEST_EXPECT_TABLE_RESULT();
  }
}

static void null_tests(test_table *table)
{
  TEST_CLEAR_RESULT();
  TEST_EXPECT(ne_time_system(table->result) == 0);
  TEST_EXPECT_TABLE_RESULT();

  TEST_CLEAR_RESULT();
  TEST_EXPECT(ne_time_high_frequency_monotonic(table->result) == 0);
  TEST_EXPECT_TABLE_RESULT();
}

static void shared_tests(test_table *table) { (void)table; }

static void exit_tests(test_table *table) { (void)table; }

void test_time(ne_core_bool simulated_environment)
{
  TEST_RUN(ne_time_supported, NE_CORE_PERMISSION_INVALID);
}
