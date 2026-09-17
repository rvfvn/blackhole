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


#include "bench.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define REPETITIONS 5

static void require(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "Validation failed: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

static void *allocate(size_t bytes) {
    void *memory = malloc(bytes);
    if (memory == NULL) {
        fprintf(stderr, "Unable to allocate %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return memory;
}

static double elapsed_since(double start) {
    double elapsed = benchmark_time_seconds() - start;
    require(elapsed > 0.0, "timer interval must be positive");
    return elapsed;
}

static double smaller(double a, double b) {
    return a < b ? a : b;
}

static double initialize(float *data, size_t length) {
    double sum = 0.0;
    for (size_t i = 0; i < length; ++i) {
        data[i] = (float)((int)(i % 31) - 15) / 16.0f;
        sum += data[i];
    }
    return sum;
}

static void check_sum(double actual, double expected) {
    require(isfinite(actual) && fabs(actual - expected) <=
            1e-9 * (1.0 + fabs(expected)), "sum mismatch or worker failure");
}

static void check_copy(const float *a, const float *b, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        require(a[i] == b[i], "copy mismatch");
    }
}

static void check_matmul(const float *a, const float *b, const float *c,
                         size_t n) {
    /* Independent double-precision reference, outside all timed regions. */
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            double expected = 0.0;
            double magnitude = 0.0;
            for (size_t k = 0; k < n; ++k) {
                double product = (double)a[i * n + k] * b[k * n + j];
                expected += product;
                magnitude += fabs(product);
            }
            double tolerance = 4.0 * n * FLT_EPSILON * magnitude + 1e-6;
            require(isfinite(c[i * n + j]) &&
                    fabs(c[i * n + j] - expected) <= tolerance,
                    "matmul differs from double-precision reference");
        }
    }
}

static void print_matmul(size_t n, const char *version, double seconds) {
    double operations = 2.0 * n * n * n;
    printf("n=%zu %-22s %.9f s  %.6f GFLOP/s  %.9f TFLOP/s  PASS\n",
           n, version, seconds, operations / seconds / 1e9,
           operations / seconds / 1e12);
}

