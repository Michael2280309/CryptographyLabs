#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>

#define CYRILLIC_CODE_POINT_START_HEX 0x400
#define CYRILLIC_CODE_POINT_END_HEX 0x4FF
#define ALPHABET_LENGTH 31
#define FIRST_CAPITAL_LETTER_HEX 0x410
#define FIRST_LOWERCASE_LETTER_HEX 0x430
#define FIRST_CAPITAL_LETTER_LOW_BYTE_HEX 0x90
#define FIRST_LOWERCASE_LETTER_LOW_BYTE_HEX 0xb0

extern int utf8_codepoint_length(const uint8_t byte);
extern uint32_t utf8_decode(const uint8_t* sequence, int length);
extern void utf8_encode(uint32_t codepoint, uint8_t utf8_sequence[4], int* length);

long get_file_size(FILE* fp){
	fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return file_size;
}

int inside_capitals_ru(uint32_t point){
	return point >= FIRST_CAPITAL_LETTER_HEX && 
				point <= (FIRST_CAPITAL_LETTER_HEX + ALPHABET_LENGTH);
}

int inside_lowercase_ru(uint32_t point){
	return point >= FIRST_LOWERCASE_LETTER_HEX &&
				point <= (FIRST_LOWERCASE_LETTER_HEX + ALPHABET_LENGTH);
}

// ceasar_encrypt
// IN: текст російською абеткою в UTF-8, цифровий ключ
// OUT: зашифрований текст 
char* caesar_encrypt(char* textUTF8, long buf_size, int key){
	// такий самий за розміром буфер
	char* encrypted = (char*)malloc(buf_size);
	// Ітеруємось по усіх utf-8 послідовностях байтів, перевіряємо чи являє собою букву російської абетки.
	// Якщо це не так, тоді пропускаємо як спец. символ та вважаємо його з блоку Basic Latin
	// Інакше, виконуємо шифрування алгоритмом Цезаря та пишемо в буфер
	for(long i = 0; i < buf_size; ){
		int codepoint_length = utf8_codepoint_length(textUTF8[i]);
		// Якщо односимвольна послідовність, записуємо спец. символ та йдемо далі
		if (codepoint_length == 1 || codepoint_length == -1) { 
			encrypted[i] = textUTF8[i];
			i++;
			continue;
		}
		// рахуємо кодову точку
		uint32_t point = utf8_decode(textUTF8 + i, codepoint_length);	
		// робимо обчислення спираючись саме на кодову точку, 
		// а не на байтове представлення
		unsigned int encr = (point + key);
		//unsigned int offset = 0;
		// нехай codepoint_legth зараз є 2
		if (codepoint_length == 2 && inside_capitals_ru(point)) {
			encr -= FIRST_CAPITAL_LETTER_HEX;
			encr %= (ALPHABET_LENGTH + 1);
			encr += FIRST_CAPITAL_LETTER_HEX;
			// саме шифрування молодшого байту робить зрушення вперед за абеткою
			// модулем буде довжина абетки (31 буква) + номер початкової букви абетки (А)
			// if (encr >= ALPHABET_LENGTH + FIRST_CAPITAL_LETTER_HEX + 1){
			// 	offset = FIRST_CAPITAL_LETTER_HEX;
			// }
			// encr = encr % (ALPHABET_LENGTH + FIRST_CAPITAL_LETTER_HEX + 1);
			// encr += offset;
			int res_codepoint_len = 0;
			uint8_t res_codepoint_bytes[4];
			memset(res_codepoint_bytes, 0, 4);
			utf8_encode(encr, res_codepoint_bytes, &res_codepoint_len);
			for(int j = 0; j < res_codepoint_len; j++){
				encrypted[i + j] = res_codepoint_bytes[j];
			}
			i += res_codepoint_len;
			continue;
		}
		else if(codepoint_length == 2 && inside_lowercase_ru(point)) {
			encr -= FIRST_LOWERCASE_LETTER_HEX;
			encr %= (ALPHABET_LENGTH + 1);
			encr += FIRST_LOWERCASE_LETTER_HEX;
			// if (encr >= ALPHABET_LENGTH + FIRST_LOWERCASE_LETTER_HEX + 1){
			// 	offset = FIRST_LOWERCASE_LETTER_HEX;
			// }
			// encr = encr % (ALPHABET_LENGTH + FIRST_LOWERCASE_LETTER_HEX + 1);
			// encr += offset;
			int res_codepoint_len = 0;
			uint8_t res_codepoint_bytes[4];
			memset(res_codepoint_bytes, 0, 4);
			utf8_encode(encr, res_codepoint_bytes, &res_codepoint_len);
			for(int j = 0; j < res_codepoint_len; j++){
				encrypted[i + j] = res_codepoint_bytes[j];
			}
			i += res_codepoint_len;
 			continue;
		}
		else{ // UTF-послідовність явно не являє собою Basic Latin, але ми просто його залишаємо
			for(int j = 0; j < codepoint_length; j++)
				encrypted[i + j] = textUTF8[i + j];
			i += codepoint_length;
			continue;
		}
	}
	return encrypted;
}

