#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char key_table[] = {14,	17,	11,	24,	1,	5,	3,	28,	15,	6,	21,	10,	23,	19,	12,	4,
26,	8,	16,	7,	27,	20,	13,	2,	41,	52,	31,	37,	47,	55,	30,	40,
51,	45,	33,	48,	44,	49,	39,	56,	34,	53,	46,	42,	50,	36,	29,	32};

char CD_shift_table[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

char IP_table[] = {58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7};

char C0_table[28] = {57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 
10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36};

char D0_table[28] = {63, 55, 47, 39, 31, 23, 15, 7, 62, 54, 46, 38, 30, 22, 
14, 6, 61, 53, 45, 37, 29, 21, 13, 5, 28, 20, 12, 4};

int64_t initial_permutation(int64_t _64block){
	int64_t accumulator = 0;
	for(int i = 63; i >= 0; i--){
		int64_t bit = (_64block >> (IP_table[abs(i - 63)] - 1)) & 1;
		accumulator |= (bit << i);
	}
	return accumulator;
}

int64_t generate_key(int64_t base_key, uint8_t iteration){ // iteration 1..16
	// Calculate number of odd bits so that we set the highest byte bit	
	int64_t byte_mask = 0x00000000000000FF;
	for(int i = 0; i < 8; i++){
		char cur = base_key >> (i * 8);
		char cur2 = cur;
		int ones_count = 0;
		while (cur2){
			ones_count++;
			cur2 &= (cur2 - 1);
		}
		if (ones_count % 2 == 0){
			if(cur & 0x80){
				// unset the highest bit
				cur &= 0x7F;
			}
			else
				cur |= 0x80;
		}
		
		// set byte back
		base_key = !(byte_mask << (i * 8)) & base_key | ((int64_t) cur << (i * 8)); // will cur be expanded with zeros?
	}

	// C0 D0 Permutation
	int32_t C0_block = 0;
	int32_t D0_block = 0;
	for(int i = 0; i < 28; i++){
		int64_t bit = (base_key >> (C0_table[i] - 1)) & 1;
		C0_block |= (bit << (27 - i));

		bit = (base_key >> (D0_table[i] - 1)) & 1;
		D0_block |= (bit << (27 - i));
	}	

	// getting the Citer, Diter vector
	for(int i = 1; i <= iteration; i++){
		// cyclic left shift
		int shift_times = CD_shift_table[i];
		int32_t C_saved_bits = C0_block >> (27 - shift_times + 1);
		C0_block = (C0_block << shift_times) | C_saved_bits;
		C0_block &= 0x0FFFFFFF; // clearing shifted bound

		int32_t D_saved_bits = D0_block >> (27 - shift_times + 1);
		D0_block = (D0_block << shift_times) | D_saved_bits;
		D0_block &= 0x0FFFFFFF;
	}
	
	// creating common vector CiDi
	int64_t CiDi = ((int64_t) C0_block << 27) | D0_block;
	// key 48 bit
	int64_t key = 0;
	for(int i = 0; i < 48; i++){
		int64_t bit = (CiDi >> (key_table[i] - 1)) & 1;		
		key |= (bit << (47 - i));
	}
	return key;
}

int main(){
	char block[8] = "hello_w";
	char key[8] = "secret_";
		
	return 0;
}
