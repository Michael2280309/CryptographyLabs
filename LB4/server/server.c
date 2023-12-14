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

// Modular exponentiation function
uint64_t modExp(uint64_t base, uint64_t exponent, uint64_t modulus) {
    uint64_t result = 1;
    base = base % modulus;
    while (exponent > 0) {
        if (exponent % 2 == 1) {
            result = (result * base) % modulus;
        }
        exponent = exponent >> 1;
        base = (base * base) % modulus;
    }
    return result;
}

// Miller-Rabin primality test
int millerRabinTest(int fd, uint64_t n, int k) {
    // Handle small numbers separately
    if (n <= 1 || n == 4) {
        return 0; // Not prime
    }
    if (n <= 3) {
        return 1; // Prime
    }

    // Find d such that n-1 = 2^r * d
    uint64_t d = n - 1;
    while (d % 2 == 0) {
        d /= 2;
    }

    // Repeat the test k times
    for (int i = 0; i < k; ++i) {
        
        uint64_t a;
        if (read(fd, &a, sizeof(a)) != sizeof(a)) {
            perror("Error reading from /dev/urandom");
            close(fd);
            exit(EXIT_FAILURE);
        }

        // Choose a truly random witness 'a' in the range [2, n-2]
        a = a % (n - 4) + 2;

        // Compute a^d % n
        uint64_t x = modExp(a, d, n);

        if (x == 1 || x == n - 1) {
            continue; // Probably prime
        }

        // Repeat the squaring and checking
        for (uint64_t r = 0; r < d - 1; ++r) {
            x = modExp(x, 2, n);
            if (x == n - 1) {
                break; // Probably prime
            }
        }

        // If no squaring produced -1, then it's definitely composite
        if (x != n - 1) {
            return 0; // Not prime
        }
    }

    return 1; // Probably prime
}

// Функция для генерации простого числа в заданном диапазоне
uint64_t generatePrime(uint64_t min, uint64_t max) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("Error opening /dev/urandom");
        exit(EXIT_FAILURE);
    }

    uint64_t p;
    do {
        if (read(fd, &p, sizeof(p)) != sizeof(p)) {
            perror("Error reading from /dev/urandom");
            close(fd);
            exit(EXIT_FAILURE);
        }
        p = p % (max - min + 1) + min;
    } while (!millerRabinTest(fd, p, 8));

    close(fd);
    return p;
}

uint64_t find_linear(uint64_t n, uint64_t* stack, uint64_t len){
    for(uint64_t i = 0; i < len; i++)
    {
        if(!stack[i]) return -1;
        if(stack[i] == n) return n;
    }

    return -1;
}

uint64_t findTrivialSqrt(uint64_t Q){
    size_t size = sqrt(Q);
    uint64_t* stack = (uint64_t*) malloc(sizeof(uint64_t) * size);
    memset(stack, 0, size * sizeof(uint64_t));
    uint64_t ind = 0;

    uint64_t* insert_count = (uint64_t*) malloc(size);
    memset(stack, 0, size * sizeof(uint64_t));

    for(uint64_t ia = 3; ia < size; ia++){ 
        for(uint64_t s = 0; s < size; s++){ 
            uint64_t ia_smq = npow(ia, s) % Q;

            if(!ia_smq)
                continue;

            if(find_linear(ia_smq, stack, size) == -1)
                stack[ind++] = ia_smq;
        }

        insert_count[ia] = ind;

        ind = 0;    
        memset(stack, 0, size * sizeof(uint64_t));
    }

    uint64_t max_insert = 0, res_a = -1;
    for(uint64_t a = 3; a < size; a++){
        if(insert_count[a] >= max_insert) {
            max_insert = insert_count[a];
            res_a = a;
        }
    }

    // puts("Insertion map: ");
    // for(uint64_t i = 3; i < size; i++){
    //     printf("%llu : %llu\n", i, insert_count[i]);
    // }

    free(stack);
    return res_a;
}

// uint64_t xor_shrink_to64(uint64_t* block128)
// {
//     unsigned char* pb = (unsigned char*) block128;
//     int step = 15;
//     for(int i = 0; i < 8; i++){
//         for(int j = 0; j < step - i; j++){
//             pb[j] ^= pb[j + 1];
//         }
//     }

//     return *block128;
// }

// uint64_t expand128_xor_shrink_to64(uint64_t block64, uint64_t key1, uint64_t key2)
// {
//     uint64_t ah = key1, al = key2;

//     ah ^= (((block64 & LZ64) << 32) + (block64 & LZ64));
//     al ^= ((((block64 & HZ64) >> 32) & LZ64) + (block64 & HZ64));

