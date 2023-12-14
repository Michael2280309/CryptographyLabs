#ifndef HASH_FOO
#define HASH_FOO

#include <stdint.h>

uint64_t xor_shrink_to64(uint64_t* block128);

uint64_t expand128_xor_shrink_to64(uint64_t block64, uint64_t key1, uint64_t key2);

uint64_t my_hash_foo(uint64_t block64, uint64_t key1, uint64_t key2);

#endif