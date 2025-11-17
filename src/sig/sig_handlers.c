//
// Created by jholloway on 11/16/25.
//

#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>

#include "sig_handlers.h"


void reap(int signo) {
  (void)signo;
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

int init_handlers(void) {
  struct sigaction sa;
  struct sigaction sa2;

  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sa2.sa_handler = reap;
  sigemptyset(&sa2.sa_mask);
  sa2.sa_flags = SA_RESTART;

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

  if (sigaction(SIGCHLD, &sa2, NULL) == -1) {
    (void)fprintf(stderr, "unable to setup sighandler for SIGCHLD\n");
    return -1;
  }

  return 0;
}