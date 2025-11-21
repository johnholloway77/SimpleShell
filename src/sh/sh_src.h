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
#include "./sh_lines.h"

#define CONT_SH_LOOP 1
#define END_SH_LOOP 0

typedef struct pipe_cmd {
  int pipe_count;
  int pipe_locations[16];  // we'll only allow a max of sixteen pipes in a
                           // command for the moment.
  int args_length;         // number of tokens in command.
  char** args;  // pointer to start of command (will be freed by preceding
                // function);
} Pipe_cmd;

int sh_execute(char** args, char* keep);
int sh_launch(char** args, int async);
void sh_init_linked_list();
void sh_loop(char** envp);

int sh_launch_pipe_version(Pipe_cmd pipeCmd, int async);

#endif  // SIMPLESHELL_SH_SRC_H