// caesar_decrypt
// IN: шифр, ключ
// OUT: текст російською абеткою в UTF-8
char* caesar_decrypt(char* cipheredText, long buf_size, int key){
	char* decrypted = (char*) malloc(buf_size);
	for(int i = 0; i < buf_size; ){
		int codepoint_len = utf8_codepoint_length(cipheredText[i]);
		if(codepoint_len == 1 || codepoint_len == -1){
			decrypted[i] = cipheredText[i];
			i++;
			continue;
		}
		int32_t point = utf8_decode(cipheredText + i, codepoint_len);
		int32_t decr = 0;
		int32_t result = 0;
		//int32_t offset = 0;
		// буква належить до нижнього регістру
		if(codepoint_len == 2 && inside_lowercase_ru(point)){
			decr = point - key - FIRST_LOWERCASE_LETTER_HEX;
			if(decr < 0){
				decr = (-decr) % (ALPHABET_LENGTH + 1);
				result = (FIRST_LOWERCASE_LETTER_HEX + ALPHABET_LENGTH + 1) - decr;
			}
			else{
				result = decr + FIRST_LOWERCASE_LETTER_HEX;
			}

			// if(decr < FIRST_LOWERCASE_LETTER_HEX)
			// 	offset = (FIRST_LOWERCASE_LETTER_HEX - decr) % ALPHABET_LENGTH;
			// if(offset > 0)
			// 	result = FIRST_LOWERCASE_LETTER_HEX + ALPHABET_LENGTH + 1 - offset;
			// else result = decr;
			int point_len = 0;
			uint8_t seq[4];
			memset(seq, 0, 4);
			utf8_encode(result, seq, &point_len);
			for(int j = 0; j < point_len; j++){
				decrypted[i + j] = seq[j];
			}
			i += point_len;
			continue;
		}
		else if(codepoint_len == 2 && inside_capitals_ru(point)){
			decr = point - key - FIRST_CAPITAL_LETTER_HEX;
			if(decr < 0){
				decr = (-decr) % (ALPHABET_LENGTH + 1);
				result = (FIRST_CAPITAL_LETTER_HEX + ALPHABET_LENGTH + 1) - decr;
			}
			else{
				result = decr + FIRST_CAPITAL_LETTER_HEX;
			}
			// if(decr < FIRST_CAPITAL_LETTER_HEX)
			// 	offset = (FIRST_CAPITAL_LETTER_HEX - decr) % ALPHABET_LENGTH;
			// if(offset > 0)
			// 	result = FIRST_CAPITAL_LETTER_HEX + ALPHABET_LENGTH + 1 - offset;
			// else result = decr;
			int point_len = 0;
			uint8_t seq[4];
			memset(seq, 0, 4);
			utf8_encode(result, seq, &point_len);
			for(int j = 0; j < point_len; j++){
				decrypted[i + j] = seq[j];
			}
			i += point_len;
			continue;
		}
		else{ // UTF-послідовність явно не являє собою Basic Latin, але ми просто його залишаємо
			for(int j = 0; j < codepoint_len; j++)
				decrypted[i + j] = cipheredText[i + j];
			i += codepoint_len;
			continue;
		}
	}
	return decrypted;
}
  
