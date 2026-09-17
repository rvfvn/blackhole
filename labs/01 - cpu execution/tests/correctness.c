#include "bench.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    const float data[] = {1.25f, -2.5f, 3.0f, 8.0f, -4.0f, 0.5f, 7.0f};
    const double expected = 13.25;
    const size_t strides[] = {0, 1, 2, 3, 7, 8, SIZE_MAX};
    for (size_t i = 0; i < sizeof(strides) / sizeof(strides[0]); ++i) {
        assert(sum_array_with_stride(data, 7, strides[i]) == expected);
        assert(sum_array_with_stride(NULL, 0, strides[i]) == 0.0);
    }
    const float rectangular[] = {1, 2, 3, 4, 5, 6};
    assert(sum_matrix_by_rows(rectangular, 2, 3) == 21.0);
    assert(sum_matrix_by_columns(rectangular, 2, 3) == 21.0);
    assert(sum_matrix_by_rows(NULL, 0, 3) == 0.0);
    assert(sum_matrix_by_columns(NULL, 3, 0) == 0.0);
    float copied[9] = {-99, 0, 0, 0, 0, 0, 0, 0, -99};
    copy_float_array(data, copied + 1, 7);
    for (size_t i = 0; i < 7; ++i) assert(copied[i + 1] == data[i]);
    assert(copied[0] == -99 && copied[8] == -99);
    copy_float_array(NULL, NULL, 0);

    const float a[] = {1, 2, 3, 4}, b[] = {5, 6, 7, 8};
    const float answer[] = {19, 22, 43, 50};
    float bt[4], c[4], ct[4];
    transpose_square_matrix(b, bt, 2);
    assert(bt[0] == 5 && bt[1] == 7 && bt[2] == 6 && bt[3] == 8);
    matmul_naive(a, b, c, 2);
    matmul_with_transposed_b(a, bt, ct, 2);
    for (size_t i = 0; i < 4; ++i) {
        assert(c[i] == answer[i] && ct[i] == answer[i]);
    }
    /* Non-square traversal above, odd square sizes and signed inputs here. */
    for (size_t n = 1; n <= 9; ++n) {
        float left[81], right[81], transposed[81], first[81], second[81];
        for (size_t i = 0; i < n * n; ++i) {
            left[i] = (float)((int)(i % 7) - 3) / 3.0f;
            right[i] = (float)((int)(i % 11) - 5) / 7.0f;
            first[i] = second[i] = NAN;
        }
        transpose_square_matrix(right, transposed, n);
        matmul_naive(left, right, first, n);
        matmul_with_transposed_b(left, transposed, second, n);
        for (size_t row = 0; row < n; ++row) {
            for (size_t col = 0; col < n; ++col) {
                assert(transposed[col * n + row] == right[row * n + col]);
                double reference = 0.0;
                for (size_t k = 0; k < n; ++k)
                    reference += (double)left[row * n + k] * right[k * n + col];
                assert(isfinite(first[row * n + col]));
                assert(isfinite(second[row * n + col]));
                assert(fabs(first[row * n + col] - reference) < 1e-5);
                assert(fabs(second[row * n + col] - reference) < 1e-5);
            }
        }
    }
    matmul_naive(NULL, NULL, NULL, 0);
    transpose_square_matrix(NULL, NULL, 0);
    matmul_with_transposed_b(NULL, NULL, NULL, 0);
    assert(sum_array_serial(data, 7) == expected);
    assert(sum_array_serial(NULL, 0) == 0.0);
    for (size_t count = 1; count <= 7; ++count)
        assert(sum_array_threaded(data, 7, count) == expected);
    assert(sum_array_threaded(NULL, 0, 0) == 0.0);
    assert(isnan(sum_array_threaded(data, 7, 0)));
    assert(isnan(sum_array_threaded(data, 7, 8)));
    puts("All correctness tests passed.");
    return 0;
}
