// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_file_supported(ne_result* result);

static const ne_result ne_result_file_error = 0;

typedef struct ne_file ne_file;

// Reads the entire size specified unless the end of the stream occurs or an error occurs.
// Note that when it reaches the end of the stream it will still return ne_result_core_success.
// This operation is non-blocking and will return how much was read.
//    ne_result_core_invalid_parameter:
//      If the file was a write only file or invalid file pointer.
//    ne_result_file_error:
//      If an error occurred on reading the file.
uint64_t NE_API ne_file_read(ne_result* result, ne_file* file, void* buffer, uint64_t size, ne_bool blocking);

// This operation is non-blocking and will return how much was written.
//    ne_result_core_invalid_parameter:
//      If the file was a read only file or invalid file pointer.
//    ne_result_file_error:
//      If an error occurred on writing the file.
uint64_t NE_API ne_file_write(ne_result* result, ne_file* file, const void* buffer, uint64_t size, ne_bool blocking);

//    ne_result_core_invalid_parameter:
//      If the file was a read only file or invalid file pointer.
//    ne_result_file_error:
//      If an error occurred on writing the file.
void NE_API ne_file_flush(ne_result* result, ne_file* file);


uint64_t NE_API ne_file_get_position(ne_result* result, ne_file* file);

uint64_t NE_API ne_file_get_length(ne_result* result, ne_file* file);


typedef uint64_t ne_file_seek_origin;
static const ne_file_seek_origin ne_file_seek_origin_begin = 0;
static const ne_file_seek_origin ne_file_seek_origin_current = 1;
static const ne_file_seek_origin ne_file_seek_origin_end = 2;
static const ne_file_seek_origin ne_file_seek_origin_count = 3;

void NE_API ne_file_seek(ne_result* result, ne_file* file, ne_file_seek_origin origin, uint64_t position);

void NE_API ne_file_free(ne_result* result, ne_file* file);

NE_END
