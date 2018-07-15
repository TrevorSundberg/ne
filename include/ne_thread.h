// MIT License (see LICENSE.md) Copyright (c) 2018 Trevor Sundberg
#pragma once
#include "ne_core.h"
NE_BEGIN

ne_bool NE_API ne_thread_supported(ne_result* result);

static const ne_result ne_result_mutex_locked = 0;

typedef struct ne_thread ne_thread;

typedef int64_t(*ne_thread_routine)(void*);

// Pass in null to get the first thread.
//    ne_result_core_invalid_parameter:
//      Parameter 'current' does not point to a valid thread.
ne_thread* NE_API ne_thread_enumerate_next(ne_result* result, ne_thread* current);

ne_thread* NE_API ne_thread_get_current(ne_result* result);

ne_thread* NE_API ne_thread_get_main(ne_result* result);

typedef uint64_t ne_thread_mode;
static const ne_thread_mode ne_thread_mode_resume = 0;
static const ne_thread_mode ne_thread_mode_suspend = 1;
static const ne_thread_mode ne_thread_mode_count = 2;

//    ne_result_core_invalid_parameter:
//      Parameter 'routine' is null or does not point at a valid function.
ne_thread* NE_API ne_thread_create(ne_result* result, ne_thread_routine routine, void* data, ne_thread_mode mode, const char* name);

typedef uint64_t ne_thread_wait_time;
static const ne_thread_wait_time ne_thread_wait_time_none = 0;
static const ne_thread_wait_time ne_thread_wait_time_infinite = (ne_thread_wait_time)-1;
ne_bool NE_API ne_thread_wait(ne_result* result, ne_thread* thread, ne_thread_wait_time nanoseconds);

// Waits on the thread to be completed and frees the resource.
int64_t NE_API ne_thread_free(ne_result* result, ne_thread* thread);

const char* NE_API ne_thread_get_name(ne_result* result, ne_thread* thread);

void NE_API ne_thread_suspend(ne_result* result, ne_thread* thread);

void NE_API ne_thread_resume(ne_result* result, ne_thread* thread);

typedef uint64_t ne_thread_priority;
static const ne_thread_priority ne_thread_priority_low = 0;
static const ne_thread_priority ne_thread_priority_medium = 1;
static const ne_thread_priority ne_thread_priority_high = 2;
static const ne_thread_priority ne_thread_priority_count = 3;
void NE_API ne_thread_set_priority(ne_result* result, ne_thread* thread, ne_thread_priority level);

typedef uint64_t ne_thread_storage_id;

// Gets a thread storage value by id. The id can be any value determined by the user.
// It is common to use the address of a static variable reinterpreted as a uint64_t.
// The address is fixed and globally unique.
void* NE_API ne_thread_storage_get(ne_result* result, ne_thread_storage_id id);
void NE_API ne_thread_storage_set(ne_result* result, ne_thread_storage_id id, void* data);

// Get the number of threads currently running.
uint64_t NE_API ne_thread_get_count(ne_result* result);

// Iterate through all the existing threads in order of creation. Passing null will return the first thread.
ne_thread* NE_API ne_thread_get_next(ne_result* result, ne_thread* thread);

typedef struct ne_thread_mutex ne_thread_mutex;

// Creates a mutex that can be locked and unlocked for the current process only.
// The mutex allows recursive locks on the same thread.
ne_thread_mutex* NE_API ne_thread_mutex_create(ne_result* result);

// Locks the mutex or waits on the mutex to be locked.
// Always returns 'ne_true' when it aquires a lock.
// If the mutex is already locked by the same calling thread,
// this will immediately return 'ne_false'.
//    ne_result_core_invalid_parameter:
//      The pointer did not point to a valid mutex.
ne_bool NE_API ne_thread_mutex_lock(ne_result* result, ne_thread_mutex* mutex, ne_thread_wait_time nanoseconds);

//    ne_result_mutex_locked:
//      The mutex was locked by another thread.
//    ne_result_core_invalid_parameter:
//      The pointer did not point to a valid mutex.
void NE_API ne_thread_mutex_unlock(ne_result* result, ne_thread_mutex* mutex);

// If the mutex is locked by the current thread, it will unlock it and free the resource.
// Once a mutex is freed the pointer becomes invalid.
//    ne_result_mutex_locked:
//      The mutex was locked by another thread.
//    ne_result_core_invalid_parameter:
//      The pointer did not point to a valid mutex.
void NE_API ne_thread_mutex_free(ne_result* result, ne_thread_mutex* mutex);


typedef struct ne_thread_event ne_thread_event;

// Creates an event that multiple threads can wait upon.
// When signalled, automatic reset events will allow a single
// thread through and turn back to the non-signalled state.
// When signalled, manual reset events will allow as many threads
// through until the event is purposely set to the non-signalled state.
void NE_API ne_thread_event_create(ne_result* result, ne_bool signaled, ne_bool auto_reset);

// Setting an automatic reset event to 'ne_false' has no effect.
//    ne_result_core_invalid_parameter:
//      The pointer did not point to a valid event.
void NE_API ne_thread_event_set_signalled(ne_result* result, ne_thread_event* event, ne_bool signaled);

//    ne_result_core_invalid_parameter:
//      The pointer did not point to a valid event.
void NE_API ne_thread_event_free(ne_result* result, ne_thread_event* event);

NE_END
