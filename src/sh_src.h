//
// Created by jholloway on 10/30/25.
//

#ifndef SIMPLESHELL_SH_SRC_H
#define SIMPLESHELL_SH_SRC_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "./toggle_echo.h"

#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"


int sh_execute(char** args);
int sh_launch(char** args);
void sh_init_linked_list();
void sh_loop(void);
char* sh_read_line(void);
char** sh_split_line(char* line);

#endif  // SIMPLESHELL_SH_SRC_H
