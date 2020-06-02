//-------------------------------------------
// Stanford CS 'Great Tree-List Recursion' problem
// Write a recursive function tree_to_list(Node root) that takes an ordered
// binary tree and rearranges the internal pointers to make a circular
// doubly linked list out of the tree nodes.
//
// http://cslibrary.stanford.edu/109/TreeListRecursion.pdf
//-------------------------------------------

#include <stdlib.h>
#include <stdio.h>

typedef struct node Node;
struct node
{
    int key;
    Node *left;
    Node *right;
};

// The problem function
Node* tree_to_list(Node* root);

// Helper function to append circular buffers
Node* append(Node* a, Node* b); 

// Tree insert functions for testing
void insert(Node** root, int key);
Node* new_node(int key);

// Print function for testing
void print_list(Node* head);

//-------------------------------------------
//
//-------------------------------------------
int main() 
{
    Node* root = NULL;
    insert(&root, 4);
    insert(&root, 2);
    insert(&root, 1);
    insert(&root, 3);
    insert(&root, 5);
    
    Node* head = tree_to_list(root);
    
    print_list(head);
    
    return 0;
}

//-------------------------------------------
// 
//-------------------------------------------
Node* append(Node* a, Node* b)
{   
    if (a == NULL) return b;
    if (b == NULL) return a;
    
    Node *tail_a, *tail_b;
    
    tail_a = a->left;
    tail_b = b->left;
    
    tail_a->right = b;
    b->left = tail_a;
    
    tail_b->right = a;
    a->left = tail_b;
    
    return a;        
}

//-------------------------------------------
// 
//-------------------------------------------
Node* tree_to_list(Node* root)
{
    // The strategy is to make an inorder traversal of the tree
    // returning the head of the sub-list at each recursive step       
    if (root == NULL)
        return NULL;
    
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = root->key;
    node->left = node;
    node->right = node;
    
    if (root->left != NULL)
        node = append(tree_to_list(root->left), node);
        
    if (root->right != NULL)
        node = append(node, tree_to_list(root->right));
        
    return node;
}

//-------------------------------------------
// 
//-------------------------------------------
void insert(Node** root, int key)
{
    if (*root == NULL)
    {
        *root = new_node(key);
        return;
    }

    Node* node = *root;

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
Node* new_node(int key)
{
    Node *node = (Node*)malloc(sizeof(Node));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

//-------------------------------------------
// 
//-------------------------------------------
void print_list(Node* head)
{    
    Node* current = head;
    
    while(current != NULL)
    {        
        printf("%d ", current->key);
        current = current->right;
        if (current == head) break;
    }
}