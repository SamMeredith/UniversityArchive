//-------------------------------------------
// A Heap is a complete (left-complete) binary tree based data structure
// A max-heap (min-heap is analagous) has the following ordering:
//      The value of each node is greater than or equal to the value of the parent
//
// A heap can be neatly stored in an array by unraveling it level by level
// This works nicely since the heap is complete
// We can then restate the heap property as A[Parent(i)] >= A[i]
//-------------------------------------------

#include "heap.h"

//-------------------------------------------
// Maintain the heap property
//-------------------------------------------
void max_heapify(int array[], int i, int num_nodes)
{
    int largest, left, right, swap;
    
    left = 2*i + 1;
    right = 2*i + 2;
    largest = i;
    
    if (left < num_nodes && array[left] > array[largest])
        largest = left;
    if (right < num_nodes && array[right] > array[largest])
        largest = right;
    if (largest != i)
    {
        int swap = array[i];
        array[i] = array[largest];
        array[largest] = swap;
        max_heapify(array, largest, num_nodes);
    }
}

//-------------------------------------------
// Build a heap from an array
// This runs (a little suprisingly) in O(n) time
// A good explanation is given here http://www.cs.umd.edu/~meesh/351/mount/lectures/lect14-heapsort-analysis-part.pdf
//-------------------------------------------
void build_heap(int array[], int length)
{
    int i;
    
    // The key observation is that all leaf nodes are stored from n/2 to n/2
    for (i = length / 2 - 1; i >= 0; i--)
    {
        max_heapify(array, i, length);
    }
}
