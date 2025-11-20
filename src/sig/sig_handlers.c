//
// Created by jholloway on 11/16/25.
//

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "sig_handlers.h"

static volatile uint32_t reap_needed = 0;

typedef struct {
    pid_t pid;
    char *cmd;
} background_job;

static background_job bg_jobs[128];
static uint32_t bg_count = 0;

 int add_bg_job(pid_t pid, const char *cmd){
    if (bg_count >= 128){
        return -1;
    }

    bg_jobs[bg_count].pid = pid;
    bg_jobs[bg_count].cmd = strdup(cmd);
    bg_count++;

    return 0;
}

static void remove_bg_job_at(uint32_t i){
    free(bg_jobs[i].cmd);
    bg_jobs[i] = bg_jobs[bg_count - 1];
    bg_count--;


}

int reap_background_jobs(void) {
    if (!reap_needed){
        return 0;
    }

    reap_needed = 0;

    for (uint32_t i = 0; i < bg_count; ) {
        int status;
        pid_t pid = bg_jobs[i].pid;
        pid_t pid_return = waitpid(pid, &status, WNOHANG);

        if (pid_return == 0) {
            //child process is still running
            i++;
        } else if (pid_return == pid) {
            //child process is finished
            int code;
            if(WIFEXITED(status)){
                code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                code = 128 + WTERMSIG(status);
            } else {
                code = 0;
            }

            char buf[64];
            memset(buf, 0, 64);
            snprintf(buf, sizeof(buf), "%d", code);
            printf("[%d] %s completed with status %d\n", pid, bg_jobs[i].cmd, code);
            fflush(stdout);
            remove_bg_job_at(i);
        } else {
            //pid_return = -1, child already reaped
            remove_bg_job_at(i);
        }

    }

    return 0;
}

static void sigchld_handler(int signo){
    (void)signo;
    reap_needed++;
}

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

    struct sigaction sa2;
    sa2.sa_handler = sigchld_handler;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa2, NULL) == -1) {
        (void)fprintf(stderr, "Unable to setup sighandler SIGCHLD\n");
        return -1;
    }


    return 0;
}

int reset_handlers(void) {
  struct sigaction sa;
  sa.sa_handler = SIG_DFL;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    (void)fprintf(stderr, "Unable to reset sighandler for SIGINT\n");
    return -1;
  }

  if (sigaction(SIGQUIT, &sa, NULL) == -1) {
    (void)fprintf(stderr, "Unable to reset sighandler SIGQUIT\n");
    return -1;
  }

    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        (void)fprintf(stderr, "Unable to reset sighandler SIGTSTP\n");
        return -1;
    }

  return 0;
}