#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define A 0x410

// Assuming straddling table contains only russian letters,
// in UTF-8 encoding they are represented by two bytes

							//  1234567... column id
char* straddling_utf8 = "АИТЕСНО" 
"БВГДЖЗКЛМП" // 8 row id
"РУФХЦЧШЩЪЫ" // 9
"ЬЭЮЯ"; // 0

int RL[4] = {7, 10, 10, 4}; // Row length in letters
							
int utf8_codepoint_length(const uint8_t byte);
void utf8_encode(uint32_t codepoint, uint8_t utf8_sequence[4], int* length);
uint32_t utf8_decode(const uint8_t* sequence, int length);

int str_utf8_len(const char* str, int byte_len)
{
	int char_count = 0;
	for(int i = 0; i < byte_len; )
	{
		int cplen = utf8_codepoint_length(str[i]);
		char_count++;	
		i += cplen;
	}
	return char_count;
}

uint32_t* straddling_codepoints()
{
	int cp_count = str_utf8_len(straddling_utf8, strlen(straddling_utf8));
	uint32_t* buf = (uint32_t*) calloc(cp_count, 4);
	int j = 0;
	for(int i = 0; i < strlen(straddling_utf8); )
	{
		int cplen = utf8_codepoint_length(straddling_utf8[i]);
		buf[j] = utf8_decode(straddling_utf8 + i, cplen);
		i += cplen;
		j++;
	}
	return buf;
}

int8_t* compress_utf8_chars_to_nums(const char* str_utf8, int len)
{
	int letter_count = str_utf8_len(str_utf8, len);
	int8_t* num_buf = (int8_t*) malloc(letter_count); // maximum letter_count bytes
	memset(num_buf, 0, letter_count);				  // one byte contains 1 or 2 letters (4 bits per digit)
	uint32_t* straddling_cps = straddling_codepoints();
	int straddling_len = str_utf8_len(straddling_utf8, strlen(straddling_utf8));
	int j = 0, shl = 0;

	for(int i = 0; i < len;)
	{
		int cplen = utf8_codepoint_length(str_utf8[i]);
		if(cplen > 0)
		{
			uint32_t cp = utf8_decode(str_utf8 + i, cplen);
			int pos = -1;
			for(int k = 0; k < straddling_len; k++)
				if(straddling_cps[k] == cp)
				{
					pos = k; break;
				}

			if(pos >= 0)
			{
				// Calculate row and col
				int rlc = RL[0]; 
				char row = 0, col = 0;
				if(pos < rlc) {
					row = 0xF;
					col = pos + 1;
				}
				else {
					for(int l = 1; l < sizeof(RL) / sizeof(RL[i]); l++)
					{
						if(pos < rlc + RL[l])
						{
							row = (7 + l) % 10;
							col = pos - rlc + 1;
							break;
						}
						rlc += RL[i];
					}
				}
				
				uint8_t result = (row << 4) | col;
				num_buf[j] = result;
				j++;
			}
			i += cplen;
		}
	}
	free(straddling_cps);
	return num_buf;
}

void one_time_pad(int8_t* src, int8_t* key, int len, char mode)
{
	for(int i = 0; i < len; i++)
	{
		char src_h = (src[i] >> 4) & 0x0F;
		char src_l = src[i] & 0x0F;
		char key_h = (key[i] >> 4) & 0x0F;
		char key_l = key[i] & 0x0F;
		// Don't cipher with key's 4h
		if(key_h & 0xF == 0xF) key_h = key_l;

		if(mode == 'e')
		{
			// Don't encrypt 4h part of src
			if(src_h != 0xF) src_h = (src_h + key_h) % 10;
			src_l = (src_l + key_l) % 10;
		}
		else if(mode == 'd')
		{
			if(src_h != 0xF) src_h = (src_h - key_h + 10) % 10;
			src_l = (src_l - key_l + 10) % 10;
		}
		src[i] = (src_h << 4) | (src_l & 0x0F);
	}
}

void print_hex(int8_t* nt1, int t1_len)
{
	for(int i = 0; i < t1_len; i++)
	{
		if((int8_t) (nt1[i] & 0xF0) == (int8_t) 0xF0)
			printf("%X ", (int32_t)nt1[i] & 0x0000000F);
		else
			printf("%X ", (int32_t)nt1[i] & 0x000000FF);
	}
	puts("");
}

char* decompress_to_utf8_char(int8_t* nt, int nt_len)
{
	char* str = (char*) malloc(nt_len * 2 + 1); // 2 bytes per sequence + ending 
	int j = 0;
	for(int i = 0; i < nt_len; i++)
	{
		if((nt[i] & 0xF0) == 0xF0)
		{
			char cpos = (nt[i] & 0x0F) - 1;
			// 1 ==> 1*2; 2 ==> 2*2 = 4
			cpos *= 2;
			str[j] = straddling_utf8[cpos];
			str[j + 1] = straddling_utf8[cpos + 1];
			j += 2;
		}
		else
		{
			char row = (nt[i] >> 4) & 0x0F;
			char col = (nt[i] & 0x0F) - 1;
			int cpos = 0;
			for(int k = 0; k < (row - 7 + 10) % 10; k++) // row = 8 k < 1; sum of all prev rows lens
				cpos += RL[k];
			cpos += col; // cpos = 7 col = 0 cpos = 7
			cpos *= 2; // 2 !!
			str[j] = straddling_utf8[cpos];
			str[j + 1] = straddling_utf8[cpos + 1];
			j += 2;
		}
	}
	str[nt_len * 2] = 0;
	return str;
}


int main()
{
	char* k1 = "ЛЕСЛЕС";
	char* t1 = "АБВГДЕ";
	int t1_len = str_utf8_len(t1, strlen(t1));

	int8_t* nt1 = compress_utf8_chars_to_nums(t1, strlen(t1));
	int8_t* nk1 = compress_utf8_chars_to_nums(k1, strlen(k1));
	puts("Original text:");
	print_hex(nt1, t1_len);
	char* text1 = decompress_to_utf8_char(nt1, t1_len);
	printf("%s\n", text1);

	one_time_pad(nt1, nk1, t1_len, 'e');

	puts("Encrypted text:");
	print_hex(nt1, t1_len);
	//char* text2 = decompress_to_utf8_char(nt1, t1_len);
	//printf("%s\n", text2);

	one_time_pad(nt1, nk1, t1_len, 'd');

	puts("Decrypted text:");
	print_hex(nt1, t1_len);
	char* text3 = decompress_to_utf8_char(nt1, t1_len);
	printf("%s\n", text3);

	free(nt1);
	free(nk1);
	free(text1); free(text3);
	exit(0);
}

