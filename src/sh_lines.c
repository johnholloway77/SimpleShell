//
// Created by jholloway on 11/11/25.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sh_lines.h"

char* sh_read_line(void) {
  char* line = NULL;
  size_t buffsize = 0;

  if (getline(&line, &buffsize, stdin) == -1) {
    if (feof((stdin))) {
      fprintf(stderr, "EOF\n");
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "Value of errno: %d\n", errno);
      exit(EXIT_FAILURE);
    }
  }
  return line;
}

char** sh_split_line(char* line) {
  int bufsize = SH_TOK_BUFSIZE;
  int position = 0;
  // char* save = NULL;
  char** tokens = malloc(bufsize * sizeof(char*));
  char** token_backup = NULL;
  char* token;

  if (!tokens) {
    fprintf(stderr, "jhsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(tokens);
        fprintf(stderr, "jhsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SH_TOK_DELIM);
  }
  tokens[position] = NULL;

  return tokens;
}