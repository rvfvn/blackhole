// ======================
// || matmul_naive.c ||
// ======================


// ||ABSTRACT||
// this program constructs the 'matmul_naive()' function, which multiplies two square float matrices A and B and
// writes their result into a third matrix C. "naive"ness in this case really just means that we are writing
// the mathematical formula and application of the matmul directly through three loops before introducing any rearrangement or blocking.
//
// matmuls are the fundemental bridge (imo really) between mathematics, machine instructions and data movement !
// the CPU repeatedly multiplies values, adds them into an accumulator and loads those values from specific addresses !
// and this repeated multiply-and-add pattern is also the core foundations to so many machine learning and ai accelerator operations.
// (like tenstorrent's blackhole cards!)


// ||WHAT MATRIX MULTIPLICATION COMPUTES||
// its going to get quite math heavy, theres a good visualization and slide on matmuls in 
// labs/01 - cpu execution/CPU-Execution-Lab-Session-1.pptx.pdf :))
//
// for every output position C[i,j], we pair one complete row from A with one complete column from B:
//              C[i,j] = A[i,0] * B[0,j] + A[i,1] * B[1,j] + ... + A[i,n-1] * B[n-1,j]
//
// the same relationship is often written more compactly as:
//              C[i,j] = sum over k of A[i,k] * B[k,j]
//
// ex.:
//      let A and B both be 2 x 2 matrices:
//              A = [ 1, 2 ]          B = [ 5, 6 ]
//                  [ 3, 4 ]              [ 7, 8 ]
//
//      the top-left output value is:
//              C[0,0] = 1 * 5 + 2 * 7 = 19
//
//      and the completed result is:
//              C = [ 19, 22 ]
//                  [ 43, 50 ]


// ||FLAT MATRIX STORAGE||
// A, B and C are the declared matrices here, yes, but each one is stored through a flat row-major float array, kind of like 
// what was described in src/row_col_access.c 
// and so, 
//
// for square matrices with side length 'n', a row and column coordinate becomes a flat index through:
//              matrix[row * n + column]
//
// therefore, the three values involved in one multiply-and-add are reached through:
//              A[i,k] = a[i * n + k]
//              B[k,j] = b[k * n + j]
//              C[i,j] = c[i * n + j]
//
// the lowercase names represent the actual flat C arrays, while the uppercase names help us discuss the matrices.


// ||THE THREE LOOPS||
// each loop answers one part of the computation:
//              loop "i" chooses an output row
//              loop "j" chooses an output column
//              loop "k" walks across A's row and down B's column to build one output value
//
// with the loop order i -> j -> k, the function completely calculates one C[i,j] value before moving to the next
// output position. an accumulator begins at 0.0f, receives each A[i,k] * B[k,j] product and is finally stored in C.


// ||NAIVE MEMORY ACCESS PATTERN||
// while 'k' changes, A[i,k] moves through neighboring values in one row:
//              a[i * n + 0], a[i * n + 1], a[i * n + 2], ...
//
// B[k,j], however, moves down one column of a row-major matrix:
//              b[0 * n + j], b[1 * n + j], b[2 * n + j], ...
//
// keep in mind that these B reads are 'n' float elements apart, or approximately:
//                          'n * sizeof(float)' bytes apart. 
//
// even though the arithmetic is regular, the CPU may have a harder time reusing each cache line while moving down B's columns. 
// and so this memory access pattern will be later explored in the transposed version (src/matmul_transposed.c), where we
// will reconsider while keeping the matrix multiplication itself equal.


// ||COUNTING THE COMPUTE||
// an n x n result contains n * n output values. each output performs n multiplications and approximately n additions,
// which gives us about:
//              floating-point operations = 2 * n * n * n
//
// finally, src/main.c then can divide that operation count by elapsed seconds and report 
// GFLOP/s (giga floating-point operations per second). 


#include "bench.h"
#include <stddef.h> 


void matmul_naive(const float *a, const float *b, float *c, size_t n){
    /* Multiplies two square n x n float matrices A and B, then writes the completed matrix result into C.

    Args:
        'a': The address of the first element in the flat, row-major input matrix A.
        'b': The address of the first element in the flat, row-major input matrix B.
        'c': The address of the first element in the flat, row-major output matrix C.
        'n': The number of rows and columns in each square matrix.

    Returns:
        Nothing. The function writes every calculated output value directly into matrix C.

    Notes:
        1. A, B and C must each contain space for at least 'n * n' floats.
        2. Matrix C must be separate from A and B so writing an output does not overwrite an input value still needed later.
        3. The function must not allocate memory, initialize matrices, print output, choose sizes, measure time, or calculate GFLOP/s.
           (src/main.c will handle those responsibilities)
        4. 'const' means this function can read A and B, but it is not allowed to modify their values.
        5. If 'n' is 0, the function should perform no reads, writes or arithmetic and simply finish.

    Algorithm:
        1. Begin with output row 'i' equal to 0 and move through all n output rows.
        2. For each row, begin with output column 'j' equal to 0 and move through all n output columns.
        3. Create a float accumulator for C[i,j] that begins at 0.0f.
        4. Move 'k' from 0 through n - 1 and add 'a[i * n + k] * b[k * n + j]' into the accumulator.
        5. Store the completed accumulator in 'c[i * n + j]'.
        6. Finish after every output position in C has been calculated exactly once.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    for (size_t i = 0; i < n; i++){
        for (size_t j = 0; j < n; j++){

            c[i * n + j] = 0.0;   // initialize accumulator to 0.0

            for (size_t k = 0; k < n; k++){
                c[i * n + j] += a[i * n + k] * b[k * n + j];   // add value to accumulator
            }
        }
    }

    // // temp lines to prevent unused-parameter warnings while the function is still empty.
    // (void)a;
    // (void)b;
    // (void)c;
    // (void)n;
}
