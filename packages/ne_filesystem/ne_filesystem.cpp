/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_filesystem/ne_filesystem.h"
#include "../ne_core/ne_core_platform.hpp"
#include "../ne_core/ne_core_private.h"
#include <cstdio>
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
#  define VC_EXTRALEAN
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <Shlobj.h>
#  include <Windows.h>

static const constexpr bool _supported = true;
#else
static const constexpr bool _supported = false;
#endif

/******************************************************************************/
static char *allocate_copy_string_result(uint64_t *result,
                                         const char *str,
                                         size_t size)
{
  // +1 for the null terminator.
  ++size;
  void *memory = ne_core_allocate(result, size);
  if (memory != nullptr)
  {
    std::memcpy(memory, str, size);
  }
  return static_cast<char *>(memory);
}

/******************************************************************************/
static char *allocate_copy_string_result(uint64_t *result,
                                         const std::string &str)
{
  return allocate_copy_string_result(result, str.c_str(), str.size());
}

/******************************************************************************/
static std::filesystem::path rooted_canonical(const std::filesystem::path &path)
{
  // This is here to handle the difference between C: and C:\\, which have
  // different meanings on Windows, however in the universal path format we
  // treat them as if they are the same. We mostly justify this because
  // technically the path 'C:' by itself is supposed to be a relative path, and
  // when canonicalized it will apply the working directory to it. However, in
  // many programs (including the Windows shell itself) typing only 'C:' will
  // result in going to the drive itself, rather than to the working directory.
  if (path.has_root_name() && !path.has_root_directory() &&
      !path.has_relative_path())
  {
    // We don't want to call canonical here because it will turn it to the
    // working directory applied to the root_name.
    return path.root_name();
  }
  return std::filesystem::canonical(path);
}

/******************************************************************************/
static std::string
filesystem_to_universal_path_canonical(const std::filesystem::path &path)
{
  // Make an approximate guess as to the path's length.
  std::string universal_path;
  universal_path.reserve(256);
  for (const auto &part : rooted_canonical(path))
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
  return universal_path;
}

/******************************************************************************/
static char *filesystem_to_universal_path_canonical_allocated(
    uint64_t *result, const std::filesystem::path &path)
{
  NE_CORE_TRY
  {
    return allocate_copy_string_result(
        result, filesystem_to_universal_path_canonical(path));
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(nullptr)
}

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
  return NE_CORE_PLATFORM_IF_WINDOWS(NE_FILESYSTEM_SCHEME_WINDOWS,
                                     NE_FILESYSTEM_SCHEME_POSIX);
}
const char *(*ne_filesystem_get_scheme)(uint64_t *result) =
    &_ne_filesystem_get_scheme;

/******************************************************************************/
static ne_core_bool _ne_filesystem_is_case_sensitive(uint64_t *result)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_FALSE);

  NE_CORE_RESULT(NE_CORE_RESULT_SUCCESS);
  return NE_CORE_PLATFORM_IF_WINDOWS(NE_CORE_FALSE, NE_CORE_TRUE);
}
ne_core_bool (*ne_filesystem_is_case_sensitive)(uint64_t *result) =
    &_ne_filesystem_is_case_sensitive;

/******************************************************************************/
static std::filesystem::path
universal_to_filesystem_path(const char *universal_path)
{
#if defined(NE_CORE_PLATFORM_WINDOWS)
  if (*universal_path == '/')
  {
    ++universal_path;
  }
#endif

  // Since all operating systems support '/' then we can treat the universal
  // path as an os path (without the beginning '/' on Windows).
  return rooted_canonical(universal_path);
}

