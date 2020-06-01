//-------------------------------------------
// Binary Tree header
//-------------------------------------------
#ifndef _BINARY_TREE_H
#define _BINARY_TREE_H

#include <stdbool.h>

typedef struct node
{
    int key;
    struct node *left;
    struct node *right;
} node_t;

// Delete key from the tree
void delete(node_t** root, int key);

// Insert a new key
void insert(node_t** root, int key);

// Check if a key is in the tree
bool lookup(node_t* root, int key);

#endif
