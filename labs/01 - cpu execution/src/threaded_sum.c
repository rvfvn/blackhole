// ======================
// || threaded_sum.c ||
// ======================


// ||ABSTRACT||
// this program constructs the 'sum_array_serial()' and 'sum_array_threaded()' functions. both functions add every
// float from the same one-dimensional array declared within here, and return the same mathematical sum. 
// however, this threaded version divides this array into separate ranges so multiple workers can contribute partial sums.
//
// this program, therefore, demonstrates that in where we change how work is divided, it may allow several CPU cores participate toward
// the task! 
//
// more workers can help for a task, but...
//      creating threads, 
//      scheduling them, 
//      waiting for them and 
//      combining their results 
// takes a slight but of time time. this meansparallel work is not automatically faster and that
// the amount of useful work has to be large enough to repay that overhead! 
//
// especially if we are dealing with a CPU and not 
// a piece of hardware that is specialized for parrell processing like a GPU or some kind of AI Application-specific-integrated-circuit
// (ASIC) like... tenstorrent's blackhole !!!


// ||WHAT A THREAD IS||
// one running program is called a process. 
// a process can contain multiple threads. 
//              and those threads share the process's memory. 
// this means every worker thread can read from the same input array without making a separate copy of it!
//
// the operating system schedules these threads onto the logical CPU cores available to the program. 
// ex.:
//       asking for 8 threads does not guarantee that all 8 execute at the exact same instant, but it gives the operating system 8 pieces 
//       of work that may/can be scheduled in parallel when enough CPU resources are available at a given time
//
// this lab uses POSIX threads, commonly called pthreads. they are available on macOS, Linux and Windows through WSL.
// students compiling this file will use the '-pthread' compiler option so the thread interface is enabled and linked.


// ||DIVIDING THE ARRAY TASK||
// each worker should receive one contiguous, non-overlapping range of array indexes. 
// ranges use:
//      a start index that is included 
// and 
//      an end index that is not included 
// which we can write as [start_index, end_index).
//
// ex. an array with 10 elements divided across 3 threads can become:
//              thread "0" = indexes [0, 4)  = 0, 1, 2, 3
//              thread "1" = indexes [4, 7)  = 4, 5, 6
//              thread "2" = indexes [7, 10) = 7, 8, 9
//
// 10 does not divide evenly by 3, so one thread receives one extra element. there is 
//      no index overlap, 
//      no index is skipped
// and 
//      every worker still reads a contiguous section of memory.


// ||PARTIAL SUMS AND DATA RACES||
// every thread instantiated will calculate its own 'partial_sum'. this means that the workers should NOT repeatedly update one 
// shared sum because several threads could read and write that value at the same time. 
// that unsafe overlap is called a data race and can produce an incorrect result.
//
// (side tangent,. data race should not be confused with what you would learn in intro to OS at uncc (itsc 3146) called a race condition: 
//  - a race condition is a high-level logic flaw regarding the order of execution, 
//  - whereas a data race is a low-level memory access conflict.
//  )
//
// instead, each worker receives a separate task record containing:
//             - the input array address
//             - its starting index
//             - its ending index
//             - its own partial sum
//
// after all workers finish, the main thread joins them and combines those separate partial sums once. joining means
// waiting until a worker has completed before using the result it produced.


// ||SERIAL TIME, THREADED TIME AND SPEEDUP||
// src/main.c will time the serial and threaded functions separately. their performance comparison can be reported as:
//              speedup = serial elapsed seconds / threaded elapsed seconds
//
// ex.: 
//      if the serial sum takes 0.080 seconds and the threaded sum takes 0.040 seconds:
//                      speedup = 0.080 / 0.040 = 2.0x
//
// to interpret this, a 2.0x speedup means the threaded version completed in half the time. 
// a value below 1.0x means threading was slower than just doing it normally.
//
// this first implementation includes: 
//      thread creation, 
//      joining 
// and 
//      coordination 
// inside the threaded function, which is intentional since these costs help demonstrate why small workloads 
// may not benefit from parallel execution in all cases!


#define _POSIX_C_SOURCE 200809L

#include "bench.h"
#include <pthread.h> // --> this provides the POSIX thread types and operations used by the threaded implementation
#include <stddef.h>  
#include <stdlib.h>


// each worker thread will receive one separate task record like this one.
// the student implementation will pass its address into a private pthread worker function.
typedef struct {
    const float *data;
    size_t start_index;
    size_t end_index;
    double partial_sum;
} ThreadSumTask;

static void *sum_worker(void *argument) {   // private pthread worker function
    ThreadSumTask *task = (ThreadSumTask *)argument;    // cast argument to task record

    task->partial_sum = 0.0;    // initialize partial sum to 0.0

    for (size_t i = task->start_index; i < task->end_index; i++) {
        task->partial_sum += task->data[i];
    }

    return NULL;
}

