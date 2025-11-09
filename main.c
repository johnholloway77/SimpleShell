//
// Created by jholloway on 10/30/25.
//

#define _POSIX_C_SOURCE 200809L
#define __BSD_VISIBLE 1

#include "src/sh_src.h"
#include "src/term.h"

int main(int argc, char** argv) {
  set_raw();
  atexit(restore);

  sh_init_linked_list();
  sh_loop();
  return EXIT_SUCCESS;
}
