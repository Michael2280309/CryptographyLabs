#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <zip.h>
#include <string.h>

void process_docx(const char* docx_filename) {
    int err;
    struct zip* archive = zip_open(docx_filename, 0, &err);
    if (archive == NULL) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        fprintf(stderr, "%s: cannot open zip archive '%s': %s\n",
                "main", docx_filename, zip_error_strerror(&error));
        zip_error_fini(&error);
        fprintf(stderr, "Error opening the DOCX file\n");
        return;
    }

    struct zip_stat stat;
    zip_stat_init(&stat);
    zip_stat(archive, "word/document.xml", 0, &stat);

    struct zip_file* file = zip_fopen(archive, "word/document.xml", 0);
    if (!file) {
        fprintf(stderr, "Error opening word/document.xml\n");
        zip_close(archive);
        return;
    }

    // Получаем размер содержимого файла
    size_t size = stat.size;

    // Выделяем буфер для содержимого файла
    char* xml_content = (char*)malloc(size + 1);
    if (!xml_content) {
        fprintf(stderr, "Memory allocation error\n");
        zip_fclose(file);
        zip_close(archive);
        return;
    }

    // Считываем содержимое файла
    zip_fread(file, xml_content, size);
    zip_fclose(file);
    zip_close(archive);

    // Добавляем завершающий нулевой символ
    xml_content[size] = '\0';

    // Парсим XML-документ
    xmlDocPtr doc = xmlReadMemory(xml_content, size, "noname.xml", NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Failed to parse XML content\n");
        free(xml_content);
        return;
    }

    // Получаем корневой элемент
    xmlNodePtr root = xmlDocGetRootElement(doc);
    if (root == NULL) {
        fprintf(stderr, "Empty XML document\n");
        xmlFreeDoc(doc);
        free(xml_content);
        return;
    }

    // Проходим по дереву XML и извлекаем текст
    for (xmlNodePtr node = root; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            xmlChar* content = xmlNodeGetContent(node);
            if (content != NULL) {
                printf("%s\n", content);
                xmlFree(content);
            }
        }
    }

    // Освобождаем ресурсы
    xmlFreeDoc(doc);
    free(xml_content);
}

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

uint64_t expand32(uint32_t block32, uint64_t key)
{
    uint64_t result; unsigned char* p_result = (unsigned char *) &result;
    for(int i = 0; i <= 3; i++)
    {
        unsigned char rev_uc = *(((unsigned char *) &block32) + (3 - i));
        unsigned char block_uc = *(((unsigned char *) &block32) + i);
        p_result[i * 2] = (block_uc & 0xF0) + (rev_uc & 0x0F);
        p_result[i * 2 + 1] = (block_uc & 0x0F) + (rev_uc & 0xF0);
    }
    result *= key; // overflow, 99%

    //printf("Expantion result: ");
    //dump_u64(result);

    return result;
}

uint32_t xor_shrink_with_key(uint64_t block64, uint64_t key)
{
    unsigned char* p_block64 = (unsigned char *) &block64;
    unsigned char* p_key = (unsigned char *) &key;
    uint32_t result32; unsigned char* p_result32 = (unsigned char *) &result32;

    for(int n = 6; n >= 3; n--)
        for(int i = 0; i <= n; i++)
        {
            unsigned char xored_key = p_block64[i] ^ p_block64[i + 1] ^ p_key[i];
            p_block64[i] = xored_key;
        }

    for(int i = 0; i < 4; i++)
    {
        p_result32[i] = p_block64[i];
    }

    //printf("Shrink result: ");
    //dump_u32(result32);

    return result32;
}

uint32_t expand_xor_key_shrink(uint32_t block32, uint64_t key, uint64_t key_expand) // 7-byte key
{
    for(int i = 0; i < 100; i++) // 10-100 ітерацій мусить бути цілком достатньо
    // для забезпечення потрібних властивостей хеш-функції
    {
        uint64_t expand = expand32(block32, key_expand);
        block32 = xor_shrink_with_key(expand, key);
    }
    return block32;
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

void brute_force_docx_collision(const char* docx_filename){
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
}


int main()
{
    srand(time(0));
    // uint32_t rn = rand();
    // puts("Current block is:");
    // dump_u32(rn);
    // rn = expand_xor_key_shrink(rn, 0xab3b74c66cd32646, 0x4737D542D8E56A4C);
    
    // puts("Resulting block dump:");
   

    uint8_t digest = 0;

    FILE* source = fopen("main.c", "rb");
    if(source != NULL){
        long size = get_file_size(source);
        uint8_t buffer[size];
        
        if(size == fread(buffer, 1, size, source))
        {
            uint32_t* ptr = (uint32_t*) buffer;
            for(long i = 0; i < size / 4; i++){
                digest ^= shrink_to_small(
                    expand_xor_key_shrink(ptr[i], 0xab3b74c66cd32646, 0x4737D542D8E56A4C), 2);
            }
            printf("Source code digest: %02x\n", digest);
        }
        fclose(source);
    }

    digest = 0;

    //process_docx("pract3.docx");
    
    FILE* img = fopen("lake.jpeg", "rb");
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

    brute_force_docx_collision("pract3.docx");

    return 0;
}
