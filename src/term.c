//
// Created by jholloway on 10/30/25.
//

#include "term.h"

static struct termios old_term;

static void die(const char* msg) {
  perror(msg);
  tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
  exit(EXIT_FAILURE);
}

void set_raw(void) {
  if (tcgetattr(STDIN_FILENO, &old_term) == -1) {
    die("tcgetattr error - set_raw()");
  }
  struct termios raw = old_term;
  //    raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_lflag &= ~(ICANON);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
    die("tcsetattr error - set_raw()");
  }
}

void toggle_echo(void) {
  struct termios current_term;
  if (tcgetattr(STDIN_FILENO, &current_term) == -1) {
    die("tcgetattr error - toggle_echo()");
  }

  current_term.c_lflag ^= ECHO;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &current_term) == -1) {
    die("tcsetattr error - toggle_echo()");
  }
}

void restore(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

static void on_down(void) {
  printf("Down\n");
  fflush(stdout);
}

static void on_up(void) {
  printf("Up\n");
  fflush(stdout);
}

static void on_left(void) {
  printf("left\n");
  fflush(stdout);
}

static void on_right(void) {
  printf("Right\n");
  fflush(stdout);
}
