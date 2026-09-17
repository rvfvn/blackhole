/* Lab 2: run with ./build.sh 2. Shared helpers are in lib/bench.c. */
#include "lib/bench.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Computation functions: edit these loops for this experiment. */
/* LAB 2: A row of A times a column of B */
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
    const size_t sizes[] = {64, 256, 512};
    puts("Naive matrix multiplication: same row-times-column formula as on paper.");
    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
        size_t n = sizes[s];
        size_t bytes = n * n * sizeof(float);
        float *a = allocate(bytes), *b = allocate(bytes), *c = allocate(bytes);
        initialize(a, n * n);
        for (size_t i = 0; i < n * n; ++i) {
            b[i] = (float)((int)(i % 17) - 8) / 8.0f;
            c[i] = NAN;
        }
        matmul_naive(a, b, c, n);
        check_matmul(a, b, c, n);
        double best = DBL_MAX;
        for (int r = 0; r < REPETITIONS; ++r) {
            double start = benchmark_time_seconds();
            matmul_naive(a, b, c, n);
            double elapsed = elapsed_since(start);
            check_matmul(a, b, c, n);
            best = smaller(best, elapsed);
        }
        print_matmul(n, "naive", best);
        free(a); free(b); free(c);
    }
    puts("\nLab validations passed.");
    return EXIT_SUCCESS;
}
