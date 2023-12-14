#ifndef LINKED_LIST
#define LINKED_LIST

#include <arpa/inet.h>

struct Node {
    uint64_t commonKey;
    int tcp_conn;
    struct Node* next;
};

// Function to create a new node with the given data
struct Node* createNode(uint64_t commonKey, int tcp);

// Function to insert a new node at the end of the linked list
void insertAtEnd(struct Node** head, uint64_t commonKey, int tcp);

struct Node* findByTCP(struct Node** head, int tcp);

// Function to print the elements of the linked list
void printList(struct Node* head);

// Function to free the memory allocated for the linked list
void freeList(struct Node** head);

void removeNode(struct Node** head, int tcp);

#endif