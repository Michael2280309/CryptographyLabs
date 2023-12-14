#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080

// Flag to indicate whether a SIGINT signal has been received
volatile sig_atomic_t sigint_received = 0;

// Signal handler function
void handle_sigint(int signum) {
    if (signum == SIGINT) {
        printf("\nReceived SIGINT. Cleaning up...\n");
        sigint_received = 1;
    }
}

void handle_client(int client_socket) {
    char buffer[1024];
    ssize_t bytes_received;

    // Receive and print data from the client
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("Received from client: %s", buffer);

        // Echo the received data back to the client
        send(client_socket, buffer, bytes_received, 0);
    }

    // Check for errors or connection closure
    if (bytes_received == 0) {
        printf("Client disconnected.\n");
    } else if (bytes_received == -1) {
        perror("Error receiving data");
    }

    // Close the client socket
    close(client_socket);
}

int main() {
    // Set up the signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating server socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding server socket");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Error listening for connections");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Main server loop
    while (!sigint_received) {
        // Accept a connection
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            // Check if accept was interrupted by a signal
            if (sigint_received) {
                break; // Break out of the loop if SIGINT is received
            }

            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }

        // Handle the client in a separate function
        handle_client(client_socket);
    }

    // Close the server socket
    close(server_socket);

    printf("Exiting gracefully.\n");

    return 0;
}
