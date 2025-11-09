//
// Created by jholloway on 11/9/25.
//
#include <string.h>
#include <ctype.h>

char* strtrim(char* string) {
    if (!string) {
        return string;
    }

    unsigned char* start = (unsigned char*)string;

    while (*start && isspace(*start)) {
        start++;
    }

    unsigned char* end = start + strlen((char*)start) - 1;
    while (*end && isspace(*end)) {
        end--;
    }

    size_t length = (size_t)(end - start + 1);
    memmove(string, start, length);
    string[length] = '\0';
    return string;
}