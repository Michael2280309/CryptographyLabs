#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <tgmath.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "../linked-list.h"
#include "../hashfoo.h"

#define HASHK1 0xab3b74c66cd32646
#define HASHK2 0x4737d542d8e56a4c
#define LZ64 0x00000000FFFFFFFF
#define HZ64 0xFFFFFFFF00000000
#define PORT 8080
#define BUFFER_SIZE 1024

uint64_t npow(uint64_t n, uint64_t s) {
    uint64_t result = 1;
    while (s > 0) {
        if (s % 2 == 1) {
            result *= n;
        }
        n *= n;
        s /= 2;
    }
    return result;
}

uint64_t getTrulyRand64() {
    int randomData = open("/dev/urandom", O_RDONLY);
    if (randomData < 0) {
        perror("Error opening /dev/urandom");
        exit(EXIT_FAILURE);
    }

    uint64_t randValue;
    if (read(randomData, &randValue, sizeof(randValue)) < 0) {
        perror("Error reading /dev/urandom");
        close(randomData);
        exit(EXIT_FAILURE);
    }

    close(randomData);
    return randValue;
}

int main(){
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Создание сокета
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Заполнение структуры sockaddr_in для сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // starting conversation
    strcpy(buffer, "start");
    send(client_socket, buffer, BUFFER_SIZE, 0);

    // retreiving Q, A
    uint64_t Q, A;
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    int parsed = sscanf(buffer, "Q = %llu A = %llu", &Q, &A);
    if(parsed != 2){
        puts("Error in response format: Q and A required.");
        close(client_socket);
        exit(0);
    }
    close(client_socket);

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        exit(EXIT_FAILURE);
    }

    // caclulate Y and send it hashed
    uint64_t Y, X, hash;
    X = getTrulyRand64() % Q;
    Y = npow(A, X) % Q;
    hash = my_hash_foo(Y, HASHK1, HASHK2);
    memset(buffer, 0, BUFFER_SIZE);
    sprintf(buffer, "Y = %llu HASH = %llx", Y, hash);
    send(client_socket, buffer, BUFFER_SIZE, 0);

    // get server's Y
    uint64_t serverY, serverhash, commonKey;
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    parsed = sscanf(buffer, "Y = %llu hash = %llx", &serverY, &serverhash);
    if(my_hash_foo(serverY, HASHK1, HASHK2) == serverhash){ // secure from man-in-the-middle
        commonKey = npow(serverY, X) % Q;
    }
    else{
        puts("Incorrect hash");
        close(client_socket);
        exit(0);
    }

    // send test message
    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, "m helloMishaItIsgoingToBeEncrypted");
    uint64_t* p64 = (uint64_t*) buffer;
    for(int i = 0; i < BUFFER_SIZE / 2 / 64; i++){
        p64[i] ^= commonKey; // encipher, simplest vigenere-like method
    }
    send(client_socket, buffer, BUFFER_SIZE, 0);
    
    // notification from server
    memset(buffer, 0, BUFFER_SIZE);
    size_t n = recv(client_socket, buffer, BUFFER_SIZE, 0);
    puts(buffer);

    memset(buffer, 0, BUFFER_SIZE);
    strcpy(buffer, "end");
    send(client_socket, buffer, BUFFER_SIZE, 0);

    close(client_socket);
}