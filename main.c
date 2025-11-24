//
// Created by jholloway on 10/30/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include "src/flags/flags.h"
#include "src/flags/set_flags.h"
#include "src/sh/sh_lines.h"
#include "src/sh/sh_src.h"
#include "src/sig/sig_handlers.h"
#include "src/str/strl.h"

char* cwd = NULL;

int main(int argc, char** argv, char** envp) {
  if (argc > 1 && argv[1][0] == '-') {
    set_flags(argv[1]);
  }

  if (1 == init_handlers()) {
    return -1;
  }

  if (!(app_flags & C_FLAG)) {
    cwd = getcwd(NULL, 0);
    if (!cwd) {
      perror("getcwd");
      exit(EXIT_FAILURE);
    }

    char* app_name = (char*)malloc(MAXPATHLEN);
    memset(app_name, 0, MAXPATHLEN);

    strlcat(app_name, cwd, MAXPATHLEN);

    setenv("SHELL", app_name, 1);

    char buff[32];
    snprintf(buff, sizeof(buff), "%d", getpid());

    setenv("$", buff, 1);

    free(cwd);
    free(app_name);

    sh_init_linked_list();
    sh_loop(envp);
  } else {
    if (argc > 2) {
      char** argv_copy = malloc((argc - 2 + 1) * sizeof(char*));
      int i = 0;
      while (i < argc - 2) {
        argv_copy[i] = strdup(argv[i + 2]);
        i++;
      }
      argv_copy[i] = NULL;

      char** updated_args;
      uint32_t updated_args_count;
      updated_args = update_args(argv_copy, envp);
      updated_args_count = arg_count(updated_args);
      sh_execute(updated_args);

      for (int j = 0; j < updated_args_count; j++) {
        free(updated_args[j]);
      }
      free(updated_args);

      for (int j = 0; j < argc - 2; j++) {
        free(argv_copy[j]);
      }
      free(argv_copy);
    }
  }

  return EXIT_SUCCESS;
}
