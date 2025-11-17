//
// Created by jholloway on 10/30/25.
//
#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <termios.h>
#include <unistd.h>

#include "../flags/flags.h"
#include "../flags/set_flags.h"
#include "../macro_linked_list.h"
#include "../str/strl.h"
#include "../str/strtrim.h"
#include "./sh_src.h"
#include "sh_lines.h"

#define UNUSED(x) (void)(x)
#define TRUE 1
#define FALSE 0

line_list__node* line_iterator;
line_list line_linked_list;

static void print_lines(char** p_line, void* x) {
  UNUSED(x);
  char* line = *p_line;

  if (!line) {
    return;
  }
  printf("%s\n", line);
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

static int saved_stdin;
static int saved_stdout;
static int saved_stderr;

void sh_backup_fd(void) {
  saved_stdin = dup(STDIN_FILENO);
  saved_stdout = dup(STDOUT_FILENO);
  saved_stderr = dup(STDERR_FILENO);
}

void sh_restore_fd(void) {
  dup2(saved_stdin, STDIN_FILENO);
  dup2(saved_stdout, STDOUT_FILENO);
  dup2(saved_stderr, STDERR_FILENO);
}

void sh_print_linked_list() {
  line_list_foreach(&line_linked_list, print_lines, NULL);
}

void sh_loop(char** envp) {
  char* line;
  char** args;
  char** updated_args;

  while (1) {
    char keep_line = 0;
    int status;

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

    // status = sh_execute(args, &keep_line);
    status = sh_execute(updated_args, &keep_line);

    if (keep_line) {
      line_list_push_tail(&line_linked_list, line_duplicate);
    } else {
      free(line_duplicate);
    }

    free(line);
    free(args);  // free's indicate we will return a pointer

    int i = 0;
    while (updated_args[i]) {
      free(updated_args[i]);
      i++;
    }

    free(updated_args);

    if (status == END_SH_LOOP) {
      sh_exit();
      break;
    }

    printf("JHsh$: ");
    fflush(stdout);
  }
}

int sh_execute(char** args, char* keep) {
  uint8_t argsc = 0;
  while (args[argsc]) {
    argsc++;
  }

  if (app_flags & X_FLAG) {
    printf("- %s\n", args[0]);
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

  char** cmd_args = NULL;
  int cmd_argsc = 0;
  int args_end = 0;

  int fd_in;
  int fd_out;

  int redir_out = FALSE;
  int redir_in = FALSE;

  sh_backup_fd();

  // parse the section that will be run
  for (int i = 0; i < argsc; i++) {
    if (((strcmp(args[i], ">") == 0) || (strcmp(args[i], ">>") == 0) ||
         (strcmp(args[i], "<") == 0))) {
      args_end = i - 1;

      cmd_args = malloc((i + 1) * sizeof(char*));

      for (int j = 0; j <= args_end; j++) {
        cmd_args[j] = args[j];
      }
      cmd_args[i] = NULL;
      break;
    }
    cmd_argsc++;
  }

  for (int i = argsc - 1; i >= 0; i--) {
    if (strcmp(args[i], "<") == 0) {
      if (i == 0 || !args[i + 1]) {
        fprintf(stderr, "invalid redirect. Missing input or output for <\n");
        sh_restore_fd();
        free(cmd_args);
        return -1;
      }

      redir_in = TRUE;

      if (args[i + 1]) {
        close(fd_in);
        if ((fd_in = open(args[i + 1], O_RDONLY | O_NONBLOCK)) < 0) {
          sh_restore_fd();
          free(cmd_args);
          fprintf(stderr, "Could not open file: %s\nerror: %s\n", args[i + 1],
                  strerror(errno));
          return -1;
        }

      } else {
        fprintf(stderr, "Invalid redirection. Need source.");
      }
    }
  }

  // redirect output to file
  // goes left to right with rightmost redirect being the dominant one.
  for (int i = 0; i < argsc; i++) {
    if (strcmp(args[i], ">") == 0) {
      if (!args[i - 1] || !args[i + 1]) {
        fprintf(stderr,
                "invalid redirect. Missing input or output for > redirect\n");
        sh_restore_fd();
        free(cmd_args);
        return -1;
      }

      redir_out = TRUE;

      if (args[i + 1]) {
        if ((fd_out = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) ==
            -1) {
          sh_restore_fd();
          free(cmd_args);
          fprintf(stderr, "could not open file %s\n", args[i + 1]);
          return -1;
        }
      }
    }

    if (strcmp(args[i], ">>") == 0) {
      if (!args[i - 1] || !args[i + 1]) {
        fprintf(stderr,
                "invalid redirect. Missing input or output for >> redirect\n");
        sh_restore_fd();
        free(cmd_args);
        return -1;
      }

      redir_out = TRUE;

      if (args[i + 1]) {
        if ((fd_out = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0666)) ==
            -1) {
          sh_restore_fd();
          free(cmd_args);
          fprintf(stderr, "could not open file %s\n", args[i + 1]);
          return -1;
        }
      }
    }
  }

  if (redir_out == TRUE) {
    dup2(fd_out, STDOUT_FILENO);
  }

  if (redir_in == TRUE) {
    dup2(fd_in, STDIN_FILENO);
  }

  if (redir_in || redir_out) {
    int ret_val = sh_launch(cmd_args);

    close(fd_out);
    close(fd_in);
    sh_restore_fd();

    if (cmd_args) {
      free(cmd_args);
    }

    return ret_val;
  }

  if (cmd_args) {
    free(cmd_args);
  }

  return sh_launch(args);
}

int sh_launch(char** args) {
  char* temp_path = "/bin/";
  char* full_path = NULL;

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    int evecv_ret_val = 0;

    if (args[0][0] != '/') {
      size_t size = strlen(temp_path) + strlen(args[0]) + 1;
      full_path = malloc(size);
      memset(full_path, 0, size);
      strlcpy(full_path, temp_path, size);
      strlcat(full_path, args[0], size);

      if ((evecv_ret_val = execv(full_path, args)) == -1) {
        fprintf(stderr, "Error: Unknown command: %s : %s\n", args[0],
                strerror(errno));
        exit(EXIT_FAILURE);
      }

      free(full_path);

    } else {
      if ((evecv_ret_val = execv(full_path, args)) == -1) {
        fprintf(stderr, "Error: Unknown command: %s : %s\n", args[0],
                strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
  } else {
    int return_val;
    waitpid(pid, &return_val, 0);

    char buff[32];
    snprintf(buff, 32, "%d", WEXITSTATUS(return_val));
    setenv("?", buff, 1);
  }
  return 1;
}