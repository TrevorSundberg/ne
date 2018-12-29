/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_core/ne_core.h"
#include "../ne_core/ne_core_private.h"
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#if defined(NE_CORE_PLATFORM_WINDOWS)
static const constexpr bool _supported = true;
#else
static const constexpr bool _supported = false;
#endif

/******************************************************************************/
template <typename Container,
          typename Key = typename Container::key_type,
          typename T = typename Container::mapped_type>
T *get_or_null(Container &container, const Key &key)
{
  auto it = container.find(key);
  if (it != container.end())
  {
    return &it->second;
  }

  return nullptr;
}

/******************************************************************************/
class ne_core_instance
{
public:
  void invoke_permission_callback(uint64_t permission,
                                  const ne_core_permission_event *event,
                                  ne_core_permission_callback callback,
                                  const void *user_data);

  void run_exit_callbacks();

  void next_frame_executors_reserve_extra(uint64_t count);

  std::unordered_map<uint64_t, ne_core_permission_event> permissions;
  std::vector<std::function<void()>> next_frame_executors;
  std::vector<std::function<void()>> exit_callbacks;
};
static ne_core_instance *_instance;

/******************************************************************************/
void ne_core_instance::invoke_permission_callback(
    uint64_t permission,
    const ne_core_permission_event *event,
    ne_core_permission_callback callback,
    const void *user_data)
{
  ne_core_permission_event invalid;
  if (event == nullptr)
  {
    invalid.permission = permission;
    invalid.current_state = ne_core_permission_state_invalid;
    invalid.previous_state = ne_core_permission_state_invalid;
    event = &invalid;
  }

  // Emulate platforms that take time to return permission requests.
  next_frame_executors.emplace_back(
      [callback, user_data, captured_event = *event]() mutable {
        callback(&captured_event, user_data);
      });
}

/******************************************************************************/
void ne_core_instance::run_exit_callbacks()
{
  // Run through all exit callbacks in reverse order (LIFO).
  while (!exit_callbacks.empty())
  {
    exit_callbacks.back()();
    exit_callbacks.pop_back();
  }
}

/******************************************************************************/
void ne_core_instance::next_frame_executors_reserve_extra(uint64_t count)
{
  next_frame_executors.reserve(next_frame_executors.size() +
                               static_cast<size_t>(count));
}

/******************************************************************************/
static ne_core_bool _ne_core_supported(uint64_t *result)
{
  NE_CORE_SUPPORTED_IMPLEMENTATION(_supported);
}
ne_core_bool (*ne_core_supported)(uint64_t *result) = &_ne_core_supported;

/******************************************************************************/
static const char *_ne_core_get_platform_name(uint64_t *result)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, nullptr);

  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  return NE_CORE_PLATFORM_NAME;
}
const char *(*ne_core_get_platform_name)(uint64_t *result) =
    &_ne_core_get_platform_name;

/******************************************************************************/
static const char *_ne_core_get_application_guid(uint64_t *result)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, nullptr);

  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  // TODO(Trevor.Sundberg): Make this actually depdent upon platform and
  // application, right now it's not unique at all!
  return "a9c4709c-14c5-4b36-ad92-0aaa342e15a6";
}
const char *(*ne_core_get_application_guid)(uint64_t *result) =
    &_ne_core_get_application_guid;

/******************************************************************************/
static void _ne_core_on_exit(uint64_t *result,
                             ne_core_exit_callback callback,
                             const void *user_data)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  NE_CORE_TRY
  {
    _instance->exit_callbacks.emplace_back(
        [callback, user_data]() mutable { callback(nullptr, user_data); });
    NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(NE_CORE_NONE)
}
void (*ne_core_on_exit)(uint64_t *result,
                        ne_core_exit_callback callback,
                        const void *user_data) = &_ne_core_on_exit;

/******************************************************************************/
static void _ne_core_query_permission(uint64_t *result,
                                      uint64_t permission,
                                      ne_core_permission_callback callback,
                                      const void *user_data)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  ne_core_permission_event *event =
      get_or_null(_instance->permissions, permission);

  NE_CORE_TRY
  {
    _instance->invoke_permission_callback(
        permission, event, callback, user_data);
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(NE_CORE_NONE)

  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_query_permission)(uint64_t *result,
                                 uint64_t permission,
                                 ne_core_permission_callback callback,
                                 const void *user_data) =
    &_ne_core_query_permission;

