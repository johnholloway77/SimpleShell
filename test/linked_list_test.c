//
// Created by jholloway on 11/10/25.
//

#include <criterion/criterion.h>

#include "../src/macro_linked_list.h"

Test(line_list_symbols, ensure_mapping_exists) {
  volatile void* p;
  p = (void*)line_list_init;
  p = (void*)line_list_push_front;
  p = (void*)line_list_push_tail;
  p = (void*)line_list_increment;
  p = (void*)line_list_decrement;
  p = (void*)line_list_clear;
  (void)p;
}

Test(line_list_init, macro_creation_test_line_list_init) {
  cr_expect_not_null(line_list_init);
}

Test(line_list_init, line_list_init_use_test) {
  line_list__node* line_iterator;
  line_list line_linked_list;

  line_list_init(&line_linked_list);

  cr_expect_null(line_linked_list.head);
  cr_expect_null(line_linked_list.tail);
}