double sum_array_serial(const float *data, size_t length){
    /* Adds every float in one array from beginning to end using the calling thread only.

    Args:
        'data': The address of the first element in the array.
        'length': The total number of float elements stored in the array.

    Returns:
        The sum of every array element as a 'double'. This is the correctness and performance baseline used to
        evaluate the threaded result.

    Notes:
        1. The function must not allocate memory, print output, select benchmark sizes, or measure time.
           (src/main.c will handle those responsibilities)
        2. 'const' means this function can read the array, but it is not allowed to modify its values.
        3. If 'length' is 0, the function should return 0.0.

    Algorithm:
        1. Create a sum that begins at 0.
        2. Move through every array index from 0 through length - 1.
        3. Add each reached value into the sum.
        4. Return the completed sum after every element has been reached exactly once.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    double sum = 0.0;   // initialize sum to 0.0

    for (size_t i = 0; i < length; i++){
        sum += data[i];   // add value to sum
    }

    return sum;

    // temp lines to prevent unused-parameter warnings while the function is still empty.
    // (void)data;
    // (void)length;

    // temporary placeholder only! you should replace '0.0' with the computed sum, however you do that.
    // return 0.0;
}


double sum_array_threaded(const float *data, size_t length, size_t thread_count){
    /* Adds every float in one array by dividing the indexes across multiple POSIX worker threads.

    Args:
        'data': The address of the first element in the shared input array.
        'length': The total number of float elements stored in the array.
        'thread_count': The number of worker threads requested for the sum.

    Returns:
        The combined sum of every worker's separate partial sum as a 'double'. The result should agree with the
        serial sum, allowing for a possible tiny floating-point rounding difference from the changed addition order.

    Notes:
        1. 'thread_count' must be greater than 0. when 'length' is also greater than 0, it should not exceed 'length'.
        2. Each worker must receive a separate, contiguous and non-overlapping range of indexes.
        3. Each worker writes only to its own 'partial_sum'; workers must not update one shared sum.
        4. Every successfully created worker must be joined before its partial sum is combined or the function finishes.
        5. The function must clean up any thread or task storage it creates before returning.
        6. The function must not print benchmark results, select sizes, measure time or calculate speedup.
           (src/main.c will handle those responsibilities)
        7. 'const' means the workers can read the shared input array, but they are not allowed to modify its values.
        8. If 'length' is 0, the function should return 0.0 without creating any worker threads.

    Algorithm:
        1. Validate the requested thread count and handle an empty array.
        2. Create storage for 'thread_count' pthread handles and 'ThreadSumTask' records.
        3. Divide 'length' elements into contiguous ranges, distributing any remainder without skipping elements.
        4. Create one worker thread for each task. each worker adds the values in [start_index, end_index) and stores
           its answer in that task's 'partial_sum'.
        5. Join every successfully created worker so all partial sums are complete.
        6. Combine the task partial sums into one final double result.
        7. Clean up the thread and task storage, then return the combined sum.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    if (length == 0){
        return 0.0;   // return 0.0 if array is empty
    }

    if (thread_count == 0){
        return 0.0;   // return 0.0 if thread count is 0
    }

    if (thread_count > length){
        thread_count = length;   // limit thread count to array length
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * thread_count);   // allocate thread handles
    ThreadSumTask *tasks = malloc(sizeof(ThreadSumTask) * thread_count);   // allocate task records

    if (threads == NULL || tasks == NULL){
        free(threads);   // free any allocated memory
        free(tasks);
        return 0.0;   // return 0.0 if memory allocation failed
    }

    size_t base_size = length / thread_count;
    size_t remainder = length % thread_count;
    size_t current_index = 0;
    size_t created_threads = 0;

    for (size_t i = 0; i < thread_count; i++){  // create one task for each thread
        size_t start_index = current_index;
        size_t end_index = current_index + base_size;

        if (i < remainder){
            end_index++;   // add one to the end index if we are on the last thread
        }

        tasks[i].data = data;
        tasks[i].start_index = start_index;
        tasks[i].end_index = end_index;
        tasks[i].partial_sum = 0.0;

        if (pthread_create(&threads[i], NULL, sum_worker, &tasks[i]) != 0){ // create thread
            for (size_t j = 0; j < i; j++){
                pthread_join(threads[j], NULL);   // wait for any created threads to complete
            }
            free(threads);   // free any allocated memory
            free(tasks);
            return 0.0;   // return 0.0 if thread creation failed
        }

        current_index = end_index;
        created_threads++;
    }

    double partial_sum = 0.0;

    for (size_t i = 0; i < created_threads; i++){
        pthread_join(threads[i], NULL);   // wait for thread to complete
        partial_sum += tasks[i].partial_sum;   // add partial sum to final sum
    }

    free(threads);   // free any allocated memory
    free(tasks);

    return partial_sum;   // return the final sum

    // the student will also create a private pthread worker function with this general signature:
    //      static void *sum_worker(void *argument);

    // temp lines to prevent unused-parameter warnings while the function is still empty.
    // (void)data;
    // (void)length;
    // (void)thread_count;

    // temporary placeholder only! you should replace '0.0' with the combined partial sums.
    // return 0.0;
}