/******************************************************************************/
static void _ne_core_request_permission(uint64_t *result,
                                        const uint64_t permissions[],
                                        uint64_t count,
                                        const char *message,
                                        ne_core_permission_callback callback,
                                        const void *user_data)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  // TODO(Trevor.Sundberg): Use the message.
  (void)message;

  NE_CORE_TRY
  {
    // Reserve ahead of time so that invoke_permission_callback cannot fail with
    // a bad allocation.
    _instance->next_frame_executors_reserve_extra(count);
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(NE_CORE_NONE)

  for (uint64_t i = 0; i < count; ++i)
  {
    uint64_t permission = permissions[i];

    ne_core_permission_event *event =
        get_or_null(_instance->permissions, permission);

    if (event != nullptr)
    {
      event->previous_state = event->current_state;

      // TODO(Trevor.Sundberg): Query other packages for permission handling.
      event->current_state = ne_core_permission_state_granted;
    }
    _instance->invoke_permission_callback(
        permission, event, callback, user_data);
  }

  // TODO(Trevor.Sundberg): Handle NE_CORE_RESULT_INVALID_PARAMETER.
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_request_permission)(uint64_t *result,
                                   const uint64_t permissions[],
                                   uint64_t count,
                                   const char *message,
                                   ne_core_permission_callback callback,
                                   const void *user_data) =
    &_ne_core_request_permission;

/******************************************************************************/
static void _ne_core_hello_world(uint64_t *result)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  std::cout << "Hello world!" << std::endl;
  fflush(stdout);
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_hello_world)(uint64_t *result) = &_ne_core_hello_world;

/******************************************************************************/
static void _ne_core_request_frame(uint64_t *result,
                                   ne_core_frame_callback callback,
                                   const void *user_data)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  _instance->next_frame_executors.emplace_back(
      [callback, user_data]() mutable { callback(nullptr, user_data); });
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_request_frame)(uint64_t *result,
                              ne_core_frame_callback callback,
                              const void *user_data) = &_ne_core_request_frame;

/******************************************************************************/
static void _ne_core_exit(uint64_t *result, int32_t return_code)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  _instance->run_exit_callbacks();
  exit(return_code);
}
void (*ne_core_exit)(uint64_t *result, int32_t return_code) = &_ne_core_exit;

/******************************************************************************/
static void _ne_core_error(uint64_t *result,
                           uint64_t error_result,
                           const char *file,
                           int64_t line,
                           const char *message)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  // We use a stringstream to avoid saving and recalling flags, as well as to
  // make the print more atomic.
  std::stringstream stream;
  stream << std::endl << file << "(" << line << "): ";

  if (error_result != 0)
  {
    stream << "error 0x" << std::setfill('0') << std::setw(16) << std::hex
           << error_result << ": ";
  }

  stream << message << std::endl;

  std::cerr << stream.str();

  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_error)(uint64_t *result,
                      uint64_t error_result,
                      const char *file,
                      int64_t line,
                      const char *message) = &_ne_core_error;

/******************************************************************************/
static uint8_t *_ne_core_allocate(uint64_t *result, uint64_t sizeBytes)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, nullptr);

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
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);

  free(memory);
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_core_free)(uint64_t *result, void *memory) = &_ne_core_free;

/******************************************************************************/
int32_t main(int32_t argc, char *argv[])
{
  ne_core_instance instance;
  _instance = &instance;

  int32_t result = ne_core_main(argc, argv);

  while (!_instance->next_frame_executors.empty())
  {
    // Be careful about functions that modify 'next_frame_executors' here
    // (iterator invalidation).
    auto executors = _instance->next_frame_executors;
    for (auto &exector : executors)
    {
      exector();
    }

    _instance->next_frame_executors.clear();
  }

  _instance->run_exit_callbacks();

  _instance = nullptr;
  return result;
}

/******************************************************************************/
void ne_core_memory_set(void *memory, uint8_t value, uint64_t size)
{
  std::memset(memory, static_cast<int>(value), static_cast<size_t>(size));
}

/******************************************************************************/
void ne_core_memory_copy(void *destination, const void *source, uint64_t size)
{
  std::memcpy(destination, source, static_cast<size_t>(size));
}

/******************************************************************************/
int64_t ne_core_memory_compare(const void *a, const void *b, uint64_t size)
{
  return static_cast<int64_t>(std::memcmp(a, b, static_cast<size_t>(size)));
}
