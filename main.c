//
// Created by jholloway on 10/30/25.
//

#include <stdio.h>
#define _POSIX_C_SOURCE 200809L
#define __BSD_VISIBLE 1

#include <stdlib.h>
#include <sys/param.h>
#include <unistd.h>

#include "src/flags/flags.h"
#include "src/flags/set_flags.h"
#include "src/sh/sh_src.h"
#include "src/sig/sig_handlers.h"
#include "src/str/strl.h"
#include "src/term/term.h"

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

    char* argv0 = argv[0];
    char* slash = strrchr(argv0, '/');
    if (slash) {
      argv0 = slash + 1;
    }

    strlcat(app_name, cwd, MAXPATHLEN);

    setenv("SHELL", app_name, 1);

    char buff[32];
    snprintf(buff, sizeof(buff), "%d", getpid());

    setenv("$", buff, 1);

    free(cwd);
    free(app_name);

    // set_raw();
    // atexit(restore);

    sh_init_linked_list();
    printf("JHsh$: ");

    fflush(stdout);
    sh_loop(envp);
  } else {
    if (argc > 2) {
      sh_launch(&argv[2], 0);
    }
  }

  return EXIT_SUCCESS;
}
