#include <stdbool.h>
#include <stdio.h>

typedef struct n
{
    int key;
    struct n *left;
    struct n *right;
} node_t;

void delete(node_t** root, int key);
void insert(node_t** root, int key);
bool lookup(node_t* root, int key);
node_t* new_node(int key);
void print_tree(node_t* root);

//-------------------------------------------
// 
//-------------------------------------------
int main()
{
    node_t *root = NULL;
    
    root = (node_t*)malloc(sizeof(node_t));
    root->left = NULL;
    root->right = NULL;
    root->key = 10;
    
    insert(&root, 8);
    insert(&root, 9);
    insert(&root, 5);
    insert(&root, 6);
    insert(&root, 11);
    
    print_tree(root);
    printf("\r\n");
    
    delete(&root, 8);
    
    print_tree(root);
    
    return 0;
}

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
        *root = new_node(key);

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

//-------------------------------------------
// 
//-------------------------------------------
void print_tree(node_t* root)
{
    if (root == NULL)
        return;
    
    printf("%d ", root->key);
    
    print_tree(root->left);
    print_tree(root->right);
}