/******************************************************************************/
static void _ne_filesystem_open_file(uint64_t *result,
                                     const ne_filesystem_open_info *info,
                                     ne_core_stream *stream_out)
{
  NE_CORE_UNSUPPORTED_RETURN(_supported, NE_CORE_NONE);
  std::filesystem::path::string_type native_path;
  NE_CORE_TRY
  {
    native_path = universal_to_filesystem_path(info->universal_path).native();
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(NE_CORE_NONE)

  uint32_t create_disposition = 0;
  if (info->if_file_exists == ne_filesystem_if_file_exists_open &&
      info->if_none_exists == ne_filesystem_if_none_exists_create)
  {
    create_disposition = NE_CORE_PLATFORM_IF_WINDOWS(OPEN_ALWAYS, 0);
  }
  else if (info->if_file_exists == ne_filesystem_if_file_exists_open &&
           info->if_none_exists == ne_filesystem_if_none_exists_error)
  {
    create_disposition = NE_CORE_PLATFORM_IF_WINDOWS(OPEN_EXISTING, 0);
  }
  else if (info->if_file_exists == ne_filesystem_if_file_exists_error &&
           info->if_none_exists == ne_filesystem_if_none_exists_create)
  {
    create_disposition = NE_CORE_PLATFORM_IF_WINDOWS(CREATE_NEW, 0);
  }
  else if (info->if_file_exists == ne_filesystem_if_file_exists_error &&
           info->if_none_exists == ne_filesystem_if_none_exists_error)
  {
    NE_CORE_RESULT(NE_CORE_RESULT_INVALID_PARAMETER);
    return;
  }
  else if (info->if_file_exists == ne_filesystem_if_file_exists_truncate &&
           info->if_none_exists == ne_filesystem_if_none_exists_create)
  {
    create_disposition = NE_CORE_PLATFORM_IF_WINDOWS(CREATE_ALWAYS, 0);
  }
  else if (info->if_file_exists == ne_filesystem_if_file_exists_truncate &&
           info->if_none_exists == ne_filesystem_if_none_exists_error)
  {
    create_disposition = NE_CORE_PLATFORM_IF_WINDOWS(TRUNCATE_EXISTING, 0);
  }

  uint32_t desired_access = 0;
  switch (info->io)
  {
  case ne_filesystem_io_read:
    desired_access = NE_CORE_PLATFORM_IF_WINDOWS(GENERIC_READ, 0);
    break;
  case ne_filesystem_io_write:
    desired_access = NE_CORE_PLATFORM_IF_WINDOWS(GENERIC_WRITE, 0);
    break;
  case ne_filesystem_io_read_write:
    desired_access =
        NE_CORE_PLATFORM_IF_WINDOWS(GENERIC_READ | GENERIC_WRITE, 0);
    break;
  case ne_filesystem_io_append:
    desired_access = NE_CORE_PLATFORM_IF_WINDOWS(FILE_APPEND_DATA, 0);
    break;
  case ne_filesystem_io_read_append:
    desired_access =
        NE_CORE_PLATFORM_IF_WINDOWS(GENERIC_READ | FILE_APPEND_DATA, 0);
    break;
  case ne_filesystem_io_max:
  case ne_filesystem_io_force_size:
  default:
    NE_CORE_RESULT(NE_CORE_RESULT_INVALID_PARAMETER);
    return;
  }

  uint32_t share_mode = 0;
  switch (info->share_flags)
  {
  case ne_filesystem_share_flags_none:
    share_mode = NE_CORE_PLATFORM_IF_WINDOWS(0, 0);
    break;
  case ne_filesystem_share_flags_read:
    share_mode = NE_CORE_PLATFORM_IF_WINDOWS(FILE_SHARE_READ, 0);
    break;
  case ne_filesystem_share_flags_write:
    desired_access = NE_CORE_PLATFORM_IF_WINDOWS(GENERIC_WRITE, 0);
    break;
  case ne_filesystem_share_flags_delete:
    desired_access =
        NE_CORE_PLATFORM_IF_WINDOWS(GENERIC_READ | GENERIC_WRITE, 0);
    break;
  case ne_filesystem_share_flags_max:
  case ne_filesystem_share_flags_force_size:
  default:
    NE_CORE_RESULT(NE_CORE_RESULT_INVALID_PARAMETER);
    return;
  }

  uint32_t attributes = NE_CORE_PLATFORM_IF_WINDOWS(FILE_ATTRIBUTE_NORMAL, 0);
  if (info->bypass_cache != NE_CORE_FALSE)
  {
    attributes = NE_CORE_PLATFORM_IF_WINDOWS(
        FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING, 0);
  }

#if defined(NE_CORE_PLATFORM_WINDOWS)
  HANDLE handle = CreateFileW(native_path.c_str(),
                              desired_access,
                              share_mode,
                              nullptr,
                              create_disposition,
                              attributes,
                              nullptr);

  if (handle == INVALID_HANDLE_VALUE)
  {
    switch (GetLastError())
    {
    case ERROR_FILE_NOT_FOUND:
      NE_CORE_RESULT(NE_FILESYSTEM_RESULT_NONE_EXISTS_ERROR);
      return;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
      NE_CORE_RESULT(NE_FILESYSTEM_RESULT_FILE_EXISTS_ERROR);
      return;
    case ERROR_ACCESS_DENIED:
      NE_CORE_RESULT(NE_FILESYSTEM_RESULT_ACCESS_DENIED);
      return;
    case ERROR_BUFFER_OVERFLOW:
    case ERROR_LABEL_TOO_LONG:
    case ERROR_FILENAME_EXCED_RANGE:
    case ERROR_META_EXPANSION_TOO_LONG:
      NE_CORE_RESULT(NE_FILESYSTEM_RESULT_PATH_TOO_LONG);
      return;
    default:
      NE_CORE_RESULT(NE_FILESYSTEM_RESULT_ERROR);
      return;
    }
  }

  std::memset(stream_out, 0, sizeof(*stream_out));
  _file_initialize(stream_out, handle);
#endif

  // The file was opened so lets initialize the stream.
  switch (info->io)
  {
  case ne_filesystem_io_read:
    stream_out->read = &_file_read;
    stream_out->get_position = &_file_get_position;
    stream_out->get_size = &_file_get_size;
    stream_out->seek = &_file_seek;
    stream_out->is_valid = &_file_is_valid;
    stream_out->free = &_file_free;
    break;
  case ne_filesystem_io_write:
    stream_out->write = &_file_write;
    stream_out->flush = &_file_flush;
    stream_out->get_position = &_file_get_position;
    stream_out->get_size = &_file_get_size;
    stream_out->seek = &_file_seek;
    stream_out->is_valid = &_file_is_valid;
    stream_out->free = &_file_free;
    break;
  case ne_filesystem_io_read_write:
    stream_out->read = &_file_read;
    stream_out->write = &_file_write;
    stream_out->flush = &_file_flush;
    stream_out->get_position = &_file_get_position;
    stream_out->get_size = &_file_get_size;
    stream_out->seek = &_file_seek;
    stream_out->is_valid = &_file_is_valid;
    stream_out->free = &_file_free;
    break;
  case ne_filesystem_io_append:
    stream_out->write = &_file_write;
    stream_out->flush = &_file_flush;
    stream_out->get_position = &_file_get_position;
    stream_out->get_size = &_file_get_size;
    stream_out->is_valid = &_file_is_valid;
    stream_out->free = &_file_free;
    break;
  case ne_filesystem_io_read_append:
    stream_out->read = &_file_read;
    stream_out->write = &_file_write;
    stream_out->flush = &_file_flush;
    stream_out->get_position = &_file_get_position;
    stream_out->get_size = &_file_get_size;
    stream_out->seek = &_file_seek;
    stream_out->is_valid = &_file_is_valid;
    stream_out->free = &_file_free;
    break;

  case ne_filesystem_io_max:
  case ne_filesystem_io_force_size:
  default:
    NE_CORE_RESULT(NE_CORE_RESULT_INVALID_PARAMETER);
    return;
  }

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

  std::string os_path;

  NE_CORE_TRY
  {
    os_path = universal_to_filesystem_path(universal_path).u8string();
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(nullptr)

  return allocate_copy_string_result(result, os_path);
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

  return filesystem_to_universal_path_canonical_allocated(result, os_path);
}
char *(*ne_filesystem_translate_os_to_universal)(uint64_t *result,
                                                 const char *os_path) =
    &_ne_filesystem_translate_os_to_universal;

/******************************************************************************/
#if defined(NE_CORE_PLATFORM_WINDOWS)
static std::filesystem::path get_special_path(uint64_t *result,
                                              const GUID &guid,
                                              const char *append_name)
{
  PWSTR path = nullptr;
  if (SHGetKnownFolderPath(
          guid, KF_FLAG_DONT_VERIFY | KF_FLAG_NO_ALIAS, NULL, &path) == S_OK)
  {
    std::filesystem::path os_path(path);
    if (os_path.empty())
    {
      NE_CORE_RESULT(NE_FILESYSTEM_RESULT_ERROR);
      return path;
    }

    if (append_name != nullptr)
    {
      os_path /= append_name;
    }
    return os_path;
  }
  else
  {
    NE_CORE_RESULT(NE_FILESYSTEM_RESULT_ERROR);
    return std::filesystem::path();
  }
}

/******************************************************************************/
static char *get_special_universal_path_canonical_allocated(
    uint64_t *result, const GUID &guid, const char *append_name)
{
  std::filesystem::path path = get_special_path(result, guid, append_name);
  if (path.empty())
  {
    return nullptr;
  }
  return filesystem_to_universal_path_canonical_allocated(result, path);
}
#endif

/******************************************************************************/
static char *_ne_filesystem_get_special_path(
    uint64_t *result, ne_filesystem_special_path special_path)
{
  NE_CORE_TRY
  {
    switch (special_path)
    {
    case ne_filesystem_special_path_directory_private:
    {
#if defined(NE_CORE_PLATFORM_WINDOWS)
      return get_special_universal_path_canonical_allocated(
          result,
          FOLDERID_LocalAppDataLow,
          ne_core_get_application_guid(nullptr));
#else
      NE_CORE_INTERNAL_ERROR_RESULT_RETURN(nullptr);
#endif
    }
    case ne_filesystem_special_path_directory_temporary:
    {
      return filesystem_to_universal_path_canonical_allocated(
          result, std::filesystem::temp_directory_path());
    }
    case ne_filesystem_special_path_directory_public:
    {
#if defined(NE_CORE_PLATFORM_WINDOWS)
      return get_special_universal_path_canonical_allocated(
          result, FOLDERID_Public, nullptr);
#else
      NE_CORE_INTERNAL_ERROR_RESULT_RETURN(nullptr);
#endif
    }
    case ne_filesystem_special_path_directory_working:
    {
      return filesystem_to_universal_path_canonical_allocated(
          result, std::filesystem::current_path());
    }
    case ne_filesystem_special_path_directory_root:
    {
#if defined(NE_CORE_PLATFORM_WINDOWS)
      std::filesystem::path path =
          get_special_path(result, FOLDERID_Windows, nullptr);
      return filesystem_to_universal_path_canonical_allocated(
          result, path.root_name() / path.root_directory());
#else
      return allocate_copy_string_result(result, "/", 1);
#endif
    }
    case ne_filesystem_special_path_max:
    case ne_filesystem_special_path_force_size:
    default:
      NE_CORE_INTERNAL_ERROR_RESULT_RETURN(nullptr);
    }
  }
  NE_CORE_CATCH_ALLOCATION_RETURN(nullptr)
}
char *(*ne_filesystem_get_special_path)(
    uint64_t *result,
    ne_filesystem_special_path special_path) = &_ne_filesystem_get_special_path;