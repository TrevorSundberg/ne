/// @file
/// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#include "../ne_filesystem/ne_filesystem.h"
#include <filesystem>

static const constexpr bool _supported = true;

/******************************************************************************/
static ne_core_bool _ne_filesystem_supported(uint64_t *result)
{
  NE_CORE_SUPPORTED_IMPLEMENTATION(_supported);
}
ne_core_bool (*ne_filesystem_supported)(uint64_t *result) =
    &_ne_filesystem_supported;
