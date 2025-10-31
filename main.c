//
// Created by jholloway on 10/30/25.
//

#define _POSIX_C_SOURCE 200809L
#define __BSD_VISIBLE 1

#include <stdio.h>
#include <string.h>

#include "src/sh_src.h"
#include "src/single_linked_list.h"
#include "src/term.h"

DEFINE_SLIST(int_list, int)
DEFINE_SLIST(str_list, char*)

#define UNUSED(x) (void)(x)

static void free_cstr(char** s) {
  free(*s);
}
static void print_int(int* x, void *arg) {
  UNUSED(arg);
  printf("%d\n", *x);
}
static void print_str(char** s, void *arg) {
  UNUSED(arg);
  puts(*s);
}

int main(int argc, char** argv) {
  int_list A;
  int_list_init(&A);
  int_list_push_front(&A, 1);
  int_list_push_tail(&A, 2);
  int_list_push_front(&A, 0);
  int_list_push_tail(&A, 3);
  int_list_push_tail(&A, 4);
  int_list_foreach(&A, print_int, NULL);

  str_list b;
  str_list_init(&b);
  str_list_push_tail(&b, strdup("World"));
  str_list_push_front(&b, strdup("Hello, "));
  str_list_foreach(&b, print_str, NULL);
  str_list_clear(&b, free_cstr);

  set_raw();
  atexit(restore);

  sh_loop();
  return EXIT_SUCCESS;
}
