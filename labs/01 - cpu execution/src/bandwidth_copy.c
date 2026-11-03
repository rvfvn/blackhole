// ========================
// || bandwidth_copy.c ||
// ========================


// ||ABSTRACT||
// this program constructs the 'copy_float_array()' function, which copies every float from one source array into
// a separate destination array i.e. the main work here is simply just moving data!
//
// each copied element requires the CPU to read one float from a 'source' and write that float into a 'destination'. observing the 
// time it take for a large enough copy action helps us observe how quickly your machine can move useful data through the 
// memory hierarchy (see src/stride_access.c).
// that rate is what we will call the:
//                                       effective memory bandwidth !


// ||MEMORY BANDWIDTH||
// memory capacity tells us how much data a memory system can store. memory bandwidth therefore, you can think, tells us:
//             " well... how much data can this program move during a period of time ? "
//
// in a very basic sense, bandwidth can be measure by:
//              bandwidth = bytes moved / elapsed seconds
//
// and, to display a result in decimal gigabytes per second:
//              effective GB/s = useful bytes moved / elapsed seconds / 1000000000.0
//
// note that bandwidth is different from latency!!! this is because:
// - latency asks how long one request takes
// while 
// - bandwidth describes how much data can continue moving over time. 
// this program, here, focuses on the second question :)


// ||ONE READ + ONE WRITE||
// one float is commonly 4 bytes. for every element in some array that is copied, this function performs:
//              source[index]       = one 4-byte read
//              destination[index]  = one 4-byte write
//
// so the useful read-and-write traffic for this benchmark can be counted as:
//              useful bytes moved = 2 * length * sizeof(float)
//
// ex.: 
//      copying an array with 1,000,000 floats gives us:
//                  source read        = 1,000,000 * 4 bytes = 4,000,000 bytes
//                  destination write  = 1,000,000 * 4 bytes = 4,000,000 bytes
//                  useful bytes moved = 8,000,000 bytes
//
// this is a useful accounting model for comparing runs, but it is not a direct measurement of every physical byte
// moved between DRAM and each cache (l1, l2, l3, as you may recall). this is because cache hits, write allocation and compiler choices 
// can influence and change the actual hardware traffic itself!


// ||KEEPING THE COPY REAL||
// 'copy_float_array()' will not calculate a checksum while it copies because that would add extra arithmetic into the
// experiment. this will actually be the job of  src/main.c:
// it will inspect the destination after the elapsed timer ends and confirm that the copied values are correct! 
// using the destination result also helps prevent the compiler from treating the copy as unused work. 
//
// something to note is that C already provides optimized copying tools like for ex. 'memcpy()' !! 
// so... why are we writing this program you may ask?? 
//          in this lab we want to write the element-by-element loop ourselves hehehehe so that we can clearly declare
//              and express each of the read operation from the source array and each write operation into the
//               destination array ! 
//
// if we use '-O2' tag in the terminal when compiling the program, the compiler may be able to 
// actually recognize that there exists this memory copying pattern in the program, and will thus optimize the machine instructions
// around it. pretty neat :P ! like ex.:
//                      lets say that if the compiler catches this memory copy activity, 
//                      it may move several nearby values with one instruction instead of copying on one float value at a time !
//                      and this, is what is known as vectorization
// this example gives us a good look to observe the fact that the compiler actually asked certain things to the CPU to execute 
// so that the machine instructions assembled (no pun intended) fits in neatly and optimally within a program's goal!


#include "bench.h"
#include <stddef.h> 


void copy_float_array(const float *source, float *destination, size_t length){
    /* Copies every float from a source array into the matching position of a separate destination array.

    Args:
        'source': The address of the first float that will be read. This array is not modified.
        'destination': The address of the first float where the copied values will be written.
        'length': The total number of float elements to copy.

    Returns:
        Nothing. The function writes its result directly into the destination array.

    Notes:
        1. The source and destination arrays must each contain space for at least 'length' floats.
        2. The source and destination memory regions must be separate and must not overlap.
        3. The function must not allocate memory, print output, choose benchmark sizes, measure time, or calculate bandwidth.
           (src/main.c will handle those responsibilities)
        4. 'const' means this function can read the source array, but it is not allowed to modify its values.
        5. If 'length' is 0, the function should perform no reads or writes and simply finish.

    Algorithm:
        1. Begin at array index 0.
        2. Read the value stored at 'source[index]'.
        3. Write that value into the matching 'destination[index]'.
        4. Continue one index at a time until 'length' elements have been copied.
        5. Finish without returning a value because the completed copy already lives inside 'destination'.
    */

    // ||YOUR CODE GOES BELOW HERE||:
    for (size_t i = 0; i < length; i++){
        destination[i] = source[i]; // copy the value from source to destination
    }

    // temp lines to prevent unused-parameter warnings while the function is still empty.
    (void)source;
    (void)destination;
    (void)length;
}
