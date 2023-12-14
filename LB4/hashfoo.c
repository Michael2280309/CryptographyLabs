#include <stdint.h>
#define LZ64 0x00000000FFFFFFFF
#define HZ64 0xFFFFFFFF00000000

uint64_t xor_shrink_to64(uint64_t* block128)
{
    unsigned char* pb = (unsigned char*) block128;
    int step = 15;
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < step - i; j++){
            pb[j] ^= pb[j + 1];
        }
    }

    return *block128;
}

uint64_t expand128_xor_shrink_to64(uint64_t block64, uint64_t key1, uint64_t key2)
{
    uint64_t ah = key1, al = key2;

    ah ^= (((block64 & LZ64) << 32) + (block64 & LZ64));
    al ^= ((((block64 & HZ64) >> 32) & LZ64) + (block64 & HZ64));

    uint64_t block128[] = {ah, al};
    return xor_shrink_to64(block128);
}

uint64_t my_hash_foo(uint64_t block64, uint64_t key1, uint64_t key2) 
{
    return expand128_xor_shrink_to64(block64, key1, key2);
}