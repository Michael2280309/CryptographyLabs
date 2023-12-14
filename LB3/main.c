#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
// #include <libxml/parser.h>
// #include <libxml/tree.h>
// #include <zip.h>
#include <string.h>
#include <fcntl.h>

#define LZ64 0x00000000FFFFFFFF
#define HZ64 0xFFFFFFFF00000000

void dump_u32(uint32_t n)
{
    unsigned char* p_n32 = (unsigned char *) &n;

    for(int i = 0; i < 4; i++)
        printf("%02x", p_n32[i]);

    printf("\n");
}

void dump_u64(uint64_t n)
{
    unsigned char* p_n64 = (unsigned char *) &n;

    for(int i = 0; i < 8; i++)
        printf("%02x", p_n64[i]);

    printf("\n");
}

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

uint8_t shrink_to_small(uint32_t block32, int bit_size)
{
    uint8_t res;

    switch(bit_size)
    {
        case 2:
            res = (block32 & 0x03);
            break;
        case 4:
            res = (block32 & 0x0f);
            break;
        case 8:
            res = (block32 & 0xff);
            break;
    }

    return res;
}

long get_file_size(FILE* fp){
    fseek(fp, 0, SEEK_END); // Move the file pointer to the end of the file
    long size = ftell(fp); // Get the current position of the file pointer (which is the file size)
    fseek(fp, 0, SEEK_SET); // to beginning
    return size;
}

/* void brute_force_docx_collision(const char* docx_filename){
    uint8_t digest_original = 0;

    FILE* source = fopen(docx_filename, "rb");
    if(source != NULL){
        long size = get_file_size(source);
        uint8_t buffer[size];
        
        if(size == fread(buffer, 1, size, source))
        {
            uint32_t* ptr = (uint32_t*) buffer;
            for(long i = 0; i < size / 4; i++){
                digest_original ^= shrink_to_small(
                    expand_xor_key_shrink(ptr[i], 0xab3b74c66cd32646, 0x4737D542D8E56A4C), 2);
            }
        }
        fclose(source);
    }
    else{
        fprintf(stderr, "File not opened: %s", docx_filename);
        return;
    }

    uint8_t digest_hash = 0;
    do {
        // Open the DOCX file
        int err;
        struct zip *archive = zip_open(docx_filename, 0, &err);
        if (archive == NULL) {
            fprintf(stderr, "Error opening DOCX file\n");
            return;
        }

        // Open word/document.xml from the archive
        struct zip_file *file = zip_fopen(archive, "[Content_Types].xml", 0);
        if (!file) {
            fprintf(stderr, "Error opening [Content_Types].xml\n");
            zip_close(archive);
            return;
        }

        // Read the content of word/document.xml into a buffer
        struct zip_stat stat;
        zip_stat_init(&stat);
        zip_stat(archive, "[Content_Types].xml", 0, &stat);

        char *xml_content = (char *)malloc(stat.size + 1 + 20);
        if (!xml_content) {
            fprintf(stderr, "Memory allocation error\n");
            zip_fclose(file);
            zip_close(archive);
            return;
        }

        zip_fread(file, xml_content, stat.size);
        zip_fclose(file);

        // Null-terminate the buffer
        xml_content[stat.size] = '\0';

        // Close the archive
        zip_close(archive);

        strcat(xml_content, " "); // Додаємо пробіл у кінець файла

        archive = zip_open(docx_filename, ZIP_CREATE, &err);
        if (archive == NULL) {
            fprintf(stderr, "Error opening DOCX file for writing\n");
            return;
        }

        zip_source_t *source = zip_source_buffer(archive, xml_content, strlen(xml_content), 1);
        zip_file_add(archive, "[Content_Types].xml", source, ZIP_FL_OVERWRITE);

        // Clean up
        zip_source_free(source);
        zip_close(archive);
        //free(xml_content);

        digest_hash = 0;

        FILE* source2 = fopen(docx_filename, "rb");
        if(source2 != NULL){
            long size = get_file_size(source2);
            uint8_t buffer[size];
            
            if(size == fread(buffer, 1, size, source2))
            {
                uint32_t* ptr = (uint32_t*) buffer;
                for(long i = 0; i < size / 4; i++){
                    digest_hash ^= shrink_to_small(
                        expand_xor_key_shrink(ptr[i], 0xab3b74c66cd32646, 0x4737D542D8E56A4C), 2);
                }
            }
            fclose(source2);
        }
        else{
            fprintf(stderr, "File not opened: %s", docx_filename);
            return;
        }

    } while(digest_hash != digest_original);

    printf("Found common digest! %02x == %02x.\n", digest_hash, digest_original);
} */

void elem_test(){
    uint64_t number = 0xf1234567890abcde;
    dump_u64(my_hash_foo(number, 0xab3b74c66cd32646, 0x4737D542D8E56A4C));
}

void test(){
    uint64_t digest = 0;

    FILE* source = fopen("./main.c", "rb");
    if(source != NULL){
        long size = get_file_size(source);
        unsigned char buffer[size];
        
        if(size == fread(buffer, 1, size, source))
        {
            uint64_t* ptr = (uint64_t*) buffer;
            for(long i = 0; i < size / 8; i++){
                digest ^= my_hash_foo(ptr[i], 0xab3b74c66cd32646, 0x4737D542D8E56A4C);
            }
            printf("Source code digest: ");
            dump_u64(digest);
        }
        fclose(source);
    }
}


int main()
{    
    
    for(int i = 0; i < 1; i++) {
        test();
        //elem_test();
    } 

    ////digest = 0;
    
    /* FILE* img = fopen("lake.jpeg", "rb");
    if(img != NULL){
        long size = get_file_size(img);
        uint8_t buffer[size];
        
        if(size == fread(buffer, 1, size, img))
        {
            uint32_t* ptr = (uint32_t*) buffer;
            for(long i = 0; i < size / 4; i++){
                digest ^= shrink_to_small(
                    expand_xor_key_shrink(ptr[i], 0xab3b74c66cd32646, 0x4737D542D8E56A4C), 2);
            }
            printf("Image digest: %02x\n", digest);
        }
        fclose(img);
    }

    digest = 0;

    FILE* doc = fopen("pract3.docx", "rb");
    if(doc != NULL){
        long size = get_file_size(doc);
        uint8_t buffer[size];
        
        if(size == fread(buffer, 1, size, doc))
        {
            uint32_t* ptr = (uint32_t*) buffer;
            for(long i = 0; i < size / 4; i++){
                digest ^= shrink_to_small(
                    expand_xor_key_shrink(ptr[i], 0xab3b74c66cd32646, 0x4737D542D8E56A4C), 2);
            }
            printf("MS Word document digest: %02x\n", digest);
        }
        fclose(doc);
    }

    brute_force_docx_collision("pract3.docx"); */

    return 0;
}
