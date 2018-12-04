// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_core/ne_core.h"
#include <cstdlib>
#include <iostream>

#if defined(NE_CORE_PLATFORM_WINDOWS)
#define NE_CORE_SUPPORTED
#endif

/******************************************************************************/
static ne_core_event_callback _event_callback;
static const void *_event_user_data;

/******************************************************************************/
static ne_core_bool _ne_core_supported(uint64_t *result)
{
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
#if defined(NE_CORE_SUPPORTED)
  return NE_CORE_TRUE;
#else
  return NE_CORE_FALSE;
#endif
}
ne_core_bool (*ne_core_supported)(uint64_t *result) = &_ne_core_supported;

/******************************************************************************/
static void _ne_core_hello_world(uint64_t *result)
{
  std::cout << "Hello world!" << std::endl;
  fflush(stdout);
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_hello_world)(uint64_t *result) = &_ne_core_hello_world;

/******************************************************************************/
[[noreturn]] static void _ne_core_exit(uint64_t *result, int32_t return_code)
{
  exit(return_code);
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_exit)(uint64_t *result, int32_t return_code) = &_ne_core_exit;

/******************************************************************************/
static void _ne_core_error(uint64_t *result,
                           const char *file,
                           int64_t line,
                           const char *message)
{
  std::cerr << std::endl
            << file << "(" << line << "): " << message << std::endl;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_error)(uint64_t *result,
                      const char *file,
                      int64_t line,
                      const char *message) = &_ne_core_error;

/******************************************************************************/
static uint8_t *_ne_core_allocate(uint64_t *result, uint64_t sizeBytes)
{
  auto *memory = static_cast<uint8_t *>(malloc(static_cast<size_t>(sizeBytes)));
  NE_CORE_RESULT(memory ? NE_CORE_RESULT_SUCCESS
                        : NE_CORE_RESULT_ALLOCATION_FAILED);
  return memory;
}
uint8_t *(*ne_core_allocate)(uint64_t *result,
                             uint64_t sizeBytes) = &_ne_core_allocate;

/******************************************************************************/
static void _ne_core_free(uint64_t *result, void *memory)
{
  free(memory);
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_free)(uint64_t *result, void *memory) = &_ne_core_free;

/******************************************************************************/
static void _ne_core_set_event_callback(uint64_t *result,
                                        ne_core_event_callback callback,
                                        const void *user_data)
{
  _event_callback = callback;
  _event_user_data = user_data;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_set_event_callback)(uint64_t *result,
                                   ne_core_event_callback callback,
                                   const void *user_data) =
    &_ne_core_set_event_callback;

/******************************************************************************/
static void _ne_core_get_event_callback(uint64_t *result,
                                        ne_core_event_callback *callback,
                                        const void **user_data)
{
  *callback = _event_callback;
  *user_data = _event_user_data;
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_get_event_callback)(uint64_t *result,
                                   ne_core_event_callback *callback,
                                   const void **user_data) =
    &_ne_core_get_event_callback;

/******************************************************************************/
int32_t main(int32_t argc, char *argv[])
{
  int32_t result = ne_core_main(argc, argv);

  while (_event_callback != nullptr)
  {
    _event_callback(nullptr, _event_user_data);
  }

  return result;
}
