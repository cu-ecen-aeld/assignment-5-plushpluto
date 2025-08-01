#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
#define ERROR_LOG(msg,...) fprintf(stderr, "threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    
    // Sleep before attempting to obtain mutex
    usleep(thread_func_args->wait_to_obtain_ms * 1000);
    
    // Attempt to obtain mutex
    int mutex_lock_result = pthread_mutex_lock(thread_func_args->mutex);
    
    if (mutex_lock_result != 0) {
        ERROR_LOG("Failed to lock mutex: %s", strerror(mutex_lock_result));
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    
    // Sleep after obtaining mutex
    usleep(thread_func_args->wait_to_release_ms * 1000);
    
    // Release mutex
    int mutex_unlock_result = pthread_mutex_unlock(thread_func_args->mutex);
    
    if (mutex_unlock_result != 0) {
        ERROR_LOG("Failed to unlock mutex: %s", strerror(mutex_unlock_result));
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }
    
    // Mark thread as successful
    thread_func_args->thread_complete_success = true;
    
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    // Dynamically allocate thread_data
    struct thread_data* thread_data = malloc(sizeof(struct thread_data));
    
    if (thread_data == NULL) {
        ERROR_LOG("Failed to allocate memory for thread data");
        return false;
    }
    
    // Initialize thread_data
    thread_data->mutex = mutex;
    thread_data->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_data->wait_to_release_ms = wait_to_release_ms;
    thread_data->thread_complete_success = false;
    
    // Create thread
    int pthread_create_result = pthread_create(thread, NULL, threadfunc, thread_data);
    
    if (pthread_create_result != 0) {
        ERROR_LOG("Failed to create thread: %s", strerror(pthread_create_result));
        free(thread_data);
        return false;
    }
    
    return true;
}