#include "../heap.c"

//-------------------------------------------
// Heapsort - O(n*log(n))
// We make (n-1) calls to heapify, each of which takes O(log(n)) time
//-------------------------------------------
void heap_sort(int* array, int length)
{
    int i, swap;
    
    build_heap(array, length);

    for (i = length - 1; i > 0; i--)
    {
        swap = *(array + i);
        *(array + i) = *array;
        *array = swap;
        
        max_heapify(array, 0, i);
    }
}
