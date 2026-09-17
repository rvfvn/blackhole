/* Supplied timing, machine information, validation, and reporting helpers. */
/* Enable the system interfaces used for timing and CPU information. */
#if defined(__APPLE__)
#define _DARWIN_C_SOURCE
#endif
#define _POSIX_C_SOURCE 200809L

#include "bench.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* Shared timing, machine information, and validation helpers. */
/* Shared infrastructure: seconds from a clock that cannot jump backwards. */
double benchmark_time_seconds(void) {
    struct timespec timestamp;
    if (clock_gettime(CLOCK_MONOTONIC, &timestamp) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double)timestamp.tv_sec + (double)timestamp.tv_nsec / 1e9;
}

void machine_info(void) {

    printf("Machine information:\n");
#if defined(__APPLE__) && defined(__MACH__)
    printf("Operating system: macOS\n");
#elif defined(__linux__)
    const char *wsl_distribution = getenv("WSL_DISTRO_NAME");
    if (wsl_distribution != NULL && wsl_distribution[0] != '\0') {printf("Operating system: Windows w/ WSL (%s)\n", wsl_distribution);}
    else {printf("Operating system: Linux\n");}
#else
    printf("Operating system: Unknown\n");
#endif

#if defined(__aarch64__) || defined(__arm64__)
    printf("Architecture: ARM64\n");
#elif defined(__x86_64__)
    printf("Architecture: x86-64\n");
#else
    printf("Architecture: Unknown\n");
#endif

#if defined(__clang__) && defined(__apple_build_version__)
    printf("Compiler: Apple Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__clang__)
    printf("Compiler: Clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
    printf("Compiler: GCC %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#else
    printf("Compiler: Unknown\n");
#endif

    long logical_cpu_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (logical_cpu_cores > 0) {printf("Logical CPU cores: %ld\n", logical_cpu_cores);}
    else {printf("Logical CPU cores: Unavailable\n");}
    printf("\n");
}

void require(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "Validation failed: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

void *allocate(size_t bytes) {
    void *memory = malloc(bytes);
    if (memory == NULL) {
        fprintf(stderr, "Unable to allocate %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return memory;
}

double elapsed_since(double start) {
    double elapsed = benchmark_time_seconds() - start;
    require(elapsed > 0.0, "timer interval must be positive");
    return elapsed;
}

double smaller(double a, double b) {
    return a < b ? a : b;
}

double initialize(float *data, size_t length) {
    double sum = 0.0;
    for (size_t i = 0; i < length; ++i) {
        data[i] = (float)((int)(i % 31) - 15) / 16.0f;
        sum += data[i];
    }
    return sum;
}

void check_sum(double actual, double expected) {
    require(isfinite(actual) && fabs(actual - expected) <=
            1e-9 * (1.0 + fabs(expected)), "sum mismatch");
}

void check_matmul(const float *a, const float *b, const float *c,
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

void print_matmul(size_t n, const char *version, double seconds) {
    double operations = 2.0 * n * n * n;
    printf("n=%zu %-22s %.9f s  %.6f GFLOP/s  %.9f TFLOP/s  PASS\n",
           n, version, seconds, operations / seconds / 1e9,
           operations / seconds / 1e12);
}
