#include <stdlib.h>

#include "binary_tree.h"

//-------------------------------------------
// Local function definitions
//-------------------------------------------
node_t* new_node(int key);

//-------------------------------------------
// 
//-------------------------------------------
void delete(node_t** root, int key)
{
    Nnode_t* current = *root;
    
    if (*root == NULL)
        return;
    
    if (key < current->key)
        delete(&current->left, key);
    else if (key > current->key)
        delete(&current->right, key);
    else
    {
        if (current->left == NULL && current->right == NULL)
        {
            free(*root);
            *root = NULL;
        }
        else if (current->left && current->right)
        {
            // Find the inorder predecessor
            node_t* predecessor = current->left;
            while (predecessor->right != NULL) predecessor = predecessor->right;
            
            // Copy the value of the predecessor
            current->key = predecessor->key;
            
            // Recursively delete the predecessor
            delete(&current->left, predecessor->key);
        }
        else
        {
            *root = (current->left) ? current->left : current->right;
            free(current);
        }
    }
}

//-------------------------------------------
// 
//-------------------------------------------
void insert(node_t** root, int key)
{
    if (*root == NULL)
    {
        *root = new_node(key);
        return;
    }

    node_t *node = *root;

    if (key <= node->key)
    {
        if (node->left == NULL) node->left = new_node(key);
        else insert(&node->left, key);
    }
    else
    {
        if (node->right == NULL) node->right = new_node(key);
        else insert(&node->right, key);
    }
}

//-------------------------------------------
// 
//-------------------------------------------
node_t* new_node(int key)
{
    node_t *node = (node_t*)malloc(sizeof(node_t));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

//-------------------------------------------
// 
//-------------------------------------------
bool lookup(node_t* node, int key)
{
    if (node == NULL)
        return false;
    
    if (node->key == key)
        return true;
    
    if (key <= node->key)
        return lookup(node->left, key);
    else
        return lookup(node->right, key);
}
