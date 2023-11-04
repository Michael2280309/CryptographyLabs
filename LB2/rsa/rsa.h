#ifndef RSA_H
#define RSA_H
#include <stdint.h>

#define die() ((rand() % (32768 - 7)) + 7)
void rsa_key_gen(uint64_t* E, uint64_t* D, uint64_t* n);
uint64_t rsa_encrypt(uint64_t a, uint64_t E, uint64_t n); 
uint64_t rsa_decrypt(uint64_t b, uint64_t D, uint64_t n); 

#endif
