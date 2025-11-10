//
// Created by jholloway on 11/9/25.
//
//

#include <criterion/criterion.h>
#include <unistd.h>      // pipe, dup2, write, close
#include <string.h>      // strlen
#include <stdlib.h>
#include "../src/sh_src.h"

static char *with_stdin(const char *script, char *(*fn)(void)) {
    int p[2]; cr_assert_eq(pipe(p), 0);
    int saved = dup(STDIN_FILENO);

    dup2(p[0], STDIN_FILENO);
    close(p[0]);

    if (script) (void)write(p[1], script, strlen(script));
    close(p[1]);                     // EOF after data so getline returns

    char *ret = fn();                // CAPTURE the result

    dup2(saved, STDIN_FILENO);
    close(saved);
    return ret;
}

static char *run_once(void) { return sh_read_line(); }

Test(sh_read_line, returns_line) {
    const char *test_string = "hello world\n";   // include newline for getline
    char *line = with_stdin(test_string, run_once);

    cr_expect_not_null(line);
    cr_expect_str_eq(line, "hello world\n");     // getline keeps the '\n'
    free(line);                                  // free what getline allocated
}


Test(sh_split_line, sh_split_lines_test){

    char *line = "the cat in the hat";
    size_t length = strlen(line) + 1;
    char *line_allocated = (char *)malloc(length);

    strlcpy(line_allocated, line, length);

    char **args;

    char *expected_args[] ={"the", "cat", "in", "the", "hat", NULL};

    args = sh_split_line(line_allocated);

    int i = 0;
    while (expected_args[i]) {
        cr_expect_not_null(args[i]);
        cr_expect_str_eq(args[i], expected_args[i]);
        i++;
    }

}