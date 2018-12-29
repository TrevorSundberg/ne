/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"

#if defined(__cplusplus)
#  if defined(NE_NO_EXCEPTIONS)
/// Defined as 'try' if we allow exceptions and are in C++.
#    define NE_CORE_TRY if (1)
/// Defined as 'catch' if we allow exceptions and are in C++.
#    define NE_CORE_CATCH(exception) else
#  else
/// Defined as 'try' if we allow exceptions.
#    define NE_CORE_TRY try
/// Defined as 'catch' if we allow exceptions.
#    define NE_CORE_CATCH(exception) catch (exception)
#  endif

/// Validate that something is true at compile time. This is only typically used
/// in headers where C portability is required.
#  define NE_CORE_STATIC_ASSERT(condition, id) static_assert((condition), #  id)
#else
/// Defined as 'try' if we allow exceptions and are in C++.
#  define NE_CORE_TRY if (1)
/// Defined as 'catch' if we allow exceptions and are in C++.
#  define NE_CORE_CATCH(exception) else

/// Validate that something is true at compile time. This is only typically used
/// in headers where C portability is required.
#  define NE_CORE_STATIC_ASSERT(condition, id)                                 \
    typedef char __static_assertion_##id[(condition) ? 1 : -1]
//
#endif

/// A byte pattern that is used for uninitialized memory.
#define NE_CORE_UNINITIALIZED_BYTE (0xCD)

#if defined(NE_CORE_PLATFORM_NE)
/// Conditionally outputs code for the defined platform.
#  define NE_CORE_PLATFORM_IF_NE(code, not_code) code
#else
/// Conditionally outputs code for the defined platform.
#  define NE_CORE_PLATFORM_IF_NE(code, not_code) not_code
#endif

#if defined(NE_CORE_PLATFORM_WINDOWS)
/// Conditionally outputs code for the defined platform.
#  define NE_CORE_PLATFORM_IF_WINDOWS(code, not_code) code
#else
/// Conditionally outputs code for the defined platform.
#  define NE_CORE_PLATFORM_IF_WINDOWS(code, not_code) not_code
#endif

#if defined(NE_CORE_PLATFORM_UNKNOWN)
/// Conditionally outputs code for the defined platform.
#  define NE_CORE_PLATFORM_IF_UNKNOWN(code, not_code) code
#else
/// Conditionally outputs code for the defined platform.
#  define NE_CORE_PLATFORM_IF_UNKNOWN(code, not_code) not_code
#endif

/// Encloses one or more statements into a single statement safely.
#define NE_CORE_ENCLOSURE(code)                                                \
  do                                                                           \
  {                                                                            \
    code                                                                       \
  } while (NE_CORE_FALSE)

#if !defined(NDEBUG)
/// Reports an error if a condition is false. Does not abort.
#  define NE_CORE_ASSERT(condition, text)                                      \
    NE_CORE_ENCLOSURE(if (!(condition)) ne_core_error(NE_CORE_NULL,            \
                                                      NE_CORE_RESULT_INVALID,  \
                                                      __FILE__,                \
                                                      __LINE__,                \
                                                      (text));)
#else
/// Reports an error if a condition is false. Does not abort.
#  define NE_CORE_ASSERT(condition, text)
#endif

/// Reports an error if a condition is true. Does not abort.
#define NE_CORE_ERROR_IF(condition, text) NE_CORE_ASSERT(!(condition), text)

/// Reports an error. Does not abort.
#define NE_CORE_ERROR(text) NE_CORE_ASSERT(0, text)

/// Sets the `result` to a given code if it's available/non-null.
#define NE_CORE_RESULT(code) NE_CORE_ENCLOSURE(if (result) *result = code;)

/// Assert for an internal error.
#define NE_CORE_INTERNAL_ERROR() NE_CORE_ERROR("Internal error")

/// Assert for an internal error. Returns a value.
#define NE_CORE_INTERNAL_ERROR_RETURN(value)                                   \
  NE_CORE_ENCLOSURE(NE_CORE_INTERNAL_ERROR(); return value;)

/// Assert for an internal error. Outputs an error result.
#define NE_CORE_INTERNAL_ERROR_RESULT()                                        \
  NE_CORE_ENCLOSURE(NE_CORE_RESULT(NE_CORE_RESULT_INTERNAL_ERROR);             \
                    NE_CORE_INTERNAL_ERROR(););

/// Assert for an internal error. Outputs an error result. Returns a value.
#define NE_CORE_INTERNAL_ERROR_RESULT_RETURN(value)                            \
  NE_CORE_ENCLOSURE(NE_CORE_RESULT(NE_CORE_RESULT_INTERNAL_ERROR);             \
                    NE_CORE_INTERNAL_ERROR_RETURN(value);)

/// Returns if a library is not supported and sets the result to
/// #NE_CORE_RESULT_NOT_SUPPORTED if the result pointer is provided.
#define NE_CORE_UNSUPPORTED_RETURN(supported, value)                           \
  NE_CORE_ENCLOSURE(if (!(supported)) {                                        \
    NE_CORE_RESULT(NE_CORE_RESULT_NOT_SUPPORTED);                              \
    return value;                                                              \
  })

/// Catches std::bad_alloc and returns an allocation failure result.
#define NE_CORE_CATCH_ALLOCATION_RETURN(value)                                 \
  NE_CORE_CATCH(const ::std::bad_alloc &)                                      \
  {                                                                            \
    NE_CORE_RESULT(NE_CORE_RESULT_ALLOCATION_FAILED);                          \
    return value;                                                              \
  }

/// TO be used by #NE_CORE_UNSUPPORTED_RETURN or
/// #NE_CORE_CATCH_ALLOCATION_RETURN when there is no return.
#define NE_CORE_NONE

/// Implements the supported call given a typically global boolean value.
#define NE_CORE_SUPPORTED_IMPLEMENTATION(supported)                            \
  NE_CORE_ENCLOSURE(NE_CORE_RESULT((supported)                                 \
                                       ? NE_CORE_RESULT_SUCCESS                \
                                       : NE_CORE_RESULT_NOT_SUPPORTED);        \
                    return (supported) ? NE_CORE_TRUE : NE_CORE_FALSE;)
