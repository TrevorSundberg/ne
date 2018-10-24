// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_core/ne_core.h"
#include <stdio.h>
#include <stdlib.h>
NE_CORE_BEGIN

NE_CORE_DEFINE_PACKAGE(ne_core);

/******************************************************************************/
static void _ne_core_hello_world(uint64_t *result) {
  printf("Hello world!\n");
  NE_CORE_RESULT(ne_core_result_success);
}
void (*ne_core_hello_world)(uint64_t *result) = &_ne_core_hello_world;

/******************************************************************************/
static void _ne_core_main_loop(uint64_t *result,
                               ne_core_main_loop_callback callback,
                               void *user_data) {

  static ne_core_bool called = 0;
  if (called) {
    NE_CORE_RESULT(ne_core_result_error);
    return;
  }
  called = NE_CORE_TRUE;

  NE_CORE_RESULT(ne_core_result_success);

  for (;;)
    callback(user_data);
}
void (*ne_core_main_loop)(uint64_t *result, ne_core_main_loop_callback callback,
                          void *user_data) = &_ne_core_main_loop;

/******************************************************************************/
static void _ne_core_exit(uint64_t *result, int32_t return_code) {
  exit(return_code);
  NE_CORE_RESULT(ne_core_result_success);
}
void (*ne_core_exit)(uint64_t *result, int32_t return_code) = &_ne_core_exit;

/******************************************************************************/
static void _ne_core_error(uint64_t *result, const char *message) {
  fputs(message, stderr);
  NE_CORE_RESULT(ne_core_result_success);
}
void (*ne_core_error)(uint64_t *result, const char *message) = &_ne_core_error;

/******************************************************************************/
static uint8_t *_ne_core_allocate(uint64_t *result, uint64_t sizeBytes) {
  uint8_t *memory = (uint8_t *)malloc((size_t)sizeBytes);
  NE_CORE_RESULT(memory ? ne_core_result_success
                        : ne_core_result_allocation_failed);
  return memory;
}
uint8_t *(*ne_core_allocate)(uint64_t *result,
                             uint64_t sizeBytes) = &_ne_core_allocate;

/******************************************************************************/
static void _ne_core_free(uint64_t *result, void *memory) {
  free(memory);
  NE_CORE_RESULT(ne_core_result_success);
}
void (*ne_core_free)(uint64_t *result, void *memory) = &_ne_core_free;

/******************************************************************************/
int32_t main(int32_t argc, char *argv[]) {
  return ne_core_main((int32_t)argc, argv);
}

NE_CORE_END
