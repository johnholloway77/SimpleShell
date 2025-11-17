//
// Created by jholloway on 11/11/25.
//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../str/strl.h"
#include "sh_lines.h"
#include "sh_src.h"

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
  if (!line) {
    return NULL;
  }

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

Path_struct* sh_parse_path_line(char* path_line) {
  if (!path_line) {
    return NULL;
  }

  Path_struct* ps = (Path_struct*)malloc(sizeof(Path_struct));

  int bufsize = SH_TOK_BUFSIZE;
  int position = 0;
  // char* save = NULL;
  ps->path_options = malloc(bufsize * sizeof(char*));
  char** token_backup = NULL;
  char* token;

  if (!ps->path_options) {
    fprintf(stderr, "jhsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(path_line, ":");
  while (token != NULL) {
    ps->path_options[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SH_TOK_BUFSIZE;
      ps->path_options = realloc(ps->path_options, bufsize * sizeof(char*));
      if (!ps->path_options) {
        free(ps->path_options);
        fprintf(stderr, "jhsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, ":");
  }
  ps->path_options[position] = NULL;
  ps->count = position;

  return ps;
}

char** update_args(char** args, char** envp) {
  if (!args || !envp) {
    return NULL;
  }

  int bufsize = SH_TOK_BUFSIZE;

  char** tokens = malloc(bufsize * sizeof(char*));

  for (size_t i = 0; i < bufsize; i++) {
    tokens[i] = NULL;
  }

  if (!tokens) {
    fprintf(stderr, "jhsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  int i = 0;
  while (args[i]) {
    char* line = (char*)malloc(SH_TOK_BUFSIZE);
    memset(line, 0, SH_TOK_BUFSIZE);

    char* value = getenv(args[i] + 1);

    if (args[i][0] == '$') {
      if (value) {
        ;
        strlcpy(line, value, SH_TOK_BUFSIZE);

        tokens[i] = line;
      }

    } else {
      strlcpy(line, args[i], SH_TOK_BUFSIZE);
      tokens[i] = line;
    }

    if (i >= bufsize) {
      bufsize += SH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(tokens);
        fprintf(stderr, "jhsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    // free(line);
    i++;
  }

  return tokens;
}