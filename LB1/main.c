#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define LB32   0x00000001
#define LB64   0x0000000000000001

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
		accumulator |= (_64block >> (64 - IP_table[i])) & LB64;
	}
	return accumulator;
}

int32_t feistel(int32_t vec, int64_t key){
	// E(R) expantion function
	int64_t expanded = 0; // 48 bit
	for(int i = 0; i < 48; i++){
		expanded <<= 1;
		expanded |= (vec >> (32 - feistel_expantion[i])) & LB64; 
	}

	// XOR it with key
	int64_t B = expanded ^ key;

	// S-transform
	int32_t transformed = 0;
	for(int i = 0; i < 8; i++){
		transformed <<= 4;
		char S = B >> ((7 - i) * 6);
		char a = ((S >> 4) & 2) | (S & 1); // 0..3
		char b = S & 0x1E; // 0..15 middle 4 bits of 6-bit S block
		int32_t res = S_[i][a * 16 + b]; // 4-bit value
		transformed |= res; 
	}

	// Post S-box permutation
	int32_t permuted = 0;
	for(int i = 0; i < 32; i++){
		permuted <<= 1;
		permuted |= (transformed >> (32 - P[i])) & LB32; 
	}
	return permuted;
}

void generate_keys(int64_t base_key, int64_t* keys){ 
	// Calculate number of odd bits so that we set the lowest byte bit	
	int64_t prepared_key = 0;
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
			if(cur & 0x01)
				cur &= 0xFE; // !0x01
			else
				cur |= 0x01;
		}
		// set byte back
		prepared_key <<= 8;
		prepared_key |= ((int64_t) cur) & byte_mask;
		//base_key = !(byte_mask << (i * 8)) & base_key | (((int64_t) cur & byte_mask) << (i * 8)); 
	}

	// C0 D0 Permutation
	int32_t C0_block = 0;
	int32_t D0_block = 0;
	for(int i = 0; i < 28; i++){
		C0_block <<= 1;
		C0_block |= (prepared_key >> (64 - C0_table[i])) & LB32;

		D0_block <<= 1;
		D0_block |= (prepared_key >> (64 - D0_table[i])) & LB32;
	}	

	// getting the Citer, Diter vector
	for(int i = 0; i < 16; i++){
		// cyclic left shift
		int shift_times = CD_shift_table[i];
		for(int j = 0; j < shift_times; j++){
			C0_block = (C0_block >> 27) & LB32 | (C0_block << 1) & 0x0fffffff;
			D0_block = (D0_block >> 27) & LB32 | (D0_block << 1) & 0x0fffffff;
		}

		// creating common vector CiDi
		int64_t CiDi = ((int64_t) C0_block << 28) | D0_block;
		// key 48 bit
		int64_t key = 0;
		for(int j = 0; j < 48; j++){
			key <<= 1;
			key |= (CiDi >> (56 - key_table[j])) & LB64;		
		}
		keys[i] = key;
	}
}

void DES(int64_t* block, int64_t block_length, int64_t key, int b_enc, int entrop_on){
	int64_t keys[16];
	// Generating keys
	generate_keys(key, keys);

	for(int k = 0; k < block_length; k++){
		// Initial permutation
		int64_t block_perm = initial_permutation(block[k]);
		int32_t L = block_perm >> 32;
		int32_t R = block_perm;
		int32_t old_R = 0, old_L = 0;

		if(b_enc && entrop_on)
			printf("16 steps per block: ---------------------------------------------------------\n");

		// 16 cycles of feistel transform
		for(int i = 0; i < 16; i++){
			old_L = L;
			old_R = R;
			L = old_R;
			if(b_enc)
			{
				R = old_L ^ feistel(old_R, keys[i]);
				if(entrop_on)
				{
					int64_t fullb = (((int64_t)L & 0x00000000FFFFFFFF) << 32) |
						((int64_t)R & 0x00000000FFFFFFFF);
//					float entrop[8];
//					float avg = 0;
					printf("Hex: %llx\n", fullb);
					int ones_count = 0;
					while(fullb)
					{
						ones_count++;
						fullb &= (fullb - 1);
					}
					float entropy = (float) ones_count / 64.0f;
					printf("Entropy = %f\n", entropy);
//					putc('[', stdout);
//					for(int j = 0; j < 8; j++)
//					{
//						char cur = fullb >> (8 * j);
//						int ones_count = 0;
//						while (cur){
//							ones_count++;
//							cur &= (cur - 1);
//						}
//						entrop[j] = (float) ones_count / 8.0f;
//						avg += entrop[j];
//						printf("%f,", entrop[j]);
//					}
//					avg /= 8;
//					putc(']', stdout);
//					printf(" Avg: %f\n", avg);
				}
			}
			else
				R = old_L ^ feistel(old_R, keys[15 - i]);
		}
		if(entrop_on)
			puts("");
	//	else{ // decryption
	//		for(int i = 15; i >= 0; i--){
	//			old_L = L;
	//			old_R = R;
	//			R = old_L;
	//			L = old_R ^ feistel(old_L, keys[i]);
	//		}
	//	}

		// Final reverse permutation
		int64_t RL_block = ((int64_t)R << 32) | ((int64_t)L & 0x00000000FFFFFFFF);
		int64_t accumulator = 0;
		for(int i = 0; i < 64; i++){
			accumulator <<= 1;
			accumulator |= (RL_block >> (64 - PI[i])) & LB64;
		}
		block[k] = accumulator;
	}
}

void weak_keys_check(){
	int64_t wks[4] = {0x0101010101010101, 0xFEFEFEFEFEFEFEFE, 0x1F1F1F1F0E0E0E0E,0xE0E0E0E0F1F1F1F1};
	for(int i = 0; i < 4; i++)
	{
		// слабкий ключ є таким, що: DES(DES(x)) = x
		int64_t b = 0x1234567890ABCDEF;
		int64_t b1 = b;
		DES(&b1, 1, wks[i], 1, 0);
		if(b == b1) printf("Weak key found: %llX\n", wks[i]);
	}
}

long get_file_size(FILE* fp){
	fseek(fp, 0, SEEK_SET);
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return size;
}

int main(){
	char initial_key1[18] = {0};
	printf("Enter key: ");
	if(scanf("%s", initial_key1) > 0)
	{
		printf("key read successfully: %llx\n", *(int64_t*)initial_key1);
	}
	int64_t initial_key = *(int64_t*)initial_key1;


	FILE* fp_test1 = fopen("./test/test1.txt", "rb");
	if(fp_test1)
	{
		long size = get_file_size(fp_test1);
		char* buf = (char*)malloc(size);
		
		fread(buf, size, 1, fp_test1);

		for(int i = 0; i < (size / 8) * 8; i++)
		{
			putc(buf[i], stdout);
		}
		putc('\n', stdout);
		puts("-----------------------------------------");
		int64_t* start = (int64_t*) buf;

		// Cipher that text
		DES(start, (size / 8), initial_key, 1, 1);
		for(int i = 0; i < (size / 8) * 8; i++)
		{
			putc(buf[i], stdout);
		}
		putc('\n', stdout);
		puts("-----------------------------------------");

		// Decrypt back
		DES(start, (size / 8), initial_key, 0, 0);
		for(int i = 0; i < (size / 8) * 8; i++)
		{
			putc(buf[i], stdout);
		}
		putc('\n', stdout);

		fclose(fp_test1);
		free(buf);
	}
	printf("WEAK KEYS TESTING\n");
	weak_keys_check();

	exit(0);
}
