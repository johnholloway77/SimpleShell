//
// Created by jholloway on 11/9/25.
//

#define __BSD_VISIBLE 1
#include <criterion/criterion.h>
#include <stdlib.h>

#include "../src/strtrim.h"
#include "../src/strl.h"

Test(strtrim, trim_test) {
  char* text = "  test   ";
  int length = strlen(text) + 1;
  char* string = (char*)malloc(length);
  strlcpy(string, text, length);
  strtrim(string);
  cr_expect_str_eq(string, "test");
  free(string);
}

Test(strtrim, trim_test_without_space) {
  char* text = "test";
  int length = strlen(text) + 1;
  char* string = (char*)malloc(length);
  strlcpy(string, text, length);
  strtrim(string);
  cr_expect_str_eq(string, "test");
  free(string);
}

Test(strtrim, trim_test_null) {
  char* string2 = NULL;
  string2 = strtrim(string2);
  cr_expect_null(string2);
}