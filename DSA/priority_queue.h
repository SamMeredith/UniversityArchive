//-------------------------------------------
// Priority Queue function declarations
//-------------------------------------------
#pragma once

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

// Create an empty priority queue
PriorityQueue* create();

// Test whether the queue is empty
bool empty(PriorityQueue *q);

// Insert an item to the queue
void insert(PriorityQueue *q, int p);

// Remove the highest priority item from the queue
int remove_max(PriorityQueue *q);
