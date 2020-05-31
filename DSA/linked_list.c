#include <stdbool.h>
#include <stdio.h>

struct node
{
    int data;
    struct node* next;
};

void append(struct node** head, int data);
void delete(struct node** head, int data);
void insert_after(struct node** head, struct node* prev_node, int data);
void print_list(struct node** head);
void push(struct node** head, int data);
bool search(struct node** head, int data, struct node* found_node);

//-------------------------------------------
// 
//-------------------------------------------
int main()
{
    struct node* head = NULL;
    
    head = (struct node*)malloc(sizeof(struct node));
    head->data = 0;
    head->next = NULL;
    
    push(&head, 3);
    append(&head, 2);
    
    print_list(&head);
    
    delete(&head, 3);
    
    print_list(&head);
    
    return 0;
}

//-------------------------------------------
// Add a new node to the end of a linked list
//-------------------------------------------
void append(struct node** head, int data)
{
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    new_node->data = data;
    new_node->next = NULL;
    
    if (*head == NULL)
    {
        *head = new_node;
        return;
    }

    struct node* last = *head;
    
    // Traverse the list
    while (last->next != NULL)
        last = last->next;
    
    last->next = new_node;
}


//-------------------------------------------
// 
//-------------------------------------------
void delete(struct node** head, int data)
{    
    struct node* temp = *head;
    
    if (temp != NULL && temp->data == data)
    {
        *head = temp->next;
        free(temp);
        return;
    }
    
    struct node* prev;
    
    while (temp != NULL && temp->data != data)
    {
        prev = temp;
        temp = temp->next;
    } 
    
    if (temp == NULL)
        return;
    
    prev->next = temp->next;
    free(temp);
}

//-------------------------------------------
// Add a new node after a given node
//-------------------------------------------
void insert_after(struct node** head, struct node* prev_node, int data)
{
    if (prev_node == NULL)
    {
        return;
    }
    
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

//-------------------------------------------
// Print a linked list
//-------------------------------------------
void print_list(struct node** head)
{
    struct node* node = *head;
    
    printf("\r\n");
    while (node != NULL)
    {
        printf("%d ", node->data);
        node = node->next;
    }
}

//-------------------------------------------
// Add a new node to the beginning of a linked list
//-------------------------------------------
void push(struct node** head, int data)
{
    struct node* new_node = (struct node*)malloc(sizeof(struct node));
    new_node->data = data;
    new_node->next = *head;
    
    *head = new_node;
}

//-------------------------------------------
// Find the first node with a given key
//-------------------------------------------
bool search(struct node** head, int data, struct node** found_node)
{
    struct node* temp = *head;
    
    while (temp != NULL)
    {
        if (temp->data == data)
        {
            found_node = &temp;
            return true;
        }
        temp = temp->next;
    }
    
    return false;
}