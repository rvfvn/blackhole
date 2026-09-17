#ifndef BENCH_H
#define BENCH_H

#include <stddef.h>

#define REPETITIONS 5

/* Declarations describe how to call shared functions. Their definitions
 * (the bodies in braces) live in bench.c. Include this header in a lab file
 * so the compiler can check its calls; build.sh links in bench.c.
 * The include guard above and below prevents processing this header twice.
 */
double benchmark_time_seconds(void);
void machine_info(void);
void require(int condition, const char *message);
void *allocate(size_t bytes);
double elapsed_since(double start);
double smaller(double a, double b);
double initialize(float *data, size_t length);
void check_sum(double actual, double expected);
void check_matmul(const float *a, const float *b, const float *c, size_t n);
void print_matmul(size_t n, const char *version, double seconds);

#endif
