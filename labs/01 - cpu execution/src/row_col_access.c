// =======================
// || row_col_access.c ||
// =======================


// ||ABSTRACT||
// this program constructs the 'sum_matrix_by_rows()' and 'sum_matrix_by_columns()' functions, which help us
// understand the relationship between how matrix's layout is stored in memory and the order of the loops used to read the matrix itself.
//
// both functions will read every matrix element exactly once and perform the same number of additions/arithmetic. 
// however, the sequence of indexes and memory addresses requested by each function changes only because of the fact that 
// we decide whether:
//      the row loop 
// or 
//      column loop 
// moves first!
//
// our matrix will remain stored in row-major order for BOTH functions. 
// 'sum_matrix_by_columns()' does NOT change the matrix into column-major storage; 
// it only changes the order through which the existing matrix is read.


// ||MATRIX STORAGE IN C||
// even though we understand a matrix as rows and columns, this lab stores its values in one flat float array.
// this can be understood through the following:
//
//      Let M represent a matrix with 3 rows and 4 columns:
//              M = [ 0,  1,  2,  3 ]
//                  [ 4,  5,  6,  7 ]
//                  [ 8,  9, 10, 11 ]
//
//      C stores the values consecutively in row-major order:
//              M = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
//
//      which means that an element located at a particular row and column is to be reached through the following:
//              'M[row * columns + column]'
//
//      so for ex.:
//           the value at row "2" and column "1" is reached through:
//                  M[2 * 4 + 1] = M[9] = 9
//
// this index formula remains exactly the same in both functions. only which loop changes first will be different!


// ||ROW VS COLUMN ACCESS||
// 'sum_matrix_by_rows()' reads across one complete row before moving into the next row:
//              row access = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
//
// 'sum_matrix_by_columns()' reads down one complete column before moving into the next column:
//              column access = 0, 4, 8, 1, 5, 9, 2, 6, 10, 3, 7, 11
//
// row access requests neighboring elements that are commonly 4 bytes apart for a float. 
// meanwhile, column access jumps by the number of columns in each row! 
// thus, the byte distance between the column access reads can be understood as:
//              column distance in bytes = columns * sizeof(float)
//
// ex.: 
//      if a matrix has 16 columns and one float is 4 bytes, moving down one column jumps:
//                      16 columns * 4 bytes = 64 bytes
//
// using what we already understand about nearby addresses and cache lines (see src/stride_access.c :P), 
// row access can often reuse values that arrived together. 
// column access may ask for a different cache line with each read when and if one row is wide enough.
// so, this program we are making here focuses on how an ordinary loop-order decision creates that difference in cache lining 
// while the matrix itself stays put!
// mathematically, both sums match, although their changed addition order can create a tiny floating-point rounding difference >.< !


#include "bench.h"
#include <stddef.h> 

double sum_matrix_by_rows(const float *matrix, size_t rows, size_t columns){
    /* Adds every value in a two-dimensional float matrix exactly once by reading across each row before moving
    into the next row.

    Args:
        'matrix': The address of the first element in the flat, row-major matrix.
        'rows': The total number of rows stored in the matrix.
        'columns': The total number of columns stored in each row of the matrix.

    Returns:
        The sum of every matrix element as a 'double'. This sum acts as a checksum that confirms
        the elements were read and helps prevent the compiler from removing the work.

    Notes:
        1. The function must not allocate memory, print output, select benchmark sizes, or measure time.
           (src/main.c will handle those responsibilities)
        2. 'const' means this function can read the matrix, but it is not allowed to modify its values.
        3. The matrix remains stored as one flat, row-major float array.

    Algorithm:
        1. Create a sum that begins at 0.
        2. Begin with the first row and move through the matrix one row at a time.
        3. Within each row, move through every column from left to right.
        4. Reach each value through 'matrix[row * columns + column]' and add it to the sum.
        5. After every matrix element has been reached exactly once, return the sum.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    double sum = 0.0;   // initialize sum to 0.0

    for (size_t row = 0; row < rows; row++) {   // iterate through rows
        for (size_t column = 0; column < columns; column++) {   // iterate through columns
            sum += matrix[row * columns + column];   // index formula
        }
    }

    return sum;

    // temp lines to prevent unused-parameter warnings while the function is still empty.
    (void)matrix;
    (void)rows;
    (void)columns;

    // temporary placeholder only! you should replace '0.0' with the computed sum, however you do that.
    return 0.0;
}


double sum_matrix_by_columns(const float *matrix, size_t rows, size_t columns){
    /* Adds every value in a two-dimensional float matrix exactly once by reading down each column before moving
    into the next column.

    Args:
        'matrix': The address of the first element in the flat, row-major matrix.
        'rows': The total number of rows stored in the matrix.
        'columns': The total number of columns stored in each row of the matrix.

    Returns:
        The sum of every matrix element as a 'double'. This sum acts as a checksum that confirms
        the elements were read and helps prevent the compiler from removing the work.

    Notes:
        1. The function must not allocate memory, print output, select benchmark sizes, or measure time.
           (src/main.c will handle those responsibilities)
        2. 'const' means this function can read the matrix, but it is not allowed to modify its values.
        3. The matrix remains stored as one flat, row-major float array. only the traversal order changes.

    Algorithm:
        1. Create a sum that begins at 0.
        2. Begin with the first column and move through the matrix one column at a time.
        3. Within each column, move through every row from top to bottom.
        4. Reach each value through 'matrix[row * columns + column]' and add it to the sum.
        5. After every matrix element has been reached exactly once, return the sum.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    double sum = 0.0;   // initialize sum to 0.0

    for (size_t column = 0; column < columns; column++) {
        for (size_t row = 0; row < rows; row++) {
            sum += matrix[row * columns + column];
        }
    }

    return sum;

    // temp lines to prevent unused-parameter warnings while the function is still empty.
    (void)matrix;
    (void)rows;
    (void)columns;

    // temporary placeholder only! you should replace '0.0' with the computed sum, however you do that.
    return 0.0;
}
