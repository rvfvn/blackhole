// ======================
// || stride_access.c ||
// ======================


// ||ABSTRACT||
// this program constructs the 'sum_array_with_stride()' function, which helps us understand 
// what happens when a CPU reads from a one-dimensional array (reads the values within it) that 
// are at increasingly larger index intervals ("1" -> 0,1,2,... , "4" -> 0,4,8,12,... , etc.). 
//
// our C code can work with the array elements and its subsequent memory addresses and  
// the CPU handles the reads through its cache lines (data that moves through caches l1, l2, l3 in blocks) 
// and main memory (RAM).
//
// so 'sum_array_with_stride()' reads every array element within the one-dimensional array exactly once,
// however it changes the order in which those elements are reached i.e. the sequence of memory addresses 
// requested by the program changes.
//
// even though we can not necessarily see the cache lines directly move, if we are able to time each stride, we
// will be able to observe the order and spacing through which a program requests data aka its memory access patterns


// ||MEMORY ACCESS PATTERNS||
// the memory hierarchy we need to understand for this lab is the following:
//                           
//   main memory (DRAM)  ----   Largest memory capacity (how much data a memory system can store), 
//         ↓                 |  slowest bandwidth (how much data the memory system can move during a period of time)
//         ↓                 |
//      l3 cache             |
//         ↓                 |
//      l2 cache             |
//         ↓                 |
//      l1 cache             |- Lowest memory capacity, highest bandwidth
//         ↓                 |   
//      CPU registers    ----   
//         ↓
// arithmetic instruction
//
// this matters because the size of the array determines where in the memory hierarchy. 
// ex.:
//  small array = mostly l1 cache behavior
//  medium array = l2 or l3 behavior
//  quite large array = more DRAM traffic
// so our stride benchmark here becomes more grounded when if we test several array sizes!


// ||STRIDES||
// intuitively, this following program reads values within the array while skipping increasingly large portions of it:
//      Let D represent a data array of integers with 10 elements:
//              D = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
//
//      A stride value of "1" accesses:
//              Stride "1" = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
//
//      A stride value of "2" accesses:
//              Stride "2" = 0, 2, 4, 6, 8
//
//      A stride value of "3" accesses:
//              Stride "3" = 0, 3, 6, 9
//
// this demonstrates that a smaller stride value reads nearby values within the array while larger strides 
// make farther jumps between reads
// 
// however, there exists a more technical understanding of strides:
// arrays are stored through consecutive memory addresses, so the address of an element can be understood as
//              'address of D[index] = starting address + index * sizeof(float)'
//
// a 'float' type, here, is commonly 4 bytes. a stride of "1" would then move 4 bytes between reads 
// and a stride of 16 therefore moves 64 bytes. 
// CPU caches move data commonly in 64-byte cache lines, although this is very contingent on the machine a program runs on.
//
// a smaller stride value can reuse more values already inside one cache line, 
// but larger stride values may have to jump into a different cache line for each read and
// make the CPU reach farther through the memory hierarchy (shown in previous section). 
//
// this can be understood via the following:
//
//      Let D begin at the hypothetical memory address "0x1000" (this is in hexadecimal), with each float taking 4 bytes:
//
//      A stride value of "1" requests addresses 4 bytes apart:
//              Stride "1" = 0x1000, 0x1004, 0x1008, 0x100C, ...
//
//      A stride value of "2" requests addresses 8 bytes apart:
//              Stride "2" = 0x1000, 0x1008, 0x1010, 0x1018, ...
//
//      A stride value of "16" requests addresses 64 bytes apart:
//              Stride "16" = 0x1000, 0x1040, 0x1080, 0x10C0, ...
//
//      if a cache line covers 64 bytes from 0x1000 through 0x103F, then D[0] through D[15] can fit inside that line.
//      thus, stride "1" can use those nearby values, while stride "16" can jump from one cache line to the next.
//
// so our final function will still read every element exactly once by making multiple offset passes through a data array.
// this means the data and arithmetic stay the same, but the order and spacing of the memory requests changes!
//      ex.:
//      for an 8-element data array D (instantiated as D[8]) and a stride value of "2":
//              offset pass "0" = D[0], D[2], D[4], D[6]
//              offset pass "1" = D[1], D[3], D[5], D[7]
//      every element is reached once, but the address-request order is now 0, 2, 4, 6, 1, 3, 5, 7.

#include "bench.h"
#include <stddef.h> // --> we include this so that we have access to 'size_t', which is an unsigned type for array lengths,
//                         indexes and memory sizes


double sum_array_with_stride(const float *data, size_t length, size_t stride){
    /* Adds every value in a one-dimensional float array exactly once. The stride changes the order of how the elements in,
    the array are read, but not how many values are read or added.

    Args:
        'data': The address of the first element in the array.
        'length': The total number of float elements stored in the array.
        'stride': The distance, measured in array elements stored in the array.

    Returns:
        The sum of every array element as a 'double'. This sum acts as a checksum that confirms 
        the elements were read and helps prevent the compiler from removing the work.

    Notes:
        1. The function must not allocate memory, print output, select benchmark sizes, or measure time.
           (src/main.c will handle those responsibilities)
        2. 'const' means this function can read the array, but it is not allowed to modify its values.

    Algorithm:
        1. Create a sum that begins at 0.
        2. Begin one offset pass for each starting position from 0 through stride - 1.
        3. During each pass, begin at that offset and advance through the array by 'stride' elements.
        4. Add every reached array value to the sum.
        5. After every element has been reached exactly once, return the sum.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    double sum = 0.0;   // initialize sum to 0.0

    for (size_t i = 0; i < stride; i++){    // iterate through array, starting at the beginning and advancing by stride
        for (size_t j = i; j < length; j += stride){        // iterate through stride elements
            sum += data[j];     // add the value to the sum
        }
    }
    return sum;

    // // temp lines to prevent unused-parameter warnings while the function is still empty.
    // (void)data;
    // (void)length;
    // (void)stride;

    // temporary placeholder only! you should replace '0.0' with the computed sum, however you do that.
    return 0.0;
}