// vigenere_encrypt
// IN: string message, string keyphrase
// OUT: encrypted message string
char* vigenere_encrypt(const char* msg, uint32_t msglen, char* key, uint32_t keylen){
	char* encrbuf = (char*) malloc(msglen);
	uint32_t k = 0;
	for(uint32_t i = 0; i < msglen; ){
		int seqlen = utf8_codepoint_length(msg[i]);
		if(seqlen == 1){
			encrbuf[i] = msg[i];
			i++;
			continue;
		}
		uint32_t point = utf8_decode(msg + i, seqlen);
		uint32_t key_index = k % keylen;
		uint32_t keyseqlen = utf8_codepoint_length(key[key_index]);
		if (keyseqlen < 2) {
			fprintf(stderr, "key UTF-8 sequence is less than 2 at %d.\n", key_index);
			// Do nothing
		}
		uint32_t point_key = utf8_decode(key + key_index, keyseqlen);
		k += keyseqlen;
		uint32_t offset_key = 0;
		if(inside_capitals_ru(point_key)){
			offset_key = point_key - FIRST_CAPITAL_LETTER_HEX;
		}
		else if(inside_lowercase_ru(point_key)){
			offset_key = point_key - FIRST_LOWERCASE_LETTER_HEX;
		}
		uint32_t offset = 0;
		uint32_t encoded = point + offset_key;
		if(seqlen == 2 && inside_capitals_ru(point)){
			encoded -= FIRST_CAPITAL_LETTER_HEX;
			encoded %= (ALPHABET_LENGTH + 1);
			encoded += FIRST_CAPITAL_LETTER_HEX;
			// if(encoded > FIRST_CAPITAL_LETTER_HEX + ALPHABET_LENGTH){
			// 	offset += FIRST_CAPITAL_LETTER_HEX;
			// }
			// encoded = encoded % (FIRST_CAPITAL_LETTER_HEX + ALPHABET_LENGTH + 1);
			// encoded += offset;
			int len = 0;
			uint8_t sequtf[4];
			memset(sequtf, 0, 4);
			utf8_encode(encoded, sequtf, &len);
			for(int j = 0; j < len; j++){
				encrbuf[i + j] = sequtf[j];
			}
			i += len;
			continue;
		}
		else if(seqlen == 2 && inside_lowercase_ru(point)){
			encoded -= FIRST_LOWERCASE_LETTER_HEX;
			encoded %= (ALPHABET_LENGTH + 1);
			encoded += FIRST_LOWERCASE_LETTER_HEX;

			// if(encoded > FIRST_LOWERCASE_LETTER_HEX + ALPHABET_LENGTH){
			// 	offset += FIRST_LOWERCASE_LETTER_HEX;
			// }
			// encoded = encoded % (FIRST_LOWERCASE_LETTER_HEX + ALPHABET_LENGTH + 1);
			// encoded += offset;
			int len = 0;
			uint8_t sequtf[4];
			memset(sequtf, 0, 4);
			utf8_encode(encoded, sequtf, &len);
			for(int j = 0; j < len; j++){
				encrbuf[i + j] = sequtf[j];
			}
			i += len;
			continue;		
		}
		else{
			for(int j = 0; j < seqlen; j++){
				encrbuf[i + j] = msg[i + j];
			}
			i += seqlen;
			continue;
		}
	}
	return encrbuf;
}

