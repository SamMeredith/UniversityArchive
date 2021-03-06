#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "binary_tree.h"

// Ex 1.
node_t* build123();

// Ex 2.
int size(node_t* node);

// Ex 3.
int max_depth(node_t* node);

// Ex 4.
int min_value(node_t* node);
int max_value(node_t* node);

// Ex 5.
void print_tree(node_t* node);

// Ex 6.
void print_postorder(node_t* node);

// Ex 7.
int has_path_sum(node_t* node, int sum);

// Ex 8.
void print_paths(node_t* node);
void print_paths_recur(node_t* node, int* path, int path_length);

// Ex 9.
void mirror(node_t* node);

// Ex 10.
void double_tree(node_t* node);

// Ex 11.
bool same_tree(node_t* a, node_t* b);

// Ex 12.
int count_trees(int num_keys);

// Ex 13.
bool is_BST(node_t* node);

// Ex 14.
bool is_BST2(node_t* node);
bool is_BST_recur(node_t* node, int min, int max);

int main()
{
    node_t* root = build123();
    
    printf("Tree build123() has size %d\r\n", size(root));
    
    printf("Tree build123() has maximum depth %d\r\n", max_depth(root));
    
    printf("The smallest key in tree build123() is %d, the largest is %d\r\n",
        min_value(root), max_value(root));
    
    printf("The inorder traversal of tree build123() is ");
    print_tree(root);
    printf("\r\n");
    
    printf("The postorder traversal of tree build123() is ");
    print_postorder(root);
    printf("\r\n");
    
    printf("Does tree build123() have a path sum of 4? %d\r\n", has_path_sum(root, 4));
    printf("Does tree build123() have a path sum of 5? %d\r\n", has_path_sum(root, 5));
    
    printf("Tree build123() has tree->leaf paths:\r\n");
    print_paths(root);
    
    mirror(root);
    printf("The postorder traversal of mirror(build123()) is ");
    print_postorder(root);
    printf("\r\n");
    
    double_tree(root);
    printf("The postorder traversal of double_tree(mirror(build123())) is ");
    print_postorder(root);
    printf("\r\n");
    
    node_t* test  = build123();
    mirror(test);
    printf("Is mirror(build123()) the same as our working tree? %d\r\n",
        same_tree(root, test));
    double_tree(test);
    printf("Is double_tree(mirror(build123())) the same as our working tree? %d\r\n",
        same_tree(root, test));
        
    printf("count_trees(4) is %d\r\n", count_trees(4));
    printf("count_trees(5) is %d\r\n", count_trees(5));
    
    printf("Is double_tree(mirror(build123())) a BST? %d\r\n", is_BST(root));
    test = build123();
    double_tree(test);
    printf("Is double_tree(build123()) a BST? %d\r\n", is_BST(test));
    
    printf("Is double_tree(mirror(build123())) a BST2? %d\r\n", is_BST(root));
    test = build123();
    double_tree(test);
    printf("Is double_tree(build123()) a BST2? %d\r\n", is_BST(test));
    
    return 0;
}

//-------------------------------------------
// Build a simple tree
//-------------------------------------------
node_t* build123()
{
    node_t* root = NULL;
    
    insert(&root, 2);
    insert(&root, 1);
    insert(&root, 3);
    
    return root;
}

//-------------------------------------------
// Count the number of nodes in the tree
//-------------------------------------------
int size(node_t* node)
{
    if (node == NULL)
        return 0;
    
    return 1 + size(node->left) + size(node->right);
}

//-------------------------------------------
// Return the length of the longest path from the root
//-------------------------------------------
int max_depth(node_t* node)
{
    if (node == NULL)
        return 0;
    
    return fmax(1 + max_depth(node->left), 1 + max_depth(node->right));
}

//-------------------------------------------
// Return the smallest value in a non-empty tree
//-------------------------------------------
int min_value(node_t* node)
{
    while (node->left != NULL)
        node = node->left;
    
    return node->key;
}

//-------------------------------------------
// Return the largest value in a non-empty tree
//-------------------------------------------
int max_value(node_t* node)
{
    while (node->right != NULL)
        node = node->right;
    
    return node->key;
}

//-------------------------------------------
// Print the inorder traversal of the tree
//-------------------------------------------
void print_tree(node_t* node)
{
    if (node == NULL)
        return;
    
    if (node->left != NULL)
        print_tree(node->left);
    
    printf("%d ", node->key);
    
    if (node->right != NULL)
        print_tree(node->right);
}

