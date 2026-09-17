/* Lab 3: run with ./build.sh 3. Shared helpers are in lib/bench.c. */
#include "lib/bench.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Computation functions: edit these loops for this experiment. */
/* LAB 3: Make B columns contiguous before multiplying */
static void transpose_square_matrix(const float *source, float *destination, size_t n) {
    for (size_t row = 0; row < n; ++row) {
        for (size_t column = 0; column < n; ++column) {
            destination[column * n + row] = source[row * n + column];
        }
    }
}

static void matmul_with_transposed_b(const float *a, const float *b_transposed, float *c, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < n; ++k) {
                sum += a[i * n + k] * b_transposed[j * n + k];
            }
            c[i * n + j] = sum;
        }
    }
}

/* Naive baseline from lab 2, included here to keep this lab standalone. */
static void matmul_naive(const float *a, const float *b, float *c, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < n; ++k) {
                sum += a[i * n + k] * b[k * n + j];
            }
            c[i * n + j] = sum;
        }
    }
}

/* Supplied benchmark driver: timing, validation, and reporting. */
int main(void) {
    machine_info();
    printf("One untimed warmup; minimum of %d measured samples.\n"
           "Initialization and validation are outside timings.\n\n", REPETITIONS);
    const size_t matmul_sizes[] = {64, 256, 512};
    puts("Standalone transpose timing batches 16 calls; combined timing is one use.");
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
    puts("\nLab validations passed.");
    return EXIT_SUCCESS;
}
