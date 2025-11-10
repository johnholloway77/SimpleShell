//
// Created by jholloway on 11/9/25.
//

#include <criterion/criterion.h>
#include <stdlib.h>
#include "../src/strtrim.h"

Test(strtrim, trim_test){

    char *string = (char *)malloc(7);
    strlcpy(string, " test ", 7);
    string = strtrim(string);
    cr_expect_str_eq(string, "test");
    free(string);
}