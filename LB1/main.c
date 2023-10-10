#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Inverse Initial Permutation Table */
static char PI[] = {
    40,  8, 48, 16, 56, 24, 64, 32, 
    39,  7, 47, 15, 55, 23, 63, 31, 
    38,  6, 46, 14, 54, 22, 62, 30, 
    37,  5, 45, 13, 53, 21, 61, 29, 
    36,  4, 44, 12, 52, 20, 60, 28, 
    35,  3, 43, 11, 51, 19, 59, 27, 
    34,  2, 42, 10, 50, 18, 58, 26, 
    33,  1, 41,  9, 49, 17, 57, 25
};

 /* Post S-Box permutation */
static char P[] = {
    16,  7, 20, 21, 
    29, 12, 28, 17, 
     1, 15, 23, 26, 
     5, 18, 31, 10, 
     2,  8, 24, 14, 
    32, 27,  3,  9, 
    19, 13, 30,  6, 
    22, 11,  4, 25
};

static char S_[8][64] = {{
    /* S1 */
    14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,  
     0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,  
     4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0, 
    15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13
},{
    /* S2 */
    15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,  
     3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,  
     0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15, 
    13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9
},{
    /* S3 */
    10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,  
    13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,  
    13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
     1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12
},{
    /* S4 */
     7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,  
    13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,  
    10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
     3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14
},{
    /* S5 */
     2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9, 
    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6, 
     4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14, 
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3
},{
    /* S6 */
    12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
    10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
     9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
     4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13
},{
    /* S7 */
     4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
     1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
     6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12
},{
    /* S8 */
    13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
     1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
     7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
     2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
}};

static char feistel_expantion[] = {
32, 1, 2, 3, 4, 5,
4, 5, 6, 7, 8, 9,
8, 9, 10, 11, 12, 13,
12, 13, 14, 15, 16, 17,
16, 17, 18, 19, 20, 21,
20, 21, 22, 23, 24, 25,
24, 25, 26, 27, 28, 29,
28, 29, 30, 31, 32, 1
};

static char key_table[] = {14,	17,	11,	24,	1,	5,	3,	28,	15,	6,	21,	10,	23,	19,	12,	4,
26,	8,	16,	7,	27,	20,	13,	2,	41,	52,	31,	37,	47,	55,	30,	40,
51,	45,	33,	48,	44,	49,	39,	56,	34,	53,	46,	42,	50,	36,	29,	32};

static char CD_shift_table[] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

static char IP_table[] = {58, 50, 42, 34, 26, 18, 10, 2, 60, 52, 44, 36, 28, 20, 12, 4, 62, 54, 46, 38, 30, 22, 14, 6, 64, 56, 48, 40, 32, 24, 16, 8,
57, 49, 41, 33, 25, 17, 9, 1, 59, 51, 43, 35, 27, 19, 11, 3,
61, 53, 45, 37, 29, 21, 13, 5, 63, 55, 47, 39, 31, 23, 15, 7};

static char C0_table[28] = {57, 49, 41, 33, 25, 17, 9, 1, 58, 50, 42, 34, 26, 18, 
10, 2, 59, 51, 43, 35, 27, 19, 11, 3, 60, 52, 44, 36};

static char D0_table[28] = {63, 55, 47, 39, 31, 23, 15, 7, 62, 54, 46, 38, 30, 22, 
14, 6, 61, 53, 45, 37, 29, 21, 13, 5, 28, 20, 12, 4};

int64_t initial_permutation(int64_t _64block){
	int64_t accumulator = 0;
	for(int i = 0; i < 64; i++){
		accumulator <<= 1;
		accumulator |= (_64block >> (IP_table[i] - 1)) & 1;
	}
	return accumulator;
}

int32_t feistel(int32_t vec, int64_t key){
	// E(R) expantion function
	int64_t expanded = 0; // 48 bit
	for(int i = 0; i < 48; i++){
		expanded <<= 1;
		expanded |= (vec >> (feistel_expantion[i] - 1)) & 1; 
	}

	// XOR it with key
	int64_t B = expanded ^ key;

	// S-transform
	int32_t transformed = 0;
	for(int i = 0; i < 8; i++){
		char S = B >> ((7 - i) * 6);
		char a = ((S >> 4) & 2) | (S & 1); // 0..3
		char b = S & 0x1E; // 0..15 middle 4 bits of 6-bit S block
		int32_t res = S_[i][a * 16 + b]; // 4-bit value
		transformed |= res << ((7 - i) * 4);
	}

	// Post S-box permutation
	int32_t permuted = 0;
	for(int i = 0; i < 32; i++){
		permuted <<= 1;
		permuted |= (transformed >> (P[i] - 1)) & 1; 
	}
	return permuted;
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
		base_key = !(byte_mask << (i * 8)) & base_key | (((int64_t) cur & byte_mask) << (i * 8)); 
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
		int shift_times = CD_shift_table[i - 1];
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
		key <<= 1;
		key |= (CiDi >> (key_table[i] - 1)) & 1;		
		//key |= (bit << (47 - i));
	}
	return key;
}

int64_t DES(int64_t block, int64_t* keys){
	// Initial permutation
	int64_t block_perm = initial_permutation(block);
	int32_t L = block_perm >> 32;
	int32_t R = block_perm;
	int32_t old_R = 0, old_L = 0;

	// 16 cycles of feistel transform
	for(int i = 0; i < 16; i++){
		old_L = L;
		old_R = R;
		L = old_R;
		R = old_L ^ feistel(old_R, keys[i]);
	}

	// Final reverse permutation
	int64_t RL_block = ((int64_t)R << 32) | ((int64_t)L & 0x00000000FFFFFFFF);
	int64_t accumulator = 0;
	for(int i = 0; i < 64; i++){
		accumulator <<= 1;
		accumulator |= (RL_block >> (PI[i] - 1)) & 1;
	}
	return accumulator;
}

int main(){
	int64_t block = 0x0123456789ABCDEF;
	int64_t initial_key = 0xABCDEFABCDEFABCD;
	int64_t keys[16];

	// Generating keys
	for(int i = 1; i <= 16; i++){
		keys[i - 1] = generate_key(initial_key, i);
	}	

	// Encryption
	int64_t enc = DES(block, keys);
	printf("enc: 0x%llX\n", enc);

	return 0;
}
