// ===========================
// || matmul_transposed.c ||
// ===========================


// ||ABSTRACT||
// this program constructs the 'transpose_square_matrix()' and 'matmul_with_transposed_b()' functions. together, they
// keep the same C = A * B matrix multiplication from our naive version in src/matmul_naive.c , but this program 
// rearranges how matrix B is stored before the multiplication begins.
//
// so even though the mathematical operation will not change like the previous examples, 
// we are making a separate transposed copy of B so that values which originally lived down one column can instead 
// be read across one neighboring row. this fundementally gives us a peek at 
// a completley different data layout within the CPU and subsequently a different path through memory!


// ||TRANSPOSING B||
// transposing a matrix swaps its row and column coordinates. this can be understood through the following:
//
//      Let B be a 3 x 3 matrix:
//              B = [ 0, 1, 2 ]
//                  [ 3, 4, 5 ]
//                  [ 6, 7, 8 ]
//
//      Its transposed copy, which we will call B^T, is:
//              B^T = [ 0, 3, 6 ]
//                    [ 1, 4, 7 ]
//                    [ 2, 5, 8 ]
//
//      one value moves according to:
//              B^T[column,row] = B[row,column]
//
//      with our flat row-major arrays, that same relationship becomes:
//              b_transposed[column * n + row] = b[row * n + column]


// ||WHY THE MATMUL READS CHANGE||
// the naive multiplication reaches one value from B through:
//              b[k * n + j]
//
// as 'k' changes, that expression moves down column 'j' and jumps by 'n' float elements between reads.
// after transposing B, the same original values can be reached through:
//              b_transposed[j * n + k]
//
// now, as 'k' changes, both inputs move through neighboring elements inside rows:
//              a[i * n + k]
//              b_transposed[j * n + k]
//
// C[i,j] therefore becomes:
//              C[i,j] = sum over k of a[i * n + k] * b_transposed[j * n + k]
//
// this function still actually computes A * B even though we have declared this all with B^T! 
// B^T, thus, is the rearranged storage used to recover B's original columns more contiguously.


// ||TRANSPOSITION HAS A COST!!||
// to rearrange matrix B is not free. this is because 'transpose_square_matrix()' must read:
//                  n * n floats from B 
// and write 
//                  n * n floats into BT.
//
// src/main.c should create BT before timing the multiplication so we can first isolate the access-pattern difference
// between the two matmul kernels. it can also report the transpose time separately so that cost does not disappear.
//
// if B is used for only one multiplication, the transpose cost matters directly. if the same B is reused many times,
// that one-time cost can be spread across many faster multiplications. this is a first example of a layout decision
// trading preparation work for more convenient data movement during compute.


#include "bench.h"
#include <stddef.h>

void transpose_square_matrix(const float *source, float *destination, size_t n){
    /* Creates a separate transposed copy of one square n x n float matrix.

    Args:
        'source': The address of the first element in the original flat, row-major matrix.
        'destination': The address of the first element where the transposed matrix will be written.
        'n': The number of rows and columns in the square matrix.

    Returns:
        Nothing. The function writes the completed transposed copy directly into the destination array.

    Notes:
        1. Source and destination must each contain space for at least 'n * n' floats.
        2. Source and destination must be separate, non-overlapping arrays; this is not an in-place transpose.
        3. The function must not allocate memory, initialize values, print output, choose sizes, or measure time.
           (src/main.c will handle those responsibilities)
        4. 'const' means this function can read the source matrix, but it is not allowed to modify its values.
        5. If 'n' is 0, the function should perform no reads or writes and simply finish.

    Algorithm:
        1. Move through every source row using a row index.
        2. Within each row, move through every source column using a column index.
        3. Read 'source[row * n + column]'.
        4. Write that value into 'destination[column * n + row]'.
        5. Finish after every source value has been copied into its swapped coordinate exactly once.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    for (size_t row = 0; row < n; row++){
        for (size_t column = 0; column < n; column++){
            destination[column * n + row] = source[row * n + column];   // copy value from source to destination
        }
    }

    // temp lines to prevent unused-parameter warnings while the function is still empty.
    (void)source;
    (void)destination;
    (void)n;
}


void matmul_with_transposed_b(const float *a, const float *b_transposed, float *c, size_t n){
    /* Multiplies square matrices A and B using an already-transposed storage copy of B, then writes A * B into C.

    Args:
        'a': The address of the first element in the flat, row-major input matrix A.
        'b_transposed': The address of the first element in the flat, row-major transposed storage copy of B.
        'c': The address of the first element in the flat, row-major output matrix C.
        'n': The number of rows and columns in each square matrix.

    Returns:
        Nothing. The function writes every calculated output value directly into matrix C.

    Notes:
        1. A, BT and C must each contain space for at least 'n * n' floats.
        2. Matrix C must be separate from A and BT so writing output does not overwrite an input still needed later.
        3. BT must already contain the correct transposed copy of the original matrix B before this function begins.
        4. The function must not transpose B, allocate memory, initialize matrices, print output, measure time, or calculate GFLOP/s.
           (src/main.c will handle those responsibilities)
        5. 'const' means this function can read A and BT, but it is not allowed to modify their values.
        6. If 'n' is 0, the function should perform no reads, writes or arithmetic and simply finish.

    Algorithm:
        1. Begin with output row 'i' equal to 0 and move through all n output rows.
        2. For each row, begin with output column 'j' equal to 0 and move through all n output columns.
        3. Create a float accumulator for C[i,j] that begins at 0.0f.
        4. Move 'k' from 0 through n - 1 and add 'a[i * n + k] * b_transposed[j * n + k]' into the accumulator.
        5. Store the completed accumulator in 'c[i * n + j]'.
        6. Finish after every output position in C has been calculated exactly once.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    for (size_t i = 0; i < n; i++){
        for (size_t j = 0; j < n; j++){
            c[i * n + j] = 0.0;   // initialize accumulator to 0.0

            for (size_t k = 0; k < n; k++){
                c[i * n + j] += a[i * n + k] * b_transposed[j * n + k];   // add value to accumulator
            }

            c[i * n + j] = c[i * n + j];   // store accumulator in C
        }
    }

    // temp lines to prevent unused-parameter warnings while the function is still empty.
    (void)a;
    (void)b_transposed;
    (void)c;
    (void)n;
}
