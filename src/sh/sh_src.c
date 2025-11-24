//
// Created by jholloway on 10/30/25.
//

#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../flags/flags.h"
#include "../flags/set_flags.h"
#include "../macro_linked_list.h"
#include "../sig/sig_handlers.h"
#include "../str/strtrim.h"
#include "../term/toggle_echo.h"
#include "./sh_src.h"
#include "bools.h"
#include "sh_launch.h"
#include "sh_lines.h"

#define UNUSED(x) (void)(x)

static line_list__node* line_iterator;
static line_list line_linked_list;

static void print_lines(char** p_line, void* x) {
  UNUSED(x);
  char* line = *p_line;

  if (!line) {
    return;
  }
  printf("\t%s\n", line);
}

static void free_lines(char** string) {
  free(*string);
}

void sh_init_linked_list() {
  line_list_init(&line_linked_list);
}

char* line_list_node_get_value(line_list__node* node) {
  return node ? node->value : NULL;
}

void sh_exit() {
  line_list_clear(&line_linked_list, free_lines);

  exit(EXIT_SUCCESS);
}

void sh_print_linked_list() {
  line_list_foreach(&line_linked_list, print_lines, NULL);
}

void sh_loop(char** envp) {
  char* line;
  char** args;
  char** updated_args;
  uint32_t updated_args_count;

  while (1) {
    char keep_line = FALSE;
    int status;

    if (!(app_flags & C_FLAG)) {
      printf("JHsh$: ");
      fflush(stdout);
    }

    (void)reap_background_jobs();

    line = sh_read_line();
    if (!line) {
      break;
    }
    strtrim(line);
    char* line_duplicate = strndup(line, strlen(line));

    args = sh_split_line(line);

    if ((updated_args = update_args(args, envp)) == NULL) {
      free(line);
      free(args);
      continue;
    }

    updated_args_count = arg_count(updated_args);

    if ((app_flags & X_FLAG)) {
      fprintf(stdout, "+ %s\n", updated_args[0]);
      fflush(stdout);
    }
    status = sh_execute(updated_args, &keep_line);

    if (keep_line) {
      line_list_push_tail(&line_linked_list, line_duplicate);
    } else {
      free(line_duplicate);
    }

    free(line);
    free(args);  // free's indicate we will return a pointer

    for (int i = 0; i < updated_args_count; i++) {
      free(updated_args[i]);
    }

    free(updated_args);

    switch (status) {
      case END_SH_LOOP:
        sh_exit();
        break;
      case INVALID_PIPE:
        fprintf(stderr, "Error, invalid pipe command\n");
        fflush(stderr);
        fflush(stdout);
        break;
      case REDIRECT_ERROR:
        fprintf(stderr, "Error, unable to redirect input/output\n");
        fflush(stderr);
        fflush(stdout);
        break;
      default:
        break;
    }
  }
}

int sh_execute(char** args, char* keep) {
  uint8_t argsc = 0;
  int async = FALSE;

  while (args[argsc]) {
    argsc++;
  }

  if (args[0] == NULL) {
    return CONT_SH_LOOP;
  }

  if (strcmp(args[0], "exit") == 0) {
    return END_SH_LOOP;
  }
  if (strcmp(args[0], "clear") == 0) {
    printf("\x1b[2J\x1b[1H");
    return CONT_SH_LOOP;
  }
  if (strcmp(args[0], "toggle_echo") == 0) {
    toggle_echo();
    return CONT_SH_LOOP;
  }
  if (strcmp(args[0], "history") == 0) {
    sh_print_linked_list();
    return CONT_SH_LOOP;
  }
  if (strcmp(args[0], "pwd") == 0) {
    char* cwd = getcwd(NULL, 0);
    printf("%s\n", cwd);
    free(cwd);
    return CONT_SH_LOOP;
  }
  if (strcmp(args[0], "cd") == 0) {
    if (argsc > 2) {
      fprintf(stderr, "Too many arguments for cd command...asshole...\n");
      return CONT_SH_LOOP;
    }

    if (argsc < 2) {
      char* home = getenv("HOME");

      if (chdir(home) == -1) {
        fprintf(stderr, "Unable to chdir tp %s: %s\n", home, strerror(errno));
        fflush(stderr);
      }

      return CONT_SH_LOOP;
    }

    if (chdir(args[1]) == -1) {
      fprintf(stderr, "Unable to chdir tp %s: %s\n", args[1], strerror(errno));
      fflush(stderr);
    }
    return CONT_SH_LOOP;
  }

  *keep = 1;
  // for IO redirect we need to recognize "<" and ">" characters.

  // args_end will tell us where the last of the command arguments are.

  Pipe_cmd pipeCmd;
  memset(&pipeCmd, 0, sizeof(pipeCmd));

  int last_position = strlen(args[argsc - 1]) - 1;
  char* last_char = &args[argsc - 1][last_position];

  if (*last_char == '&') {
    *last_char = '\0';
    async = TRUE;
  }

  pipeCmd.args_length = argsc;
  pipeCmd.args = args;

  // parse the section that will be run
  for (int i = 0; i < argsc; i++) {
    if (strcmp(args[i], "|") == 0) {
      pipeCmd.pipe_locations[pipeCmd.pipe_count] = i;
      pipeCmd.pipe_count++;
    }
  }

  return sh_launch_pipe_version(pipeCmd, async);
}
