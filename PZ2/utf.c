#include <stdint.h>

int utf8_codepoint_length(const uint8_t byte) {
    if ((byte & 0x80) == 0) {
        return 1; // Однобайтовая кодовая точка
    } else if ((byte & 0xE0) == 0xC0) {
        return 2; // Двухбайтовая кодовая точка
    } else if ((byte & 0xF0) == 0xE0) {
        return 3; // Трехбайтовая кодовая точка
    } else if ((byte & 0xF8) == 0xF0) {
        return 4; // Четырехбайтовая кодовая точка
    } else {
        return -1; // Недопустимая последовательность
    }
}

void utf8_encode(uint32_t codepoint, uint8_t utf8_sequence[4], int* length) {
    if (codepoint <= 0x7F) {
        // Single-byte character
        utf8_sequence[0] = (uint8_t)codepoint;
        *length = 1;
    } else if (codepoint <= 0x7FF) {
        // Two-byte character
        utf8_sequence[0] = (uint8_t)((codepoint >> 6) | 0xC0); // Setting 2h bits of upper 6-bit portion
        utf8_sequence[1] = (uint8_t)((codepoint & 0x3F) | 0x80); // Setting 1h bit of 2-nd byte preserving 6l bits
        *length = 2;
    } else if (codepoint <= 0xFFFF) {
        // Three-byte character
        utf8_sequence[0] = (uint8_t)((codepoint >> 12) | 0xE0);
        utf8_sequence[1] = (uint8_t)(((codepoint >> 6) & 0x3F) | 0x80);
        utf8_sequence[2] = (uint8_t)((codepoint & 0x3F) | 0x80);
        *length = 3;
    } else if (codepoint <= 0x10FFFF) {
        // Four-byte character
        utf8_sequence[0] = (uint8_t)((codepoint >> 18) | 0xF0);
        utf8_sequence[1] = (uint8_t)(((codepoint >> 12) & 0x3F) | 0x80);
        utf8_sequence[2] = (uint8_t)(((codepoint >> 6) & 0x3F) | 0x80);
        utf8_sequence[3] = (uint8_t)((codepoint & 0x3F) | 0x80);
        *length = 4;
    } else {
        // Invalid code point
        *length = 0;
    }
}

uint32_t utf8_decode(const uint8_t* sequence, int length) {
    if (length <= 0) {
        return 0;
    }
    uint32_t codepoint = 0;
    if (length == 1) {
        // Однобайтовая кодовая точка
        codepoint = sequence[0];
    } else {
        // Многобайтовая кодовая точка
        codepoint = sequence[0] & (0xFF >> (length + 1)); // clearing 3h bits of [0]
        for (int i = 1; i < length; i++) { // starting from second byte
            codepoint <<= 6; // shift left 6 times
            codepoint |= (sequence[i] & 0x3F); // clear 2h bits and set that 6 bits
        }
    }
    return codepoint;
}
