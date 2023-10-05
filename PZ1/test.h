int test_caesar_encrypt_rot1(){
	FILE* fp = fopen("./test/test2.txt", "rb");
	if(fp == NULL) {
		perror("Error opening test2.txt.\n");
		return 1;
	}
	long buf_size = get_file_size(fp);
	char* buf = (char*) malloc(buf_size);
	long read = fread(buf, 1, buf_size, fp);
	if (read != buf_size){
		perror("Read less bytes than buf_size.\n");
		return 1;
	}
	char* crypted = caesar_encrypt(buf, buf_size, 1);
	FILE* fp_res = fopen("./test/test2-encr.txt", "wb");
	if (fp_res == NULL){
		perror("Error creating test2-encr.txt.\n");
		return 1;
	}
	fwrite(crypted, 1, buf_size, fp_res);
	free(buf);
	free(crypted);
	fclose(fp);
	fclose(fp_res);
	return 0;
}

void test_caesar_encrypt_decrypt(){
	FILE* fp = fopen("./test/test1-original.txt", "rb");
	if (fp == NULL) {
        perror("Error opening the file.\n");
        return;
    }

    size_t file_size = get_file_size(fp);
    char* utf8_original_buffer = (char*)malloc(file_size);
    size_t read = fread(utf8_original_buffer, 1, file_size, fp);
    if (file_size != read){
    	perror("test1-original wasn't read to the end.\n");
    	return;
    }

	char* result = caesar_encrypt(utf8_original_buffer, file_size, 18);
    FILE* fp_result = fopen("./test/test1-encrypted.txt", "wb");
    fwrite(result, 1, file_size, fp_result);

    char* decrypted = caesar_decrypt(result, file_size, 18);
    FILE* fp_decr = fopen("./test/test1-decrypted.txt", "wb");
    fwrite(decrypted, 1, file_size, fp_decr);

    fclose(fp_decr);
    free(decrypted);
    fclose(fp_result);
    free(result);

    free(utf8_original_buffer);
    fclose(fp);
}

void test_vigenere_encrypt_decrypt(){
	FILE* fp = fopen("./test/vigenere-test1.txt", "rb");
	if(fp == NULL){
		perror("Error opening vigenere-test1.txt.\n");
		return;
	}
	long file_size = get_file_size(fp);
	char* test1 = (char*) malloc(file_size);
	long read = fread(test1, 1, file_size, fp);
	if(read != file_size){
		perror("vigener-test1.txt wasn't read to the end.\n");
		return;
	}
	FILE* fp_key = fopen("./test/vigenere-key.txt", "rb");
	if(fp_key == NULL){
		perror("Error opening vigenere-key.txt.\n");
		return;
	}
	long key_file_size = get_file_size(fp_key);
	char* key1 = (char*) malloc(key_file_size);
	long key_read = fread(key1, 1, key_file_size, fp_key);
	if(key_read != key_file_size){
		perror("vigener-key.txt wasn't read to the end.\n");
		return;
	}
	char* res1 = vigenere_encrypt(test1, file_size, key1, key_file_size);
	FILE* fp_out = fopen("./test/vigenere-encrypted.txt", "wb");
	if(fp_out == NULL){
		perror("Error opening vigenere-encrypted.txt for write binary.\n");
		return;
	}
	fwrite(res1, 1, file_size, fp_out);

	char* res0 = vigenere_decrypt(res1, file_size, key1, key_file_size);
	FILE* fp_dec = fopen("./test/vigenere-decrypted.txt", "wb");
	if(fp_dec == NULL){
		perror("Error opening vigenere-decrypted.txt for write binary.\n");
		return;
	}
	//fwrite(res0, 1, file_size, stdout);
	fwrite(res0, 1, file_size, fp_dec);
	
	fclose(fp);
	fclose(fp_key);
	fclose(fp_out);
	fclose(fp_dec);
	free(res0);
	free(test1);
	free(res1);
	free(key1);
}

void test_vigenere_decrypt(){
	FILE* fp = fopen("./test/vigenere-encrypted.txt", "rb");
	if(fp == NULL){
		perror("Error opening vigenere-encrypted.txt.\n");
		return;
	}
	long size = get_file_size(fp);
	char* enc = (char*) malloc(size);
	long read = fread(enc, 1, size, fp);
	if(read != size){
		perror("vigenere-encrypted.txt wasn't read to the end");
		return;
	}	
	FILE* fp_key = fopen("./test/vigenere-key.txt", "rb");
	if(fp_key == NULL){
		perror("Error opening vigenere-key.txt.\n");
		return;
	}
	long key_file_size = get_file_size(fp_key);
	char* key1 = (char*) malloc(key_file_size);
	long key_read = fread(key1, 1, key_file_size, fp_key);
	if(key_read != key_file_size){
		perror("vigener-key.txt wasn't read to the end.\n");
		return;
	}
	char* dec = vigenere_decrypt(enc, size, key1, key_file_size);
	FILE* fp_dec = fopen("./test/vigenere-decrypted.txt", "wb");
	if(fp_dec == NULL){
		perror("Error opening vigenere-decrypted.txt for write binary.\n");
		return;
	}
	fwrite(dec, 1, size, fp_dec);

	fclose(fp_dec);
	fclose(fp_key);
	fclose(fp);
	free(dec);
	free(enc);
	free(key1);
}