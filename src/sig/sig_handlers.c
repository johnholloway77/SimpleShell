//
// Created by jholloway on 11/16/25.
//

#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

#include "sig_handlers.h"

int init_handlers(void) {
  struct sigaction sa;

  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    (void)fprintf(stderr, "Unable to setup sighandler for SIGINT\n");
    return -1;
  }

  if (sigaction(SIGQUIT, &sa, NULL) == -1) {
    (void)fprintf(stderr, "Unable to setup sighandler SIGQUIT\n");
    return -1;
  }

  if (sigaction(SIGTSTP, &sa, NULL) == -1) {
    (void)fprintf(stderr, "Unable to setup sighandler SIGSTP\n");
    return -1;
  }

  return 0;
}