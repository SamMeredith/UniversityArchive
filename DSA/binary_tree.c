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
    if (*root == NULL)
        return;
    
    node_t *node = *root;
   
    if (node->key == key)
    {
        if (node->left == NULL)
        {
            *root = node->right;
        }
        else if (node->right == NULL)
        {
            *root = node->left;
        }
        else
        {
            // Find the largest node on the left subtree
            node_t *prev_node = node;
            node_t *next_node = node->left;
           
            while (next_node->right != NULL)
            {
                prev_node = next_node;
                next_node = next_node->right;
            }
            
            prev_node->right = next_node->left;
            // Replace the node to be deleted with this node
            next_node->right = node->right;
            next_node->left = node->left;
           
            *root = next_node;
        }
       
        free(node);
    }
    else
        delete((key <= node->key) ? &node->left : &node->right, key);
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
