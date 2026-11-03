// ================
// || main.c ||
// ================


// ||ABSTRACT||
// every C program begins executing through a function named 'main()'. in this lab, main is where we will organize
// our complete benchmark: 
//
// we will:
//          prepare the data, 
//          call the experiment functions written in the other source files,
//          measure their execution time, 
//          check their answers and print the results we want to compare.
//
// the experiment files are intentionally responsible for only their own computation. for example,
// 'copy_float_array()' copies the array, but it does not choose the array size, allocate the arrays or calculate
// bandwidth. these larger experiment decisions belong in this main program, so every function can be tested through one shared program.


#include "bench.h" // --> this provides the shared function menu for the source files in this lab
#include <stddef.h> // --> this provides 'size_t' for array lengths, matrix dimensions and byte counts
#include <stdio.h>  // --> this provides 'printf()' for displaying benchmark information and results
#include <stdlib.h> // --> this provides 'malloc()', 'free()' and tools for reporting allocation failure


int main(void){
    // ||STAGE 1: IDENTIFY THE MACHINE||
    // print the operating environment, CPU architecture, compiler and available logical CPU cores.
    // this gives important context because students will run these experiments on different computers.

    // ||YOUR CODE GOES BELOW HERE||:
    printf("CPU / Memory benchmark lab\n");
    machine_info();

    // ||STAGE 2: CHOOSE THE EXPERIMENT SETTINGS||
    // decide which array lengths, matrix dimensions, stride values, repetition counts and thread counts to test.
    // these settings should be large enough to measure meaningfully without requiring excessive memory or time.

    // ||YOUR CODE GOES BELOW HERE||:
    const size_t array_length = 10000000;
    const size_t matrix_size = 512;
    const size_t thread_count = 8;
    const size_t stride_values[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
    const size_t stride_count = sizeof(stride_values) / sizeof(size_t);

    const double expected_array_sum = (double)array_length;   // expected sum of array
    const double expected_matrix_sum = (double)matrix_size * matrix_size;   // expected sum of matrix

    printf("Array Lengths = %zu\n", array_length);
    printf("matrix size = %zu\n", matrix_size);
    printf("thread count = %zu\n", thread_count);

    // ||STAGE 3: CREATE AND INITIALIZE THE DATA||
    // allocate the arrays and matrices needed by the experiments, verify every allocation and fill the inputs with
    // predictable values. predictable data gives us results that we can check after each experiment finishes.

    // ||YOUR CODE GOES BELOW HERE||:
    float *array = malloc(array_length * sizeof(float));    // allocate array

    float *matrix = malloc(
        matrix_size * matrix_size * sizeof(float)
    );

    float *a = malloc(
        matrix_size * matrix_size * sizeof(float)
    );

    float *b = malloc(
        matrix_size * matrix_size * sizeof(float)
    );

    float *b_transposed = malloc(
        matrix_size * matrix_size * sizeof(float)
    );

    float *c_naive = malloc(
        matrix_size * matrix_size * sizeof(float)
    );

    float *c_transposed = malloc(
        matrix_size * matrix_size * sizeof(float)
    );

    float *destination = malloc(
        array_length * sizeof(float)
    );

    if (array == NULL ||    // check if memory allocation succeeded
        matrix == NULL ||
        a == NULL ||
        b == NULL ||
        b_transposed == NULL ||
        c_naive == NULL ||
        c_transposed == NULL ||
        destination == NULL) {

        printf("Memory allocation failed.\n");

        free(array);
        free(matrix);
        free(a);
        free(b);
        free(b_transposed);
        free(c_naive);
        free(c_transposed);
        free(destination);

        return 1;
    }

    for (size_t i = 0; i < array_length; i++) { // initialize array and destination with 1.0f
        array[i] = 1.0f;
        destination[i] = 0.0f;
    }

    for (size_t i = 0; i < matrix_size * matrix_size; i++) {    // initialize matrices with 1.0f
        matrix[i] = 1.0f;
        a[i] = 1.0f;
        b[i] = 1.0f;
        b_transposed[i] = 0.0f;
        c_naive[i] = 0.0f;
        c_transposed[i] = 0.0f;
    }

    // const double expected_array_sum = (double)array_length;   // expected sum of array


    // ||STAGE 4: OBSERVE STRIDED ARRAY ACCESS||
    // run 'sum_array_with_stride()' with several stride values. time each run and confirm that every traversal returns
    // the same sum even though it requests the array elements in a different order.

    // ||YOUR CODE GOES BELOW HERE||:
    printf("\nStrided Array Access:\n");

    for (size_t i = 0; i < stride_count; i++) {
        size_t stride = stride_values[i];

        double start = benchmark_time_seconds();    // start timer

        double sum = sum_array_with_stride(   // calculate sum
            array,
            array_length,
            stride
        );

        double elapsed = benchmark_time_seconds() - start;   // stop timer

        const char *status = (sum == expected_array_sum) ? "PASS" : "FAIL";

        printf(
            "Stride %zu: %.6f seconds, checksum %.0f, %s\n",    // print result
            stride,
            elapsed,
            sum,
            status
        );
    }

    // ||STAGE 5: COMPARE ROW AND COLUMN ACCESS||
    // run 'sum_matrix_by_rows()' and 'sum_matrix_by_columns()' on the same row-major matrix. compare their times and
    // verify that both functions produce the same mathematical sum.

    // ||YOUR CODE GOES BELOW HERE||:
    printf("\nRow vs Column Traversal:\n");

    double start = benchmark_time_seconds();

    double row_sum = sum_matrix_by_rows(    // calculate sum
        matrix,
        matrix_size,
        matrix_size
    );

    double row_elapsed = benchmark_time_seconds() - start;  // stop timer

    start = benchmark_time_seconds();   // start timer

    double column_sum = sum_matrix_by_columns(   // calculate sum
        matrix,
        matrix_size,
        matrix_size
    );

    double column_elapsed = benchmark_time_seconds() - start;   // stop timer

    printf(
        "Rows: %.6f seconds, checksum %.0f, %s\n",  // print result
        row_elapsed,
        row_sum,
        (row_sum == expected_matrix_sum) ? "PASS" : "FAIL"
    );

    printf(
        "Columns: %.6f seconds, checksum %.0f, %s\n",
        column_elapsed,
        column_sum,
        (column_sum == expected_matrix_sum) ? "PASS" : "FAIL"
    );

    // ||STAGE 6: MEASURE COPY BANDWIDTH||
    // time 'copy_float_array()', inspect the destination after the timed region and calculate effective bandwidth from:
    //              useful bytes moved = 2 * length * sizeof(float)
    //              effective GB/s = useful bytes moved / elapsed seconds / 1000000000.0

    // ||YOUR CODE GOES BELOW HERE||:
    printf("\nCopy Bandwidth\n");

    start = benchmark_time_seconds();   // start timer

    copy_float_array(   // copy array
        array,
        destination,
        array_length
    );

    double copy_elapsed = benchmark_time_seconds() - start; // stop timer

    size_t last_index = array_length - 1;   // last index

    const double useful_bytes = // calculate useful bytes moved
        2.0 * (double)array_length * sizeof(float);

    const double bandwidth =    // calculate effective bandwidth
        useful_bytes / copy_elapsed / 1000000000.0;
    
    const char *copy_status =   // print result
        (destination[0] == 1.0f &&
         destination[last_index] == 1.0f)
            ? "PASS"
            : "FAIL";

    printf(
        "Time: %.6f seconds\n",
        copy_elapsed
    );

    printf(
        "Effective bandwidth: %.3f GB/s\n",
        bandwidth
    );

    printf(
        "Validation: %s\n",
        copy_status
    );

    // ||STAGE 7: COMPARE THE TWO MATRIX MULTIPLICATIONS||
    // create the transposed copy of B, then time 'matmul_naive()' and 'matmul_with_transposed_b()' separately. verify
    // that both produce the same output matrix and calculate their approximate performance using 2 * n * n * n FLOPs.
    // the transpose should be timed separately because rearranging B has its own cost.

    // ||YOUR CODE GOES BELOW HERE||:
    printf("\nMatrix Multiplication:\n");

    start = benchmark_time_seconds();

    transpose_square_matrix(    // transpose matrix
        b,
        b_transposed,
        matrix_size
    );

    double transpose_elapsed =  // stop timer
        benchmark_time_seconds() - start;

    start = benchmark_time_seconds();

    matmul_naive(   // calculate naive matrix multiplication
        a,
        b,
        c_naive,
        matrix_size
    );

    double naive_elapsed =  // stop timer
        benchmark_time_seconds() - start;

    start = benchmark_time_seconds();

    matmul_with_transposed_b(   // calculate transposed matrix multiplication
        a,
        b_transposed,
        c_transposed,
        matrix_size
    );

    double transposed_elapsed = // stop timer
        benchmark_time_seconds() - start;

    const double flops =   // calculate GFLOP/s
        2.0 *
        (double)matrix_size *
        (double)matrix_size *
        (double)matrix_size;

    const double naive_gflops = // calculate GFLOP/s
        flops / naive_elapsed / 1000000000.0;

    const double transposed_gflops =    // calculate GFLOP/s
        flops / transposed_elapsed / 1000000000.0;

    int matmul_valid = 1;   // assume matrix multiplication is valid

    for (size_t i = 0; i < matrix_size * matrix_size; i++) {
        if (c_naive[i] != c_transposed[i]) {
            matmul_valid = 0;
            break;
        }
    }

    printf(
        "Transpose: %.6f seconds\n",
        transpose_elapsed
    );

    printf(
        "Naive: %.6f seconds, %.3f GFLOP/s, %s\n",
        naive_elapsed,
        naive_gflops,
        matmul_valid ? "PASS" : "FAIL"
    );

    printf(
        "Transposed B: %.6f seconds, %.3f GFLOP/s, %s\n",
        transposed_elapsed,
        transposed_gflops,
        matmul_valid ? "PASS" : "FAIL"
    );

    // ||STAGE 8: COMPARE SERIAL AND THREADED WORK||
    // time 'sum_array_serial()' and 'sum_array_threaded()' with the same input. verify their sums and calculate:
    //              speedup = serial elapsed seconds / threaded elapsed seconds
    // this experiment also lets us observe when thread coordination costs more time than it saves.

    // ||YOUR CODE GOES BELOW HERE||:
    printf("\nSerial vs Threaded Sum:\n");

    start = benchmark_time_seconds();

    double serial_sum = sum_array_serial(   // calculate sum
        array,
        array_length
    );

    double serial_elapsed = // stop timer
        benchmark_time_seconds() - start;

    start = benchmark_time_seconds();   // start timer

    double threaded_sum = sum_array_threaded(   // calculate sum
        array,
        array_length,
        thread_count
    );

    double threaded_elapsed =   // stop timer
        benchmark_time_seconds() - start;

    const double speedup =  // calculate speedup
        serial_elapsed / threaded_elapsed;

    const char *serial_status =
        (serial_sum == expected_array_sum)
            ? "PASS"
            : "FAIL";

    const char *threaded_status =
        (threaded_sum == expected_array_sum)
            ? "PASS"
            : "FAIL";

    printf(
        "Serial: %.6f seconds, checksum %.0f, %s\n",
        serial_elapsed,
        serial_sum,
        serial_status
    );

    printf(
        "Threaded (%zu threads): %.6f seconds, checksum %.0f, %s\n",
        thread_count,
        threaded_elapsed,
        threaded_sum,
        threaded_status
    );

    printf(
        "Speedup: %.3fx\n",
        speedup
    );

    // ||STAGE 9: RELEASE THE MEMORY||
    // free every allocation created by this program. each successful 'malloc()' should eventually have one matching
    // 'free()' after the final experiment that needs that memory has finished.

    // ||YOUR CODE GOES BELOW HERE||:
    free(array);    // be free
    free(matrix);
    free(a);
    free(b);
    free(b_transposed);
    free(c_naive);
    free(c_transposed);
    free(destination);
    
    printf("\nBenchmark completed successfully\n");

    // returning 0 tells the operating system that the program finished successfully.
    return 0;
}