//     uint64_t block128[] = {ah, al};
//     return xor_shrink_to64(block128);
// }

// uint64_t my_hash_foo(uint64_t block64, uint64_t key1, uint64_t key2) 
// {
//     return expand128_xor_shrink_to64(block64, key1, key2);
// }

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

int server_socket = -1, client_socket = -1;
struct Node* head = NULL; // Initialize an empty linked list

// Global variable to indicate whether a termination signal is received
volatile sig_atomic_t termination_requested = 0;

// Signal handler function
void handle_signal(int signum) {
    if (signum == SIGINT) {
        printf("\nReceived Ctrl+C. Cleaning up...\n");

        // Set the termination flag
        termination_requested = 1;
        if(server_socket != -1) close(server_socket);
        if(client_socket != -1) close(client_socket);
        freeList(&head);
        puts("Exiting...");
        exit(0);
    }
}

void notifyChat(struct Node** head, char* msg){
    struct Node* current = *head;
    while(current){
        int tcp = current -> tcp_conn;
        send(tcp, msg, BUFFER_SIZE, 0);
        current = current -> next;
    }
}

int main(){
    // Set up the signal handler
    signal(SIGINT, handle_signal);
    char buffer[BUFFER_SIZE];

    puts("Generating prime Q...");
    uint64_t Q = generatePrime(10e3, 10e6);
    printf("Q = %llu\n", Q);
    puts("Searching for trivial root A...");
    uint64_t A = findTrivialSqrt(Q);
    printf("A = %llu\n", A);

    struct sockaddr_in server_addr, client_addr;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1");

    // Привязка сокета к адресу и порту
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Перевод сокета в режим прослушивания
    if (listen(server_socket, 10) == -1) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    socklen_t client_addr_len = sizeof(client_addr);

    while(client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) {
        if(client_socket < 0) {
            perror("Bad return from accept");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        size_t len = recv(client_socket, buffer, BUFFER_SIZE, 0);
        buffer[len] = 0;
        if(!strcmp(buffer, "start")){ // new client, give it algorithm parameters
            memset(buffer, 0, BUFFER_SIZE);
            sprintf(buffer, "Q = %llu A = %llu", Q, A);
            send(client_socket, buffer, BUFFER_SIZE, 0);
            close(client_socket);
            continue;
        }
        else if(!strncmp(buffer, "Y", 1)){ // client sending public Yc key
            uint64_t clientY, hash;
            int parsed = sscanf(buffer, "Y = %llu HASH = %llx", &clientY, &hash);
            if(parsed != 2){
                puts("Error parsing arguments");
                close(client_socket);
                continue;
            }
            else{
                if(my_hash_foo(clientY, HASHK1, HASHK2) == hash){ // secure from man-in-the-middle
                    uint64_t serverX = getTrulyRand64() % Q;
                    uint64_t serverY = npow(A, serverX) % Q;
                    uint64_t commonK = npow(clientY, serverX) % Q; // common key K
                    memset(buffer, 0, BUFFER_SIZE);
                    sprintf(buffer, "Y = %llu hash = %llx", serverY, my_hash_foo(serverY, HASHK1, HASHK2));
                    // session: save connection
                    insertAtEnd(&head, commonK, client_socket);
                    send(client_socket, buffer, BUFFER_SIZE, 0);
                    //close(client_socket);
                    continue;
                }
                else{
                    puts("Incorrect hash");
                    close(client_socket);
                    continue;
                }
            }
        }
        else if(!strncmp(buffer, "m", 1)){ // request to send message at chat
            char message[BUFFER_SIZE];
            memset(message, 0, BUFFER_SIZE);
            strcpy(message, buffer + 2); // skipping m+space
            uint64_t* msgptr = (uint64_t*) message;
            struct Node* clientData = findByTCP(&head, client_socket);
            if(!clientData){
                puts("Client not authorized");
                continue;
            }
            for(int i = 0; i < BUFFER_SIZE / 64; i++){
                msgptr[i] ^= clientData->commonKey; // decipher, simplest vigenere-like method
            }
            char chat_msg[BUFFER_SIZE];
            memset(chat_msg, 0, BUFFER_SIZE);
            strcpy(chat_msg, "[Anon]: ");
            strcat(chat_msg, message);
            
            notifyChat(&head, chat_msg);
        }
        else if(!strcmp(buffer, "end")){ // disconnect
            struct Node* current = findByTCP(&head, client_socket);
            if(current){
                close(current->tcp_conn);
                removeNode(&head, client_socket);
            }
        }
    }

    // Finalize due to SIGINT caught
    close(server_socket);
    freeList(&head);
}