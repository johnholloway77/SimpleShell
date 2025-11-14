//
// Created by jholloway on 10/30/25.
//

#include <stdint.h>

#include "src/flags.h"
#define _POSIX_C_SOURCE 200809L
#define __BSD_VISIBLE 1

#include "src/set_flags.h"
#include "src/sh_src.h"
#include "src/term.h"

int main(int argc, char** argv) {
  if (argc > 1 && argv[1][0] == '-') {
    set_flags(argv[1]);
  }

  if (!(app_flags & C_FLAG)) {
    set_raw();
    atexit(restore);

    sh_init_linked_list();
    printf("JHsh$: ");

    fflush(stdout);
    sh_loop();
  } else {
    if (argc > 2) {
      sh_launch(&argv[2]);
    }
  }

  return EXIT_SUCCESS;
}
