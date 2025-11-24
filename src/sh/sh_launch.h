//
// Created by jholloway on 11/23/25.
//

#ifndef SIMPLESHELL_SH_LAUNCH_H
#define SIMPLESHELL_SH_LAUNCH_H

#include "pipecmd.h"

#define SUCCESS_PIPE 2
#define INVALID_PIPE 3
#define PIPE_FAILURE 4
#define MAX_PIPE_COUNT 16

typedef struct redirect_str {
  int saved_stdin;
  int saved_stdout;
  int saved_stderr;
  int fd_in;
  int fd_out;
  int fd_err;
  int redir_out;
  int redir_in;
  int redir_err;
} Redirect_str;

int sh_launch_pipe_version(Pipe_cmd pipeCmd, int async);

#endif  // SIMPLESHELL_SH_LAUNCH_H
