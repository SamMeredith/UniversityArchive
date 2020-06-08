//-------------------------------------------
// Priority Queue
//-------------------------------------------

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "heap.h"

#define DEFAULT_QUEUE_SIZE 11

typedef struct queue PriorityQueue;
struct queue
{
    int num_items;
    int size;
    int *queue;
};

// Adjust the priority of an item in queue
void adjust_priority(PriorityQueue *q, int i, int p);
static int parent(int i) { return ((i - 1) / 2); };

// Create an empty priority queue
PriorityQueue* create();

// Test whether the queue is empty
bool empty(PriorityQueue *q);

// Insert an item to the queue
void insert(PriorityQueue *q, int p);

// Remove the highest priority item from the queue
int remove_max(PriorityQueue *q);

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
        // Reallocate smaller queue
        q->size = (q->size * 2) / 3;
        q->queue = (int*)realloc(q->queue, q->size * sizeof(int));
    }
    
    return max;
}

void print_preorder(PriorityQueue *q, int i);

//-------------------------------------------
//
//-------------------------------------------
int main()
{
    int max;
    
    PriorityQueue *q = create();
    
    insert(q, 35);
    insert(q, 26);
    insert(q, 33);
    insert(q, 15);
    insert(q, 24);
    insert(q, 5);
    insert(q, 4);
    insert(q, 12);
    insert(q, 1);
    insert(q, 23);
    insert(q, 21);
    insert(q, 2);
    insert(q, 34);
    print_preorder(q, 0);
    printf("\r\n");
    
    // Test tree from http://pages.cs.wisc.edu/~vernon/cs367/notes/PRIORITY-QAnswers.html#ans2
    /*insert(q, 6);
    insert(q, 40);
    insert(q, 28);
    print_preorder(q, 0);
    printf("\r\n");*/
    
    // Test tree from http://pages.cs.wisc.edu/~vernon/cs367/notes/PRIORITY-QAnswers.html#ans3
    max = remove_max(q);
    max = remove_max(q);
    max = remove_max(q);
    max = remove_max(q);
    print_preorder(q, 0);
    printf("\r\n");
    
    return 0;
}

//-------------------------------------------
//
//-------------------------------------------
void print_preorder(PriorityQueue *q, int i)
{
    if (empty(q) || i < 0 || i > q->num_items)
        return;
    
    printf("%d ", *(q->queue + i));
    
    if (i*2 + 1 < q->num_items)
        print_preorder(q, i*2 + 1);
    
    if (i*2 + 2 < q->num_items)
        print_preorder(q, i*2 + 2);
}
