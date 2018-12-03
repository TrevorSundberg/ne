// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../test_timer/test_timer.h"
NE_CORE_BEGIN

NE_CORE_DEFINE_PACKAGE(test_timer);

static void ne_timer_full_tests(ne_core_bool *is_success_out, uint64_t *result,
                                uint64_t expected_result, void *user_data) {
  TEST_IGNORE_UNUSED_PARAMETERS();

  TEST_CLEAR_RESULT();
  uint64_t timer_count = ne_timer_count(result);
  TEST_EXPECT("ne_timer_count", timer_count >= 1);
  TEST_EXPECT_RESULT("ne_timer_count", expected_result);

  // Validate invalid timer values.
  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_timer_frequency",
              ne_timer_frequency(result, timer_count) == 0);
  TEST_EXPECT_RESULT("ne_timer_frequency", NE_CORE_RESULT_INVALID_PARAMETER);

  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_timer_frequency",
              ne_timer_frequency(result, 0xFFFFFFFFFFFFFFFF) == 0);
  TEST_EXPECT_RESULT("ne_timer_frequency", NE_CORE_RESULT_INVALID_PARAMETER);

  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_timer_ticks", ne_timer_ticks(result, timer_count) == 0);
  TEST_EXPECT_RESULT("ne_timer_ticks", NE_CORE_RESULT_INVALID_PARAMETER);

  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_timer_ticks",
              ne_timer_ticks(result, 0xFFFFFFFFFFFFFFFF) == 0);
  TEST_EXPECT_RESULT("ne_timer_ticks", NE_CORE_RESULT_INVALID_PARAMETER);

  uint64_t last_frequency = 0xFFFFFFFFFFFFFFFF;

  // Check that each timer actually increments and has a valid frequency.
  for (uint32_t timer = 0; timer < timer_count; ++timer) {
    TEST_CLEAR_RESULT();
    uint64_t frequency = ne_timer_frequency(result, timer);
    TEST_EXPECT("ne_timer_frequency", frequency != 0);
    TEST_EXPECT_RESULT("ne_timer_frequency", expected_result);

    // Verify that the timers are ordered from highest to lowest frequency.
    TEST_EXPECT("ne_timer_frequency", frequency <= last_frequency);
    last_frequency = frequency;

    TEST_CLEAR_RESULT();
    uint64_t initial_ticks = ne_timer_ticks(result, timer);
    TEST_EXPECT_RESULT("ne_timer_ticks", expected_result);

    // Verify that it increments, otherwise it will freeze here.
    TEST_CLEAR_RESULT();
    while (ne_timer_ticks(result, timer) <= initial_ticks) {
      TEST_EXPECT_RESULT("ne_timer_ticks", expected_result);
    }
  }
}

static void ne_timer_null_tests(ne_core_bool *is_success_out, uint64_t *result,
                                uint64_t expected_result, void *user_data) {
  TEST_IGNORE_UNUSED_PARAMETERS();

  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_timer_count", ne_timer_count(result) == 0);
  TEST_EXPECT_RESULT("ne_timer_count", expected_result);

  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_timer_frequency", ne_timer_frequency(result, 0) == 0);
  TEST_EXPECT_RESULT("ne_timer_frequency", NE_CORE_RESULT_INVALID_PARAMETER);

  TEST_CLEAR_RESULT();
  TEST_EXPECT("ne_timer_ticks", ne_timer_ticks(result, 0) == 0);
  TEST_EXPECT_RESULT("ne_timer_ticks", NE_CORE_RESULT_INVALID_PARAMETER);
}

static void ne_timer_shared_tests(ne_core_bool *is_success_out,
                                  uint64_t *result, uint64_t expected_result,
                                  void *user_data) {
  TEST_IGNORE_UNUSED_PARAMETERS();
}

ne_core_bool test_timer() { TEST_RUN(ne_timer); }

NE_CORE_END
