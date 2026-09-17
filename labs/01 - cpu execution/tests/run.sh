#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")/.."
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/cpu-lab-tests.XXXXXX")
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
cc -std=c11 -O2 -Wall -Wextra -pedantic -Iinclude \
    tests/correctness.c src/stride_access.c src/row_col_access.c \
    src/bandwidth_copy.c src/matmul_naive.c src/matmul_transposed.c \
    src/threaded_sum.c -pthread -o "$test_dir/correctness"
"$test_dir/correctness"
cc -std=c11 -O1 -g -Wall -Wextra -pedantic -fsanitize=address,undefined \
    -Iinclude tests/correctness.c src/stride_access.c src/row_col_access.c \
    src/bandwidth_copy.c src/matmul_naive.c src/matmul_transposed.c \
    src/threaded_sum.c -pthread -o "$test_dir/correctness-sanitized"
"$test_dir/correctness-sanitized"
