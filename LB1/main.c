#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char IP_table[] = {58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7};

int64_t initial_permutation(int64_t _64block){
	int64_t accumulator = 0;
	for(int i = 63; i >= 0; i--){
		char offset = i - IP_table[abs(i - 63)] + 1;
		int64_t bit = _64block & (0x01 << (IP_table[abs(i - 63)] - 1));
		if (offset > 0) bit = bit << offset;
		else if(offset < 0) bit = bit >> (-offset);
		accumulator |= bit;
	}
}

int main(){
	char block[8] = "hello_w";
	char key[8] = "secret_";
	
	return 0;
}
