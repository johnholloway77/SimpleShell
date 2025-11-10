//
// Created by jholloway on 10/30/25.
//

#include <string.h>

#include "./sh_src.h"
#include "single_linked_list.h"
#include "strtrim.h"

#define UNUSED(x) (void)(x)
DEFINE_SLIST(line_list, char*)

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
}

void sh_print_linked_list() {
  line_list_foreach(&line_linked_list, print_lines, NULL);
}

void sh_loop(void) {
  char* line;
  char** args;
  int status;

  do {
    char keep_line = 0;

    printf("JHsh$: ");
    line = sh_read_line();
    line = strtrim(line);
    char* line_duplicate = strndup(line, strlen(line));

    args = sh_split_line(line);
    status = sh_execute(args, &keep_line);

    if (keep_line) {
      line_list_push_tail(&line_linked_list, line_duplicate);
    } else {
      free(line_duplicate);
    }

    free(line);
    free(args);  // free's indicate we will return a pointer

  } while (status != END_SH_LOOP);

  sh_exit();
}

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
  char* save = NULL;
  char** tokens = malloc(bufsize * sizeof(char*));
  char** token_backup;
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
      token_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        free(tokens);
        fprintf(stderr, "jhsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
     // free(token_backup);
    }

    token = strtok(NULL, SH_TOK_DELIM);
  }
  tokens[position] = NULL;

  return tokens;
}

int sh_execute(char** args, char* keep) {
  if (args[0] == NULL) {
    return 1;
  }

  if (strcmp(args[0], "exit") == 0) {
    printf("Exiting shell\n");
    return END_SH_LOOP;
  }
  if (strcmp(args[0], "clear") == 0) {
    printf("\x1b[2J\x1b[1H");
    return 1;
  }
  if (strcmp(args[0], "toggle_echo") == 0) {
    toggle_echo();
    return 1;
  }
  if (strcmp(args[0], "history") == 0) {
    sh_print_linked_list();
    return 1;
  }
  if (strcmp(args[0], "test") == 0) {
    if (!line_iterator) {
      line_iterator = line_linked_list.head;
    }
    printf("value: %s\n", line_list_node_get_value(line_iterator));
    return 1;
  }
  if (strcmp(args[0], "increment") == 0) {
    // line_list_node_increment(&line_iterator);
    line_list_increment(&line_iterator, &line_linked_list);
    return 1;
  }
  if (strcmp(args[0], "decrement") == 0) {
    // line_list_node_decrement(&line_iterator);
    line_list_decrement(&line_iterator, &line_linked_list);
    return 1;
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

      (void)execv(full_path, args);

      free(full_path);

    } else {
      execv(args[0], args);
      ;
    }
  } else {
    waitpid(pid, 0, 0);
  }
  return 1;
}