char* vigenere_decrypt(const char* cyph, uint32_t cyphlen, char* key, uint32_t keylen){
	char* msg = (char*) malloc(cyphlen);
	uint32_t k = 0;
	for(int i = 0; i < cyphlen; ){ 
		int cplen = utf8_codepoint_length(cyph[i]);
		if(cplen == 1){
			msg[i] = cyph[i];
			i++;
			continue;
		}
		uint32_t cp = utf8_decode(cyph + i, cplen);
		uint32_t keyind = k % keylen;
		int cpkeylen = utf8_codepoint_length(key[keyind]);
		uint32_t cpkey = utf8_decode(key + (keyind), cpkeylen);
		k += cpkeylen;
		uint32_t keyoffset = 0;
		if(inside_capitals_ru(cpkey))
			keyoffset = cpkey - FIRST_CAPITAL_LETTER_HEX;
		else if(inside_lowercase_ru(cpkey))
			keyoffset = cpkey - FIRST_LOWERCASE_LETTER_HEX;
		int32_t diff = cp - keyoffset;
		//int32_t offset_alphabet = 0;
		int32_t resultcp = 0;
		if(cplen == 2 && inside_capitals_ru(cp)){
			diff -= FIRST_CAPITAL_LETTER_HEX;
			if(diff < 0){
				diff = (-diff) % (ALPHABET_LENGTH + 1);
				resultcp = (FIRST_CAPITAL_LETTER_HEX + ALPHABET_LENGTH + 1) - diff;
			}
			else{
				resultcp = diff + FIRST_CAPITAL_LETTER_HEX;
			}
			// if(diff < FIRST_CAPITAL_LETTER_HEX){
			// 	offset_alphabet = (FIRST_CAPITAL_LETTER_HEX - diff) % ALPHABET_LENGTH;
			// 	resultcp = (FIRST_CAPITAL_LETTER_HEX + ALPHABET_LENGTH + 1 - offset_alphabet);
			// }
			// else resultcp = diff;
			int rescplen = 0;
			uint8_t res_seq[4];
			utf8_encode(resultcp, res_seq, &rescplen);
			for(int j = 0; j < rescplen; j++){
				msg[i + j] = res_seq[j];
			}
			i += rescplen;
			continue;
		}
		else if(cplen == 2 && inside_lowercase_ru(cp)){
			diff -= FIRST_LOWERCASE_LETTER_HEX;
			if(diff < 0){
				diff = (-diff) % (ALPHABET_LENGTH + 1);
				resultcp = (FIRST_LOWERCASE_LETTER_HEX + ALPHABET_LENGTH + 1) - diff;
			}
			else{
				resultcp = diff + FIRST_LOWERCASE_LETTER_HEX;
			}
			// if(diff < FIRST_LOWERCASE_LETTER_HEX){
			// 	offset_alphabet = (FIRST_LOWERCASE_LETTER_HEX - diff) % ALPHABET_LENGTH;
			// 	resultcp = (FIRST_LOWERCASE_LETTER_HEX + ALPHABET_LENGTH + 1 - offset_alphabet);
			// }
			// else resultcp = diff;
			int rescplen = 0;
			uint8_t res_seq[4];
			utf8_encode(resultcp, res_seq, &rescplen);
			for(int j = 0; j < rescplen; j++){
				msg[i + j] = res_seq[j];
			}
			i += rescplen;
			continue;
		}
		else{
			for(int j = 0; j < cplen; j++){
				msg[i + j] = cyph[i + j];
			}
			i += cplen;
			continue;
		}
	}
	return msg;
}


#include "test.h"

int main(int argc, char const *argv[])
{
	UINT oldCodePage;

	oldCodePage = GetConsoleOutputCP();
    if (!SetConsoleOutputCP(65001)) {
        perror("Error setting CP.\n");
        return 1;
    }

// Тести
    test_caesar_encrypt_decrypt();
	test_vigenere_encrypt_decrypt(); 

    SetConsoleOutputCP(oldCodePage);
    return 0;
}
