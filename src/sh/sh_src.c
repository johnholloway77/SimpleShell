//
// Created by jholloway on 10/30/25.
//
#include <limits.h>
#include <signal.h>
#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../flags/flags.h"
#include "../flags/set_flags.h"
#include "../macro_linked_list.h"
#include "../sig/sig_handlers.h"
#include "../str/strtrim.h"
#include "./sh_src.h"
#include "sh_lines.h"

#define UNUSED(x) (void)(x)
#define FALSE 0
#define TRUE 1
#define SUCCESS_PIPE 2
#define INVALID_PIPE 3
#define PIPE_FAILURE 4
#define MAX_PIPE_COUNT 16

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

    switch (status) {
      case END_SH_LOOP:
        sh_exit();
        break;
      case INVALID_PIPE:
        fprintf(stderr, "Error, invalid pipe command\n");
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

  Pipe_cmd pipeCmd;
  memset(&pipeCmd, 0, sizeof(pipeCmd));

  int last_position = strlen(args[argsc - 1]) - 1;
  char* last_char = &args[argsc - 1][last_position];

  if (*last_char == '&') {
    *last_char = '\0';
    async = TRUE;
  }

  Redirect_str redirectStr;

  sh_redirect_init(&redirectStr);

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

int sh_launch(char** args, int async) {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    if (reset_handlers() == -1) {
      (void)fprintf(stderr, "Error resetting signal handlers for child\n");
    }

    if (execvp(args[0], args) < 0) {
      fprintf(stderr, "Error:%s : %s\n", args[0], strerror(errno));
      _exit(EXIT_FAILURE);
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

int sh_launch_pipe_version(Pipe_cmd pipeCmd, int async) {
  // error checking for token before and after |
  int pipe_array[MAX_PIPE_COUNT][2];
  pid_t pids[MAX_PIPE_COUNT + 1];

  for (int i = 0; i < pipeCmd.pipe_count; i++) {
    int pipe_index = pipeCmd.pipe_locations[i];

    if (pipe_index == 0) {
      // pipe must be at least one token in
      return INVALID_PIPE;
    }

    if (pipeCmd.args[pipe_index + 1] == NULL) {
      // must have token after pipe

      return INVALID_PIPE;
    }

    if (strcmp(pipeCmd.args[pipe_index + 1], "|") == 0) {
      // cannot have to pipes tokens in a row.
      return INVALID_PIPE;
    }
  }

  // remove the | tokens
  for (int i = 0; i < pipeCmd.pipe_count; i++) {
    int pipe_index = pipeCmd.pipe_locations[i];

    pipeCmd.args[pipe_index] = NULL;
  }

  for (int p = 0; p < pipeCmd.pipe_count; ++p) {
    if (pipe(pipe_array[p]) == -1) {
      return PIPE_FAILURE;
    }
  }

  int cmd_count = pipeCmd.pipe_count + 1;

  for (int i = 0; i < cmd_count; i++) {
    //

    // we're lazy so we're limiting the shell to receive 17 tokens
    // each token is defined as the arguments separated by pipes
    char* tokens[MAX_PIPE_COUNT + 1] = {0};

    int iterator;

    if (i == 0) {
      iterator = 0;
    } else {
      iterator = pipeCmd.pipe_locations[i - 1] + 1;
    }

    int token_counter = 0;
    while (pipeCmd.args[iterator]) {
      tokens[token_counter] = pipeCmd.args[iterator];
      token_counter++;
      iterator++;
    }
    tokens[token_counter] = NULL;

#ifdef DEBUGMODE
    int test = 0;
    while (tokens[test]) {
      printf("%s ", tokens[test]);
      test++;
    }
    printf("\n");
#endif

    int redirect = FALSE;
    char** cmd_args = NULL;
    int cmd_args_count = 0;
    int cmd_args_end = 0;

    // parse the section that will be run
    for (int l = 0; l < token_counter; l++) {
      if (((strcmp(tokens[l], ">") == 0) || (strcmp(tokens[l], ">>") == 0) ||
           (strcmp(tokens[l], "<") == 0))) {
        redirect = TRUE;
        cmd_args_end = l - 1;

        cmd_args = malloc((l + 1) * sizeof(char*));

        for (int j = 0; j <= cmd_args_end; j++) {
          cmd_args[j] = tokens[j];
        }
        cmd_args[l] = NULL;
        break;
      }

      cmd_args_count++;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FORK HERE
    /////////////////////////////////////

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      for (int p = 0; p < pipeCmd.pipe_count; ++p) {
        close(pipe_array[p][0]);
        close(pipe_array[p][1]);
      }
      return PIPE_FAILURE;
    }

    if (pid == 0) {
      if (pipeCmd.pipe_count > 0) {
        if (i == 0) {
          // first command
          dup2(pipe_array[i][1], STDOUT_FILENO);
        } else if (i == cmd_count - 1) {
          // last command
          dup2(pipe_array[pipeCmd.pipe_count - 1][0], STDIN_FILENO);

        } else {
          // middle commands
          dup2(pipe_array[i - 1][0], STDIN_FILENO);
          dup2(pipe_array[i][1], STDOUT_FILENO);
        }
      }

      for (int p = 0; p < pipeCmd.pipe_count; p++) {
        close(pipe_array[p][0]);
        close(pipe_array[p][1]);
      }

      ///////////////////////////////////////////////////
      // redirect
      //////////////////////////////////////////////////
      if (redirect && cmd_args != NULL) {
        Redirect_str redirectStr;
        sh_redirect_init(&redirectStr);

        for (int l = token_counter - 1; l >= 0; l--) {
          if (strcmp(tokens[l], "<") == 0) {
            if (!tokens[l + 1]) {
              fprintf(stderr, "invalid redirect. Missing input for <\n");
              sh_restore_fd(&redirectStr);
              free(cmd_args);
              return -1;
            }

            redirectStr.redir_in = TRUE;

            if (tokens[l + 1]) {
              if (redirectStr.fd_in != -1) {
                close(redirectStr.fd_in);
              }

              if ((redirectStr.fd_in =
                       open(tokens[l + 1], O_RDONLY | O_NONBLOCK)) < 0) {
                sh_restore_fd(&redirectStr);
                free(cmd_args);
                fprintf(stderr, "Could not open file: %s\nerror: %s\n",
                        tokens[l + 1], strerror(errno));
                return -1;
              }

            } else {
              fprintf(stderr, "Invalid redirection. Need source.");
            }
          }
        }

        // redirect output to file
        // goes left to right with rightmost redirect being the dominant one.
        for (int l = 0; l < token_counter; l++) {
          if (strcmp(tokens[l], ">") == 0) {
            if (l == 0 || !tokens[l + 1]) {
              fprintf(
                  stderr,
                  "invalid redirect. Missing input or output for > redirect\n");
              sh_restore_fd(&redirectStr);
              free(cmd_args);
              return -1;
            }

            if (tokens[l + 1]) {
              redirectStr.redir_out = TRUE;

              if (redirectStr.fd_out != -1) {
                close(redirectStr.fd_out);
              }

              if ((redirectStr.fd_out = open(
                       tokens[l + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666)) ==
                  -1) {
                sh_restore_fd(&redirectStr);
                free(cmd_args);
                fprintf(stderr, "could not open file %s\n", tokens[l + 1]);
                return -1;
              }
            }
          }

          if (strcmp(tokens[l], ">>") == 0) {
            if (l == 0 || !tokens[l + 1]) {
              fprintf(stderr,
                      "invalid redirect. Missing input or output for >> "
                      "redirect\n");
              sh_restore_fd(&redirectStr);
              free(cmd_args);
              return -1;
            }

            if (tokens[l + 1]) {
              redirectStr.redir_out = TRUE;
              if (redirectStr.fd_out != -1) {
                close(redirectStr.fd_out);
              }

              if ((redirectStr.fd_out = open(
                       tokens[l + 1], O_WRONLY | O_CREAT | O_APPEND, 0666)) ==
                  -1) {
                sh_restore_fd(&redirectStr);
                free(cmd_args);
                fprintf(stderr, "could not open file %s\n", tokens[l + 1]);
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
          // int ret_val = sh_launch(cmd_args, async);

          pid_t pid2 = fork();
          if (pid2 < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
          }

          if (pid2 == 0) {
            if (reset_handlers() == -1) {
              (void)fprintf(stderr,
                            "Error resetting signal handlers for child\n");
            }

            if (execvp(cmd_args[0], cmd_args) < 0) {
              fprintf(stderr, "Error:%s : %s\n", cmd_args[0], strerror(errno));
              _exit(EXIT_FAILURE);
            }

          } else {
            int return_val = -1;
            waitpid(pid2, &return_val, 0);
            exit(EXIT_SUCCESS);
          }

          // int ret_val = printf("redirect launch here\n");

          sh_close_fd(&redirectStr);
          sh_restore_fd(&redirectStr);

          return 1;
        }

        if (cmd_args_count > 0) {
          free(cmd_args);
        }

        // return sh_launch(tokens, async);
        printf("redirect launch here\n");
      } else {
        ///////////////////////////////
        //         no redirect
        ////////////////////////////

        if (execvp(tokens[0], tokens) < 0) {
          fprintf(stderr, "Error:%s : %s\n", tokens[0], strerror(errno));
          _exit(EXIT_FAILURE);
        }
      }

      if (cmd_args != NULL) {
        free(cmd_args);
      }

    } else {
      pids[i] = pid;
    }
  }

  for (int p = 0; p < pipeCmd.pipe_count; p++) {
    close(pipe_array[p][0]);
    close(pipe_array[p][1]);
  }

  if (async) {
    // background job
    add_bg_job(pids[0], pipeCmd.args[0]);
  } else {
    // foreground job

    int status = 0;
    int last_status = 0;

    for (int i = 0; i < cmd_count; i++) {
      if (waitpid(pids[i], &status, 0) > 0) {
        last_status = status;
      }
    }

    char return_buff[32];
    snprintf(return_buff, sizeof(return_buff), "%d", WEXITSTATUS(last_status));
    setenv("?", return_buff, 1);
  }

  return SUCCESS_PIPE;
}