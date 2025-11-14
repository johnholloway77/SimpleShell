//
// Created by jholloway on 10/30/25.
//

#ifndef SIMPLESHELL_TERM_H
#define SIMPLESHELL_TERM_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

void restore(void);
void set_raw(void);

static void die(const char* msg);
void toggle_echo(void);
static void on_down(void);
static void on_up(void);
static void on_left(void);
static void on_right(void);

#endif  // SIMPLESHELL_TERM_H
