//
// Created by jholloway on 10/30/25.
//

#ifndef SIMPLESHELL_SH_SRC_H
#define SIMPLESHELL_SH_SRC_H

#include "pipecmd.h"

#define CONT_SH_LOOP 1
#define END_SH_LOOP 0

int sh_execute(char** args, char* keep);
void sh_init_linked_list();
void sh_loop(char** envp);

int sh_launch_pipe_version(Pipe_cmd pipeCmd, int async);

#endif  // SIMPLESHELL_SH_SRC_H
