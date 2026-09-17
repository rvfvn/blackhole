#!/usr/bin/env sh
# The one place that defines how this lab is compiled.
set -eu
cd "$(dirname "$0")"

usage() {
    cat <<'MENU'
Usage: ./build.sh NUMBER
  1  Memory access: rows, columns, stride, and caches
  2  Naive matrix multiplication and FLOP/s
  3  Transposed-B matrix multiplication comparison
  all       Run all three experiments
  assembly  Generate lab1.s, lab2.s, lab3.s for inspection
  --help    Show this menu
MENU
}

if [ "$#" -eq 0 ]; then usage; exit 0; fi
if [ "$#" -ne 1 ]; then usage >&2; exit 1; fi
case "$1" in
    --help|-h) usage; exit 0 ;;
    1|2|3|all|assembly) ;;
    *) usage >&2; exit 1 ;;
esac

# Keep the timed functions as calls when they share a source file with the
# driver. This also makes their loops easier to locate in generated assembly.
lab_selection=$1
if [ "$lab_selection" = assembly ]; then
    for lab_source in lab1.c lab2.c lab3.c; do
        cc -std=c11 -O2 -fno-inline -Wall -Wextra -pedantic -I. \
            -S "$lab_source" -o "${lab_source%.c}.s"
    done
    echo 'Assembly written to lab1.s, lab2.s, and lab3.s'
else
    if [ "$lab_selection" = all ]; then
        lab_numbers="1 2 3"
    else
        lab_numbers=$lab_selection
    fi
    for lab_number in $lab_numbers; do
        cc -std=c11 -O2 -fno-inline -Wall -Wextra -pedantic -I. \
            "lab${lab_number}.c" lib/bench.c -o a.out
        ./a.out
    done
fi
