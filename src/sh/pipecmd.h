//
// Created by jholloway on 11/23/25.
//

#ifndef SIMPLESHELL_PIPECMD_H
#define SIMPLESHELL_PIPECMD_H

typedef struct pipe_cmd {
  int pipe_count;
  int pipe_locations[16];  // we'll only allow a max of sixteen pipes in a
  // command for the moment.
  int args_length;  // number of tokens in command.
  char** args;      // pointer to start of command (will be freed by preceding
                    // function);
} Pipe_cmd;

#endif  // SIMPLESHELL_PIPECMD_H
