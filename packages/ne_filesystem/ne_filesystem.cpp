/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_filesystem/ne_filesystem.h"
#include <cstring>
#include <filesystem>

// For older systems that only support std::experimental::filesystem.
namespace std // NOLINT
{
namespace experimental
{
namespace filesystem
{
} // namespace filesystem
} // namespace experimental
namespace filesystem
{
using namespace std::experimental::filesystem; // NOLINT
} // namespace filesystem
} // namespace std

#if defined(NE_CORE_PLATFORM_WINDOWS)
// https://docs.microsoft.com/en-us/windows/desktop/fileio/naming-a-file
// https://docs.microsoft.com/en-us/dotnet/standard/io/file-path-formats
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

static const constexpr bool _supported = true;
#else
static const constexpr bool _supported = false;
#endif

/******************************************************************************/
static char *_ne_filesystem_translate_universal_to_os(
    uint64_t *result, const char *universal_path);

/******************************************************************************/
static ne_core_bool _ne_filesystem_supported(uint64_t *result)
{
  NE_CORE_SUPPORTED_IMPLEMENTATION(_supported);
}
ne_core_bool (*ne_filesystem_supported)(uint64_t *result) =
    &_ne_filesystem_supported;

/******************************************************************************/
static const char *_ne_filesystem_get_scheme(uint64_t *result)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, nullptr);

  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
#if defined(NE_CORE_PLATFORM_WINDOWS)
  return NE_FILESYSTEM_SCHEME_WINDOWS;
#else
  return NE_FILESYSTEM_SCHEME_POSIX;
#endif
}
const char *(*ne_filesystem_get_scheme)(uint64_t *result) =
    &_ne_filesystem_get_scheme;

/******************************************************************************/
static void _ne_filesystem_open_file(uint64_t *result,
                                     const ne_filesystem_open_info *info,
                                     ne_core_stream *stream_out)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);
  (void)result;
  (void)info;
  (void)stream_out;

  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
}
void (*ne_filesystem_open_file)(uint64_t *result,
                                const ne_filesystem_open_info *info,
                                ne_core_stream *stream_out) =
    &_ne_filesystem_open_file;

/******************************************************************************/
static char *_ne_filesystem_translate_universal_to_os(
    uint64_t *result, const char *universal_path)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, nullptr);

#if defined(NE_CORE_PLATFORM_WINDOWS)
  if (*universal_path == '/')
  {
    ++universal_path;
  }
#endif

  std::string os_path;

  // Since all operating systems support '/' then we can treat the universal
  // path as an os path (without the beginning '/' on Windows).
  NE_CORE_TRY
  {
    std::filesystem::path path = std::filesystem::canonical(universal_path);
    os_path = path.u8string();
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(nullptr)

  // +1 for the null terminator.
  size_t os_path_size = os_path.size() + 1;
  void *os_path_memory = ne_core_allocate(nullptr, os_path_size);
  if (os_path_memory == nullptr)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_ALLOCATION_FAILED);
    return nullptr;
  }
  memcpy(os_path_memory, os_path.c_str(), os_path_size);
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  return static_cast<char *>(os_path_memory);
}
char *(*ne_filesystem_translate_universal_to_os)(uint64_t *result,
                                                 const char *universal_path) =
    &_ne_filesystem_translate_universal_to_os;

/******************************************************************************/
static char *_ne_filesystem_translate_os_to_universal(uint64_t *result,
                                                      const char *os_path)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, nullptr);

  // TODO(Trevor.Sundberg): This could be quite a bit more efficient if we wrote
  // it all ourselves. On POSIX platforms the path could just be a passthrough,
  // but on Windows we would need a full path grammar.

  std::string universal_path;

  NE_CORE_TRY
  {
    std::filesystem::path path = std::filesystem::canonical(os_path);

    // Make an approximate guess as to the path's length.
    universal_path.reserve(256);
    for (const auto &part : path)
    {
      auto cstr = part.c_str();
      auto c = cstr[0];
      if (c == '\0')
      {
        continue;
      }
      if (cstr[1] == '\0' &&
          (c == '/' || c == std::filesystem::path::preferred_separator))
      {
        continue;
      }
      universal_path += '/' + part.u8string();
    }
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(nullptr)

  // +1 for the null terminator.
  size_t universal_path_size = universal_path.size() + 1;
  void *universal_path_memory = ne_core_allocate(nullptr, universal_path_size);
  if (universal_path_memory == nullptr)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_ALLOCATION_FAILED);
    return nullptr;
  }

  memcpy(universal_path_memory, universal_path.c_str(), universal_path_size);
  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  return static_cast<char *>(universal_path_memory);
}
char *(*ne_filesystem_translate_os_to_universal)(uint64_t *result,
                                                 const char *os_path) =
    &_ne_filesystem_translate_os_to_universal;
