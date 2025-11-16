//
// Created by jholloway on 10/30/25.
//

#include <termios.h>
#include <unistd.h>
#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1
#endif

#include <string.h>
#include <sys/param.h>

#include "../flags/flags.h"
#include "../flags/set_flags.h"
#include "../macro_linked_list.h"
#include "../str/strl.h"
#include "../str/strtrim.h"
#include "./sh_src.h"
#include "sh_lines.h"

#define UNUSED(x) (void)(x)

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

    updated_args = update_args(args, envp);

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
  uint8_t argc = 0;
  while (args[argc]) {
    argc++;
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
    if (argc > 2) {
      fprintf(stderr, "Too many arguments for cd command...asshole...\n");
      return CONT_SH_LOOP;
    }

    if (argc < 2) {
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

  // for IO redirect we need to recognize "<" and ">" characters.
  *keep = 1;
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
    if (args[0][0] != '/') {
      size_t size = strlen(temp_path) + strlen(args[0]) + 1;
      full_path = malloc(size);
      memset(full_path, 0, size);
      strlcpy(full_path, temp_path, size);
      strlcat(full_path, args[0], size);

      if (app_flags & X_FLAG) {
        printf("- %s\n", args[0]);
      }

      (void)execv(full_path, args);

      free(full_path);

    } else {
      execv(args[0], args);
    }
  } else {
    waitpid(pid, 0, 0);
  }
  return 1;
}