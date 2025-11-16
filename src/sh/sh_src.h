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

#include "../term/toggle_echo.h"

#define CONT_SH_LOOP 1
#define END_SH_LOOP 0

int sh_execute(char** args, char* keep);
int sh_launch(char** args);
void sh_init_linked_list();
void sh_loop(char** envp);

#endif  // SIMPLESHELL_SH_SRC_H
