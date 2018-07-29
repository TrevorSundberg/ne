// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "../ne_core/ne_core.h"
NE_BEGIN

NE_DECLARE_PACKAGE(ne_thread, 0, 0);

static const uint64_t ne_thread_result_mutex_locked = 0;

typedef struct ne_thread ne_thread;

typedef int64_t (*ne_thread_routine)(void*);

// Pass in null to get the first thread.
//    ne_core_result_invalid_parameter:
//      Parameter 'current' does not point to a valid thread.
NE_API ne_thread* (*ne_thread_enumerate_next)(uint64_t* result, ne_thread* current);

NE_API ne_thread* (*ne_thread_get_current)(uint64_t* result);

NE_API ne_thread* (*ne_thread_get_main)(uint64_t* result);

typedef uint64_t ne_thread_mode;
static const ne_thread_mode ne_thread_mode_resume = 0;
static const ne_thread_mode ne_thread_mode_suspend = 1;
static const ne_thread_mode ne_thread_mode_count = 2;

//    ne_core_result_invalid_parameter:
//      Parameter 'routine' is null or does not point at a valid function.
NE_API ne_thread* (*ne_thread_create)(uint64_t* result, ne_thread_routine routine, void* data, ne_thread_mode mode, const char* name);

typedef uint64_t ne_thread_wait_time;
static const ne_thread_wait_time ne_thread_wait_time_none = 0;
static const ne_thread_wait_time ne_thread_wait_time_infinite = (ne_thread_wait_time)-1;
NE_API ne_core_bool (*ne_thread_wait)(uint64_t* result, ne_thread* thread, ne_thread_wait_time nanoseconds);

// Waits on the thread to be completed and frees the resource.
NE_API int64_t (*ne_thread_free)(uint64_t* result, ne_thread* thread);

NE_API const char* (*ne_thread_get_name)(uint64_t* result, ne_thread* thread);

NE_API void (*ne_thread_suspend)(uint64_t* result, ne_thread* thread);

NE_API void (*ne_thread_resume)(uint64_t* result, ne_thread* thread);

typedef uint64_t ne_thread_priority;
static const ne_thread_priority ne_thread_priority_low = 0;
static const ne_thread_priority ne_thread_priority_medium = 1;
static const ne_thread_priority ne_thread_priority_high = 2;
static const ne_thread_priority ne_thread_priority_count = 3;
NE_API void (*ne_thread_set_priority)(uint64_t* result, ne_thread* thread, ne_thread_priority level);

typedef uint64_t ne_thread_storage_id;

// Gets a thread storage value by id. The id can be any value determined by the user.
// It is common to use the address of a static variable reinterpreted as a uint64_t.
// The address is fixed and globally unique.
NE_API void* (*ne_thread_storage_get)(uint64_t* result, ne_thread_storage_id id);
NE_API void (*ne_thread_storage_set)(uint64_t* result, ne_thread_storage_id id, void* data);

// Get the number of threads currently running.
NE_API uint64_t (*ne_thread_get_count)(uint64_t* result);

// Iterate through all the existing threads in order of creation. Passing null will return the first thread.
NE_API ne_thread* (*ne_thread_get_next)(uint64_t* result, ne_thread* thread);

typedef struct ne_thread_mutex ne_thread_mutex;

// Creates a mutex that can be locked and unlocked for the current process only.
// The mutex allows recursive locks on the same thread.
NE_API ne_thread_mutex* (*ne_thread_mutex_create)(uint64_t* result);

// Locks the mutex or waits on the mutex to be locked.
// Always returns 'ne_core_true' when it aquires a lock.
// If the mutex is already locked by the same calling thread,
// this will immediately return 'ne_core_false'.
//    ne_core_result_invalid_parameter:
//      The pointer did not point to a valid mutex.
NE_API ne_core_bool (*ne_thread_mutex_lock)(uint64_t* result, ne_thread_mutex* mutex, ne_thread_wait_time nanoseconds);

//    ne_thread_result_mutex_locked:
//      The mutex was locked by another thread.
//    ne_core_result_invalid_parameter:
//      The pointer did not point to a valid mutex.
NE_API void (*ne_thread_mutex_unlock)(uint64_t* result, ne_thread_mutex* mutex);

// If the mutex is locked by the current thread, it will unlock it and free the resource.
// Once a mutex is freed the pointer becomes invalid.
//    ne_thread_result_mutex_locked:
//      The mutex was locked by another thread.
//    ne_core_result_invalid_parameter:
//      The pointer did not point to a valid mutex.
NE_API void (*ne_thread_mutex_free)(uint64_t* result, ne_thread_mutex* mutex);


typedef struct ne_thread_event ne_thread_event;

// Creates an event that multiple threads can wait upon.
// When signalled, automatic reset events will allow a single
// thread through and turn back to the non-signalled state.
// When signalled, manual reset events will allow as many threads
// through until the event is purposely set to the non-signalled state.
NE_API void (*ne_thread_event_create)(uint64_t* result, ne_core_bool signaled, ne_core_bool auto_reset);

// Setting an automatic reset event to 'ne_core_false' has no effect.
//    ne_core_result_invalid_parameter:
//      The pointer did not point to a valid event.
NE_API void (*ne_thread_event_set_signalled)(uint64_t* result, ne_thread_event* event, ne_core_bool signaled);

//    ne_core_result_invalid_parameter:
//      The pointer did not point to a valid event.
NE_API void (*ne_thread_event_free)(uint64_t* result, ne_thread_event* event);

NE_END