int main(void) {
    /* Stages 1-3: identify the machine, choose sizes, prepare data. */
    machine_info();
    const size_t lengths[] = {4096, 262144, 4194304};
    const size_t strides[] = {1, 4, 16, 64, 256};
    const size_t sides[] = {64, 512, 2048};
    const size_t matmul_sizes[] = {64, 256, 512};
    const size_t thread_counts[] = {1, 2, 4, 8};
    printf("One untimed warmup per case; minimum of %d measured runs.\n"
           "Allocation, initialization and validation are outside timings.\n"
           "Copy: 256 calls/sample; transpose: 16 calls/sample; times per call.\n"
           "Threaded timings include worker creation and joining.\n\n",
           REPETITIONS);

    /* Stage 4: every stride reads the same number of elements. */
    puts("Strided access:");
    for (size_t l = 0; l < sizeof(lengths) / sizeof(lengths[0]); ++l) {
        size_t length = lengths[l];
        float *data = allocate(length * sizeof(*data));
        double expected = initialize(data, length);
        for (size_t s = 0; s < sizeof(strides) / sizeof(strides[0]); ++s) {
            size_t stride = strides[s];
            check_sum(sum_array_with_stride(data, length, stride), expected);
            double best = DBL_MAX;
            for (int r = 0; r < REPETITIONS; ++r) {
                double start = benchmark_time_seconds();
                double sum = sum_array_with_stride(data, length, stride);
                double elapsed = elapsed_since(start);
                check_sum(sum, expected);
                best = smaller(best, elapsed);
            }
            printf("length=%zu stride=%zu %.9f s checksum=%.6f PASS\n",
                   length, stride, best, expected);
        }
        free(data);
    }

    /* Stage 5: traversal changes, row-major storage stays fixed. */
    puts("\nRow and column traversal:");
    for (size_t s = 0; s < sizeof(sides) / sizeof(sides[0]); ++s) {
        size_t n = sides[s];
        float *matrix = allocate(n * n * sizeof(*matrix));
        double expected = initialize(matrix, n * n);
        double best[2] = {DBL_MAX, DBL_MAX};
        for (int version = 0; version < 2; ++version) {
            double (*sum_matrix)(const float *, size_t, size_t) =
                version == 0 ? sum_matrix_by_rows : sum_matrix_by_columns;
            check_sum(sum_matrix(matrix, n, n), expected);
            for (int r = 0; r < REPETITIONS; ++r) {
                double start = benchmark_time_seconds();
                double sum = sum_matrix(matrix, n, n);
                double elapsed = elapsed_since(start);
                check_sum(sum, expected);
                best[version] = smaller(best[version], elapsed);
            }
            printf("n=%zu %-7s %.9f s checksum=%.6f PASS\n", n,
                   version == 0 ? "rows" : "columns", best[version], expected);
        }
        printf("column time / row time: %.3fx\n", best[1] / best[0]);
        free(matrix);
    }

    /* Stage 6: useful read-plus-write bandwidth, not measured DRAM traffic. */
    puts("\nCopy bandwidth:");
    for (size_t l = 0; l < sizeof(lengths) / sizeof(lengths[0]); ++l) {
        size_t length = lengths[l];
        float *source = allocate(length * sizeof(*source));
        float *destination = allocate(length * sizeof(*destination));
        initialize(source, length);
        copy_float_array(source, destination, length);
        check_copy(source, destination, length);
        double best = DBL_MAX;
        for (int r = 0; r < REPETITIONS; ++r) {
            double start = benchmark_time_seconds();
            for (int batch = 0; batch < 256; ++batch) {
                copy_float_array(source, destination, length);
            }
            double elapsed = elapsed_since(start) / 256.0;
            check_copy(source, destination, length);
            best = smaller(best, elapsed);
        }
        printf("length=%zu %.9f s %.6f effective GB/s PASS\n", length,
               best, 2.0 * length * sizeof(float) / best / 1e9);
        free(source);
        free(destination);
    }

    /* Stage 7: same arithmetic, different layout of B. */
    puts("\nMatrix multiplication:");
    for (size_t s = 0; s < sizeof(matmul_sizes) / sizeof(matmul_sizes[0]); ++s) {
        size_t n = matmul_sizes[s];
        size_t bytes = n * n * sizeof(float);
        float *a = allocate(bytes), *b = allocate(bytes), *bt = allocate(bytes);
        float *naive = allocate(bytes), *prepared = allocate(bytes);
        initialize(a, n * n);
        for (size_t i = 0; i < n * n; ++i) {
            b[i] = (float)((int)(i % 17) - 8) / 8.0f;
            naive[i] = prepared[i] = NAN;
        }
        transpose_square_matrix(b, bt, n);
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                require(bt[j * n + i] == b[i * n + j], "transpose mismatch");
            }
        }
        matmul_naive(a, b, naive, n);
        matmul_with_transposed_b(a, bt, prepared, n);
        check_matmul(a, b, naive, n);
        check_matmul(a, b, prepared, n);
        double naive_best = DBL_MAX, transpose_best = DBL_MAX;
        double prepared_best = DBL_MAX, total_best = DBL_MAX;
        for (int r = 0; r < REPETITIONS; ++r) {
            double start = benchmark_time_seconds();
            matmul_naive(a, b, naive, n);
            double elapsed = elapsed_since(start);
            naive_best = smaller(naive_best, elapsed);
            check_matmul(a, b, naive, n);

            start = benchmark_time_seconds();
            for (int batch = 0; batch < 16; ++batch) {
                transpose_square_matrix(b, bt, n);
            }
            elapsed = elapsed_since(start) / 16.0;
            transpose_best = smaller(transpose_best, elapsed);

            start = benchmark_time_seconds();
            matmul_with_transposed_b(a, bt, prepared, n);
            elapsed = elapsed_since(start);
            prepared_best = smaller(prepared_best, elapsed);
            check_matmul(a, b, prepared, n);

            /* Measure the combined interval directly, not a sum of minima. */
            start = benchmark_time_seconds();
            transpose_square_matrix(b, bt, n);
            matmul_with_transposed_b(a, bt, prepared, n);
            elapsed = elapsed_since(start);
            total_best = smaller(total_best, elapsed);
            check_matmul(a, b, prepared, n);
        }
        print_matmul(n, "naive", naive_best);
        print_matmul(n, "transposed-B kernel", prepared_best);
        print_matmul(n, "transpose + kernel", total_best);
        printf("transpose: %.9f s; kernel speedup: %.3fx; "
               "one-use speedup: %.3fx\n", transpose_best,
               naive_best / prepared_best, naive_best / total_best);
        free(a); free(b); free(bt); free(naive); free(prepared);
    }

    /* Stages 8-9: measure thread coordination and free all data. */
    puts("\nSerial and threaded sums:");
    for (size_t l = 0; l < sizeof(lengths) / sizeof(lengths[0]); ++l) {
        size_t length = lengths[l];
        float *data = allocate(length * sizeof(*data));
        double expected = initialize(data, length);
        check_sum(sum_array_serial(data, length), expected);
        double serial_best = DBL_MAX;
        for (int r = 0; r < REPETITIONS; ++r) {
            double start = benchmark_time_seconds();
            double sum = sum_array_serial(data, length);
            double elapsed = elapsed_since(start);
            check_sum(sum, expected);
            serial_best = smaller(serial_best, elapsed);
        }
        printf("length=%zu serial %.9f s checksum=%.6f PASS\n",
               length, serial_best, expected);
        for (size_t t = 0; t < sizeof(thread_counts) / sizeof(thread_counts[0]); ++t) {
            size_t count = thread_counts[t];
            check_sum(sum_array_threaded(data, length, count), expected);
            double best = DBL_MAX;
            for (int r = 0; r < REPETITIONS; ++r) {
                double start = benchmark_time_seconds();
                double sum = sum_array_threaded(data, length, count);
                double elapsed = elapsed_since(start);
                check_sum(sum, expected);
                best = smaller(best, elapsed);
            }
            printf("length=%zu threads=%zu %.9f s speedup=%.3fx "
                   "checksum=%.6f PASS\n", length, count, best,
                   serial_best / best, expected);
        }
        free(data);
    }
    puts("\nAll benchmark validations passed.");
    return EXIT_SUCCESS;
}
