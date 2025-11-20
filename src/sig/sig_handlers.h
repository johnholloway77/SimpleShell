//
// Created by jholloway on 11/16/25.
//

#ifndef SIMPLESHELL_SIG_HANDLERS_H
#define SIMPLESHELL_SIG_HANDLERS_H

#include <signal.h>
int init_handlers(void);
int reset_handlers(void);

int init_reap();
int stop_reap();
int reap_background_jobs(void);
int add_bg_job(pid_t pid, const char *cmd);

#endif  // SIMPLESHELL_SIG_HANDLERS_H