//-------------------------------------------
// Print the postorder traversal of the tree
//-------------------------------------------
void print_postorder(node_t* node)
{
    if (node == NULL)
        return;
    
    if (node->left != NULL)
        print_postorder(node->left);
    
    if (node->right != NULL)
        print_postorder(node->right);
    
    printf("%d ", node->key);
}

//-------------------------------------------
// Return true if the sum of keys on any path from the root to a leaf is equal to sum
//-------------------------------------------
int has_path_sum(node_t* node, int sum)
{
    if (node == NULL || sum < 0)
        return false;   
    
    bool has_left_path_sum = node->key == sum;
    if (node->left != NULL)
        has_left_path_sum = has_path_sum(node->left, sum - node->key);
    
    bool has_right_path_sum = node->key == sum;
    if (node->right != NULL)
        has_right_path_sum = has_path_sum(node->right, sum - node->key);
    
    return has_left_path_sum || has_right_path_sum;
}

//-------------------------------------------
// Print all root to leaf paths
//-------------------------------------------
void print_paths(node_t* node)
{
    if (node == NULL)
        return;
    
    int depth = max_depth(node);
    int* path = (int*)malloc(depth*sizeof(int));
    int path_length = 0;
    
    *(path + path_length++) = node->key;
    print_paths_recur(node->left, path, path_length);
    print_paths_recur(node->right, path, path_length);
}

//-------------------------------------------
// Recursive helper function
//-------------------------------------------
void print_paths_recur(node_t* node, int* path, int path_length)
{
    if (node == NULL)
        return;
    
    if (node->left == NULL && node->right == NULL)
    {
        for (int i = 0; i < path_length; i++)
        {
            printf("%d ", *(path + i));
        }
        printf("%d\r\n", node->key);
    }
    else
    {
        *(path + path_length++) = node->key;
        print_paths_recur(node->left, path, path_length);
        print_paths_recur(node->right, path, path_length);
    }
}

//-------------------------------------------
// Mirror the given tree
//-------------------------------------------
void mirror(node_t* node)
{
    if (node == NULL)
        return;
    
    node_t* temp = node->right;
    node->right = node->left;
    node->left = temp;
    
    mirror(node->right);
    mirror(node->left);
}

//-------------------------------------------
// For each node in a binary search tree, create a new duplicate node as the left child of the original node
//-------------------------------------------
void double_tree(node_t* node)
{
    if (node == NULL)
        return;
    
    node_t *new_node = (node_t*)malloc(sizeof(node_t));
    new_node->key = node->key;
    new_node->left = node->left;
    new_node->right = NULL;
    
    node->left = new_node;
    
    double_tree(node->right);
    double_tree(new_node->left);
}

//-------------------------------------------
// Return true if the trees are structurally identical
//-------------------------------------------
bool same_tree(node_t* a, node_t* b)
{
    if (a == NULL && b == NULL)
        return true;
    
    if (a == NULL || b == NULL)
        return false;
    
    return (a->key == b->key && same_tree(a->left, b->left) && same_tree(a->right, b->right));
}

//-------------------------------------------
// Returns the number of structurally unique binary search trees with keys 1,...,num_keys
//-------------------------------------------
int count_trees(int num_keys)
{
    int count = 0;
    
    if (num_keys <= 1)
        return 1;
    
    for (int i = 1; i <= num_keys; i++)
    {
        count += count_trees(i - 1) * count_trees(num_keys - i);
    }
    
    return count;
}

//-------------------------------------------
// Returns true if a tree is a BST
//-------------------------------------------
bool is_BST(node_t* node)
{
    if (node == NULL)
        return true;
    
    // Tree is a BST if, for each node, all nodes left are <= and all nodes right are >
    bool is_left_BST;
    if (node->left != NULL && max_value(node->left) > node->key)
        is_left_BST = false;
    else is_left_BST = is_BST(node->left);
    
    bool is_right_BST;
    if (node->right != NULL && min_value(node->right) <= node->key)
        is_right_BST = false;
    else is_right_BST = is_BST(node->right);
    
    return is_left_BST && is_right_BST;
}

//-------------------------------------------
// Returns true if a tree is a BST
//-------------------------------------------
bool is_BST2(node_t* node)
{
    return is_BST_recur(node, INT_MIN, INT_MAX);
}

//-------------------------------------------
// 
//-------------------------------------------
bool is_BST_recur(node_t* node, int min, int max)
{
    if (node == NULL);
        return true;
    
    if (node->key > max || node->key < min)
        return false;
    
    return is_BST_recur(node->left, min, node->key) && is_BST_recur(node->right, node->key + 1, max);
}