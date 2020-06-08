//-------------------------------------------
// Priority Queue
//-------------------------------------------

#include <stdbool.h>
#include <stdlib.h>

#include "heap.h"
#include "priority_queue.h"

static int parent(int i) { return ((i - 1) / 2); };

//-------------------------------------------
// Adjust the priority of an item in queue
//-------------------------------------------
void adjust_priority(PriorityQueue *q, int i, int p)
{
    int swap;
    
    if (i < 0 || i > q->num_items)
        return;
    
    *(q->queue + i) = p;
    
    while ( i > 0 && *(q->queue + parent(i)) < *(q->queue + i))
    {
        swap = *(q->queue + parent(i));
        *(q->queue + parent(i)) = *(q->queue + i);
        *(q->queue + i) = swap;
        
        i = parent(i);
    }
}

//-------------------------------------------
// Create an empty priority queue
//-------------------------------------------
PriorityQueue* create()
{
    PriorityQueue *q = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    q->num_items = 0;
    q->size = DEFAULT_QUEUE_SIZE;
    q->queue = (int*)malloc(sizeof(int)*DEFAULT_QUEUE_SIZE);
    
    return q;
}

//-------------------------------------------
// Test whether the queue is empty
//-------------------------------------------
bool empty(PriorityQueue *q)
{
    return q->num_items == 0;
}

//-------------------------------------------
// Insert an item to the queue
//-------------------------------------------
void insert(PriorityQueue *q, int p)
{
    int num_items;
    
    num_items = q->num_items + 1;
    if (q->size == num_items)
    {
        // Need to allocate more size for the queue
        q->size *= 2;
        q->queue = (int*)realloc(q->queue, q->size * sizeof(int));
    }
    
    adjust_priority(q, q->num_items, p);
    q->num_items = num_items;
}

//-------------------------------------------
// Remove the highest priority item from the queue
//-------------------------------------------
int remove_max(PriorityQueue *q)
{
    int max;
    
    if (q->num_items <= 0)
        return -1;
    
    max = *(q->queue);
    *(q->queue) = *(q->queue + q->num_items - 1);
    max_heapify(q->queue, 0, q->num_items);
    q->num_items--;
    
    if (q->num_items < q->size / 2)
    {
        // Reallocate smaller queue to save space
        q->size = (q->size * 2) / 3;
        q->queue = (int*)realloc(q->queue, q->size * sizeof(int));
    }
    
    return max;
}
