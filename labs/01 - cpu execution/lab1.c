/* Lab 1: run with ./build.sh 1. Shared helpers are in lib/bench.c. */
#include "lib/bench.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

/* Computation functions: edit these loops for this experiment. */
/* LAB 1: Stride is the distance between reads inside the inner loop.
 * matrix[row * columns + column] maps a coordinate into a flat array.
 * Row traversal has stride 1; column traversal has stride 'columns'.
 * Caches hold nearby data in blocks called cache lines. Neighboring reads
 * can reuse values from the same line. Both loops still read every value.
 */
static double sum_matrix_by_rows(const float *matrix, size_t rows, size_t columns) {
    double sum = 0.0;
    for (size_t row = 0; row < rows; ++row) {
        for (size_t column = 0; column < columns; ++column) {
            sum += matrix[row * columns + column];
        }
    }
    return sum;
}

static double sum_matrix_by_columns(const float *matrix, size_t rows, size_t columns) {
    double sum = 0.0;
    for (size_t column = 0; column < columns; ++column) {
        for (size_t row = 0; row < rows; ++row) {
            sum += matrix[row * columns + column];
        }
    }
    return sum;
}

/* Supplied benchmark driver: no timing or printing is needed in the sums above. */
int main(void) {
    printf("One untimed warmup; minimum of %d measured samples.\n"
           "Initialization and validation are outside timings.\n\n", REPETITIONS);
    const size_t sides[] = {64, 512, 2048};
    puts("Lab 1: Read the same matrix across rows vs. down columns");
    printf("%-14s %12s %12s %14s %8s\n",
           "Matrix", "Rows (ms)", "Columns (ms)", "Columns / rows", "Check");
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
        }
        char dimensions[32];
        snprintf(dimensions, sizeof(dimensions), "%zu x %zu", n, n);
        printf("%-14s %12.3f %12.3f %13.2fx %8s\n", dimensions,
               best[0] * 1e3, best[1] * 1e3, best[1] / best[0], "PASS");
        free(matrix);
    }
    puts("Columns / rows: 2.00x means column traversal took twice as long.");
    puts("\nLab validations passed.");
    return EXIT_SUCCESS;
}
