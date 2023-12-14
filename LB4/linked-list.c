#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>


// Define the structure for a node
struct Node {
    uint64_t commonKey;
    int tcp_conn;
    struct Node* next;
};

struct Node* findByTCP(struct Node** head, int tcp){
    struct Node* current = *head;
    while (current != NULL) {
        if(current->tcp_conn == tcp) return current;
        current = current->next;
    }
    return NULL;
}

// Function to create a new node with the given data
struct Node* createNode(uint64_t commonKey, int tcp) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    newNode->commonKey = commonKey;
    newNode->tcp_conn = tcp;
    newNode->next = NULL;
    return newNode;
}

// Function to insert a new node at the end of the linked list
void insertAtEnd(struct Node** head, uint64_t commonKey, int tcp) {
    struct Node* newNode = createNode(commonKey, tcp);

    if (*head == NULL) {
        // If the list is empty, make the new node the head
        *head = newNode;
    } else {
        // Traverse to the end and append the new node
        struct Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

// Function to print the elements of the linked list
void printList(struct Node* head) {
    struct Node* current = head;
    while (current != NULL) {
        printf("%d - %llu", current->tcp_conn, current->commonKey);
        current = current->next;
    }
    printf("\n");
}

// Function to free the memory allocated for the linked list
void freeList(struct Node** head) {
    if(head == NULL) return;
    struct Node* current = *head;
    struct Node* next;

    while (current != NULL) {
        next = current->next;
        close(current->tcp_conn);
        free(current);
        current = next;
    }

    *head = NULL; // Set the head to NULL after freeing all nodes
}

// Function to remove a node with the specified TCP connection from the linked list
void removeNode(struct Node** head, int tcp) {
    // Find the node with the specified TCP connection
    struct Node* current = *head;
    struct Node* prev = NULL;

    while (current != NULL && current->tcp_conn != tcp) {
        prev = current;
        current = current->next;
    }

    // If the node with the specified TCP connection is found
    if (current != NULL) {
        // If the node is the head of the list
        if (prev == NULL) {
            *head = current->next;
        } else {
            // If the node is not the head, update the next pointer of the previous node
            prev->next = current->next;
        }

        // Free the memory allocated for the removed node
        free(current);
    }
}
