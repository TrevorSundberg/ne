// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_io, 0, 0);

// Must only support: read, get_position, get_length, is_eof
NE_API void (*ne_io_get_input)(uint64_t* result, ne_core_stream* stream_out);

// Must only support: write, flush, get_position, get_length
NE_API void (*ne_io_get_output)(uint64_t* result, ne_core_stream* stream_out);

NE_END
