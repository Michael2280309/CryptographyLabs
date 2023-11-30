#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../rsa/rsa.h"
#include "../net/net.h"
#include <time.h>
#define BUFF 512

typedef enum {false, true} bool;

int main (void) {
	srand(time(0));
    const int conn = connect_net("127.0.0.1:8080");
    if (conn < 0) {
        fprintf(stderr, "Error: connect_net\n");
        return conn;
    }

    char buffer[BUFF];
	uint64_t e, d, n;
	rsa_key_gen(&e, &d, &n);

	int len = sprintf(buffer, "%llu %llu", e, n);
	if (len < 0) 
	{
		fprintf(stderr, "Error: formatting numbers\n");
		return 3;
	}
	send_net(conn, buffer, BUFF);
	memset(buffer, 0, BUFF);

	recv_net(conn, buffer, BUFF);
	uint64_t test_block_encr = atoll(buffer);
	uint64_t test_block = rsa_decrypt(test_block_encr, d, n);
	len = sprintf(buffer, "%llu", test_block);
	if (len < 0) 
	{
		fprintf(stderr, "Error: formatting test block\n");
		return 3;
	} 
	send_net(conn, buffer, BUFF);
	memset(buffer, 0, BUFF);

	recv_net(conn, buffer, BUFF);
	uint64_t se = atoll(buffer);
	int i = 0;
	for(; buffer[i] != ' '; i++);
	i++;
	uint64_t sn = atoll(buffer + i);
	uint64_t test_en = die();
	uint64_t test_en_out = rsa_encrypt(test_en, se, sn);
	memset(buffer, 0, BUFF);
	len = sprintf(buffer, "%llu", test_en_out);
	if (len < 0) 
	{
		fprintf(stderr, "Error: formatting test block\n");
		return 3;
	}
	send_net(conn, buffer, BUFF);
	memset(buffer,0,BUFF);
	recv_net(conn, buffer, BUFF);
	uint64_t got_test = atoll(buffer);
	if(got_test == test_en)
	{
		char* succ = "Successful server authentication!";
		printf("Client says: %s\n", succ);
	}
	else
	{
		char* fail = "Server authentication failed!";
		printf("Client says: %s\n", fail);
		close_net(conn);
	}
	
    close_net(conn);

    return 0;
}
