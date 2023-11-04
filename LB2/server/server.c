#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "../rsa/rsa.h"
#include "../net/net.h"
#define BUFF 512

typedef enum {false, true} bool;

int main (void) {
    int listener = listen_net("127.0.0.1:8080");
	if(listener < 0)
	{
		fprintf(stderr, "Error: %d\n", listener);
		return listener;
	}
	printf("Server is listening ...\n");

    char buffer[BUFF];

	const int conn = accept_net(listener);

	if (conn < 0) {
		fprintf(stderr, "Error: accept_net\n");
		return conn;
	}

	recv_net(conn, buffer, BUFF);
	uint64_t ce = atoll(buffer);
	int i = 0;
	for(; buffer[i] != ' '; i++);
	uint64_t cn = atoll(buffer + i + 1);
	uint64_t test_block = die();
	uint64_t i_test_block = rsa_encrypt(test_block, ce, cn);
	memset(buffer, 0, BUFF);
	int len = sprintf(buffer, "%llu", i_test_block);
	if(len < 0)
	{
		fprintf(stderr, "Error formatting test_block");
		return 4;
	}
	send_net(conn, buffer, BUFF);
	
	memset(buffer, 0, BUFF);
	recv_net(conn, buffer, BUFF);
	uint64_t got_test_block = atoll(buffer);
	
	if(got_test_block == test_block)
	{
		static char* greet = "Successful client authentication!";
		printf("Server says: %s\n", greet);
	}
	else
	{
		char* failure = "Client authentication failed.";
		printf("Server says: %s\n", failure);
		close_net(conn);
	}

	memset(buffer, 0, BUFF);
	uint64_t e,d,n;
	rsa_key_gen(&e, &d, &n);

	len = sprintf(buffer, "%llu %llu", e, n);
	if(len < 0)
	{
		fprintf(stderr, "Error formatting public key");
		return 4;
	}
	send_net(conn, buffer, BUFF);

	memset(buffer, 0, BUFF);
	recv_net(conn, buffer, BUFF);
	uint64_t test_in = atoll(buffer);
	uint64_t test_out = rsa_decrypt(test_in, d, n);
	memset(buffer, 0, BUFF);
	len = sprintf(buffer, "%llu", test_out);
	if(len < 0)
	{
		fprintf(stderr, "Error formatting test block");
		return 4;
	}
	send_net(conn, buffer, BUFF);

	close_net(conn);

    return 0;
}
