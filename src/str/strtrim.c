//
// Created by jholloway on 11/9/25.
//
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

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

  // Error check
  ptrdiff_t difference = end - start + 1;

  if (difference <= 0){
    char *out = (char *)malloc(1);
    if(out) {
      out[0] = '\0';
    }
    return out;
  }

  size_t length = (size_t)difference;
  memmove(string, start, length);
  string[length] = '\0';
  return string;
}