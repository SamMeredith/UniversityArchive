//-------------------------------------------
// An AVL tree is a balance binary search tree, that is
// for any node, the difference in depth between its right
// and left subtrees is in {-1, 0, 1}
//-------------------------------------------

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct node Node;
struct node
{
    int key;
    Node *left;
    Node *right;
};

// Delete key from the tree
void delete(Node** root, int key);

// Insert a new key
void insert(Node** root, int key);
static Node* create_node(int key);

// Check if a key is in the tree
bool lookup(Node* root, int key);

static int depth(Node* root);
static int balance_factor(Node* root);
// Definitely helps to draw these first
static void left_left_rotate(Node** root);
static void left_right_rotate(Node** root);
static void right_left_rotate(Node** root);
static void right_right_rotate(Node** root);

// Print method for testing
void print_postorder(Node* node);

//-------------------------------------------
//
//-------------------------------------------
int main()
{
    Node* root = NULL;
    
    // Test insertion - following steps of https://www.youtube.com/watch?v=7m94k2Qhg68
    insert(&root, 43);
    insert(&root, 18);
    insert(&root, 22);
    insert(&root, 9);
    insert(&root, 21);
    insert(&root, 6);
    insert(&root, 8);
    insert(&root, 20);
    insert(&root, 63);
    insert(&root, 50);
    insert(&root, 62);
    insert(&root, 51);
    
    printf("Postorder traversal should be 6 9 8 20 21 18 43 51 63 62 50 22\r\n");
    printf("Postorder traversal is ");
    print_postorder(root);
    printf("\r\n\r\n");
    
    // Test deletion - checked by hand
    delete(&root, 62);
    delete(&root, 51);
    delete(&root, 63);
    delete(&root, 50);
    delete(&root, 43);
    printf("Postorder traversal should be 6 9 8 20 22 21 18\r\n");
    printf("Postorder traversal is ");
    print_postorder(root);
    printf("\r\n");
    
    return 0;
}

//-------------------------------------------
// Delete key from the tree
//-------------------------------------------
void delete(Node** root, int key)
{
    int balance, child_balance = 0;
    Node* current = *root;
    
    if (*root == NULL)
        return;
    
    if (key < (*root)->key)
    {
        delete(&current->left, key);
        child_balance = balance_factor(current->right);
    }
    else if (key > (*root)->key)
    {
        delete(&current->right, key);
        child_balance = balance_factor(current->left);
    }
    else
    {
        if (current->left == NULL && current->right == NULL)
        {
            free(*root);
            *root = NULL;
            return;
        }
        else if (current->left && current->right)
        {
            // Find the inorder predecessor
            Node* predecessor = current->left;
            while (predecessor->right != NULL) predecessor = predecessor->right;
            
            // Copy the value of the predecessor
            current->key = predecessor->key;
            
            // Recursively delete the predecessor
            delete(&current->left, predecessor->key);
            child_balance = balance_factor(current->right);
        }
        else
        {
            *root = (current->left) ? current->left : current->right;
            free(current);
            return;
        }
    }
    
    // Check tree is balanced as recursion resolves
    balance = balance_factor(current);
    if (balance > 1)
    {
        if (child_balance >= 0)
            right_right_rotate(root);
        else
            right_left_rotate(root);
    }
    else if (balance < -1)
    {
        if (child_balance <= 0)
            left_left_rotate(root);
        else
            left_right_rotate(root);
    }
}

//-------------------------------------------
// Insert a new key
//-------------------------------------------
void insert(Node** root, int key)
{
    Node* current = *root;
    int balance, child_balance = 0;
    
    // Insert the new node, recursing down the tree
    if (current == NULL)
    {
        *root = create_node(key);
        return;
    }

    if (key <= current->key)
    {
        if (current->left == NULL) 
            current->left = create_node(key);
        else 
        {
            insert(&current->left, key);
            child_balance = balance_factor(current->left);
        }
    }
    else
    {
        if (current->right == NULL)
            current->right = create_node(key);
        else 
        {
            insert(&current->right, key);
            child_balance = balance_factor(current->right);
        }
    }
    
    // Check tree is balanced as recursion resolves
    balance = balance_factor(current);
    if (balance > 1)
    {
        if (child_balance >= 0)
            right_right_rotate(root);
        else
            right_left_rotate(root);
    }
    else if (balance < -1)
    {
        if (child_balance <= 0)
            left_left_rotate(root);
        else
            left_right_rotate(root);
    }
}

//-------------------------------------------
// Check if a key is in the tree
//-------------------------------------------
bool lookup(Node* root, int key)
{
    if (root == NULL)
        return false;
    
    if (root->key == key)
        return true;
    
    if (key <= root->key)
        return lookup(root->left, key);
    else
        return lookup(root->right, key);
}

//-------------------------------------------
// 
//-------------------------------------------
static int balance_factor(Node* root)
{
    if (root == NULL)
        return 0;
    
    return (depth(root->right) - depth(root->left));
}

//-------------------------------------------
// 
//-------------------------------------------
Node* create_node(int key)
{
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    
    return node;
}

//-------------------------------------------
// 
//-------------------------------------------
static int depth(Node* root)
{
    if (root == NULL)
        return 0;
    
    return fmax(1 + depth(root->left), 1 + depth(root->right));
}

//-------------------------------------------
// Imbalance in the left child of the left subtree
//-------------------------------------------
static void left_left_rotate(Node** root)
{
    if (*root == NULL)
        return;
    
    if ((*root)->left != NULL)
    {
        Node* left = (*root)->left;
        (*root)->left = left->right;
        
        left->right = *root;
        *root = left;
    }
}

//-------------------------------------------
// Imbalance in the right child of the left subtree
//-------------------------------------------
static void left_right_rotate(Node** root)
{
    if (*root == NULL)
        return;
    
    if ((*root)->left->right != NULL)
    {
        Node* left = (*root)->left;
        Node* left_right = left->right;
        
        left->right = left_right->left;
        (*root)->left = left_right->right;
        
        left_right->left = left;
        left_right->right = *root;
        *root = left_right;
    }
}

//-------------------------------------------
// Imbalance in the left child of the right subtree
//-------------------------------------------
static void right_left_rotate(Node** root)
{
    if (*root == NULL)
        return;
    
    if ((*root)->right->left != NULL)
    {
        Node* right = (*root)->right;
        Node* right_left = right->left;
        
        right->left = right_left->right;
        (*root)->right = right_left->left;
        
        right_left->right = right;
        right_left->left = *root;
        *root = right_left;
    }
}

//-------------------------------------------
// Imbalance in the right child of the right subtree
//-------------------------------------------
static void right_right_rotate(Node** root)
{
    if (*root == NULL)
        return;
    
    if ((*root)->right != NULL)
    {
        Node* right = (*root)->right;
        (*root)->right = right->left;
        
        right->left = *root;
        *root = right;
    }
}

//-------------------------------------------
// Print the postorder traversal of the tree
//-------------------------------------------
void print_postorder(Node* node)
{
    if (node == NULL)
        return;
    
    if (node->left != NULL)
        print_postorder(node->left);
    
    if (node->right != NULL)
        print_postorder(node->right);
    
    printf("%d ", node->key);
}
