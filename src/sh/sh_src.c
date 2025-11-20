//
// Created by jholloway on 10/30/25.
//
#include <signal.h>
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
#include "../sig/sig_handlers.h"
#include "../str/strtrim.h"
#include "./sh_src.h"
#include "sh_lines.h"

#define UNUSED(x) (void)(x)
#define TRUE 1
#define FALSE 0

static line_list__node* line_iterator;
static line_list line_linked_list;

Path_struct* ps;

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

void sh_redirect_init(Redirect_str* redir) {
  redir->redir_out = FALSE;
  redir->redir_in = FALSE;
  redir->redir_err = FALSE;
  redir->saved_stdin = dup(STDIN_FILENO);
  redir->saved_stdout = dup(STDOUT_FILENO);
  redir->saved_stderr = dup(STDERR_FILENO);
  redir->fd_out = -1;
  redir->fd_in = -1;
  redir->fd_err = -1;
}

void sh_restore_fd(Redirect_str* redir) {
  if (redir->redir_in == TRUE) {
    dup2(redir->saved_stdin, STDIN_FILENO);
  }

  if (redir->redir_out) {
    dup2(redir->saved_stdout, STDOUT_FILENO);
  }
  if (redir->redir_err) {
    dup2(redir->saved_stderr, STDERR_FILENO);
  }
}

void sh_close_fd(Redirect_str* redir) {
  if (redir->fd_in != -1) {
    close(redir->fd_in);
  }

  if (redir->fd_out != -1) {
    close(redir->fd_out);
  }

  if (redir->fd_err != -1) {
    close(redir->fd_err);
  }
}

void sh_print_linked_list() {
  line_list_foreach(&line_linked_list, print_lines, NULL);
}

void sh_loop(char** envp) {
  char* line;
  char** args;
  char** updated_args;

  char* paths = getenv("PATH");
  // printf("path: %s\n", paths);
  ps = sh_parse_path_line(paths);

  while (1) {
    char keep_line = 0;
    int status;

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
  int async = FALSE;

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

  int last_position = strlen(args[argsc - 1]) - 1;
  char* last_char = &args[argsc - 1][last_position];

  if (*last_char == '&') {
    *last_char = '\0';
    async = TRUE;
  }

  Redirect_str redirectStr;

  sh_redirect_init(&redirectStr);

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
      if (!args[i + 1]) {
        fprintf(stderr, "invalid redirect. Missing input for <\n");
        sh_restore_fd(&redirectStr);
        free(cmd_args);
        return -1;
      }

      redirectStr.redir_in = TRUE;

      if (args[i + 1]) {
        if (redirectStr.fd_in != -1) {
          close(redirectStr.fd_in);
        }

        if ((redirectStr.fd_in = open(args[i + 1], O_RDONLY | O_NONBLOCK)) <
            0) {
          sh_restore_fd(&redirectStr);
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
      if (i == 0 || !args[i + 1]) {
        fprintf(stderr,
                "invalid redirect. Missing input or output for > redirect\n");
        sh_restore_fd(&redirectStr);
        free(cmd_args);
        return -1;
      }

      if (args[i + 1]) {
        redirectStr.redir_out = TRUE;

        if (redirectStr.fd_out != -1) {
          close(redirectStr.fd_out);
        }

        if ((redirectStr.fd_out =
                 open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
          sh_restore_fd(&redirectStr);
          free(cmd_args);
          fprintf(stderr, "could not open file %s\n", args[i + 1]);
          return -1;
        }
      }
    }

    if (strcmp(args[i], ">>") == 0) {
      if (i == 0 || !args[i + 1]) {
        fprintf(stderr,
                "invalid redirect. Missing input or output for >> redirect\n");
        sh_restore_fd(&redirectStr);
        free(cmd_args);
        return -1;
      }

      if (args[i + 1]) {
        redirectStr.redir_out = TRUE;
        if (redirectStr.fd_out != -1) {
          close(redirectStr.fd_out);
        }

        if ((redirectStr.fd_out = open(
                 args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1) {
          sh_restore_fd(&redirectStr);
          free(cmd_args);
          fprintf(stderr, "could not open file %s\n", args[i + 1]);
          return -1;
        }
      }
    }
  }

  if (redirectStr.redir_out == TRUE) {
    dup2(redirectStr.fd_out, STDOUT_FILENO);
  }

  if (redirectStr.redir_in == TRUE) {
    dup2(redirectStr.fd_in, STDIN_FILENO);
  }

  if (redirectStr.redir_in || redirectStr.redir_out) {
    int ret_val = sh_launch(cmd_args, async);

    sh_close_fd(&redirectStr);
    sh_restore_fd(&redirectStr);

    if (cmd_args) {
      free(cmd_args);
    }

    return ret_val;
  }

  if (cmd_args) {
    free(cmd_args);
  }

  return sh_launch(args, async);
}

int sh_launch(char** args, int async) {
  char buff[MAXPATHLEN];
  memset(buff, 0, MAXPATHLEN);

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    if (reset_handlers() == -1) {
      (void)fprintf(stderr, "Error resetting signal handlers for child\n");
    }

    if (args[0][0] != '/') {
      snprintf(buff, MAXPATHLEN, "./%s", args[0]);

      fflush(stdout);
      execv(buff, args);

      for (int i = 0; i < ps->count; i++) {
        snprintf(buff, MAXPATHLEN, "%s/%s", ps->path_options[i], args[0]);
        fflush(stdout);
        execv(buff, args);
      }

      fprintf(stderr, "Error:%s : %s\n", args[0], strerror(errno));
      _exit(EXIT_FAILURE);
    } else {
      fflush(stdout);
      if ((execv(args[0], args)) == -1) {
        fprintf(stderr, "Error: Unknown command: %s : %s\n", args[0],
                strerror(errno));
        _exit(EXIT_FAILURE);
      }
    }
  } else {
    if (async) {
      add_bg_job(pid, args[0]);
    } else {
      int return_val = -1;
      waitpid(pid, &return_val, 0);

      char return_buff[32];
      snprintf(return_buff, 32, "%d", WEXITSTATUS(return_val));
      setenv("?", return_buff, 1);
    }
  }
  return 1;
}