// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_io_supported(ne_result* result);

ne_file* NE_API ne_io_get_input(ne_result* result);
ne_file* NE_API ne_io_get_output(ne_result* result);

NE_END
