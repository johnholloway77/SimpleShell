////
//// Created by jholloway on 11/10/25.
////

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <string.h>

#include "../src/macro_linked_list.h"
#include "../src/str/strl.h"

static int count = 0;

void free_chars(char** c) {
  free(*c);
}

void do_nothing(char** c) {
  // meh
}

void count_strings(char** c, void* x) {
  (void)(x);

  if (c) {
    count++;
  }
}

Test(line_list_symbols, ensure_mapping_exists) {
  volatile void* p;

  p = (void*)line_list_init;
  cr_expect_not_null(p);
  (void)p;

  p = (void*)line_list_push_front;
  cr_expect_not_null(p);
  (void)p;

  p = (void*)line_list_push_tail;
  cr_expect_not_null(p);
  (void)p;

  p = (void*)line_list_increment;
  cr_expect_not_null(p);
  (void)p;

  p = (void*)line_list_decrement;
  cr_expect_not_null(p);
  (void)p;

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

Test(line_list_init, get_value_test) {
  line_list line_linked_list;

  char* string = "This string is pushed first";

  line_list_init(&line_linked_list);

  cr_expect_null(line_linked_list.head);
  cr_expect_null(line_linked_list.tail);

  line_list__node* line_iterator_1 = line_linked_list.head;
  cr_expect_null(line_iterator_1);

  line_list_push_front(&line_linked_list, string);

  cr_expect_not_null(line_linked_list.head);
  cr_expect_not_null(line_linked_list.tail);

  line_list__node* line_iterator = line_linked_list.head;

  cr_expect_not_null(line_iterator);

  char* string2 = line_list_get_value(line_iterator);

  cr_expect_str_eq(string, string2);

  // Can't free as it is declared on and pointing to a
  // char* on the stack
  // free(string2);
}

Test(line_list_init, ll_iterator_tests) {
  char* string1 = "This string is pushed first";
  char* string2 = "This string is pushed second";
  char* string3 = "This string is pushed third";

  line_list line_linked_list;
  line_list_init(&line_linked_list);

  line_list_push_front(&line_linked_list, string1);
  line_list_push_tail(&line_linked_list, string2);
  line_list_push_tail(&line_linked_list, string3);

  line_list__node* line_iterator = line_linked_list.head;
  cr_expect_not_null(line_iterator);

  char* string_return_1 = line_list_get_value(line_iterator);
  cr_expect_str_eq(string1, string_return_1);

  line_list_increment(&line_iterator, &line_linked_list);
  cr_expect_not_null(line_iterator);
  char* string_return_2 = line_list_get_value(line_iterator);
  cr_expect_str_eq(string2, string_return_2);

  line_list_increment(&line_iterator, &line_linked_list);
  cr_expect_not_null(line_iterator);
  char* string_return_3 = line_list_get_value(line_iterator);
  cr_expect_str_eq(string3, string_return_3);

  line_list_increment(&line_iterator, &line_linked_list);
  cr_expect_null(line_iterator);
  char* string_return_x = line_list_get_value(line_iterator);
  cr_expect_null(string_return_x);

  line_list_decrement(&line_iterator, &line_linked_list);
  cr_expect_not_null(line_iterator);
  char* string_return_3a = line_list_get_value(line_iterator);
  cr_expect_str_eq(string3, string_return_3a);

  line_list_decrement(&line_iterator, &line_linked_list);
  cr_expect_not_null(line_iterator);
  char* string_return_2a = line_list_get_value(line_iterator);
  cr_expect_str_eq(string2, string_return_2a);

  line_list_decrement(&line_iterator, &line_linked_list);
  cr_expect_not_null(line_iterator);
  char* string_return_1a = line_list_get_value(line_iterator);
  cr_expect_str_eq(string1, string_return_1a);

  line_list_decrement(&line_iterator, &line_linked_list);
  cr_expect_null(line_iterator);
  char* string_return_xa = line_list_get_value(line_iterator);
  cr_expect_null(string_return_xa);
}

Test(line_list_init, clear_all_test) {
  size_t buffer_length = 50;

  char* string1 = (char*)malloc(buffer_length);
  char* string2 = (char*)malloc(buffer_length);
  char* string3 = (char*)malloc(buffer_length);

  char* string_a = "This string is pushed first";
  char* string_b = "This string is pushed second";
  char* string_c = "This string is pushed third";

  strlcpy(string1, string_a, buffer_length);
  strlcpy(string2, string_b, buffer_length);
  strlcpy(string3, string_c, buffer_length);

  cr_assert_str_eq(string_a, string1);
  cr_assert_str_eq(string_b, string2);
  cr_assert_str_eq(string_c, string3);

  line_list line_linked_list;
  line_list_init(&line_linked_list);

  line_list_push_tail(&line_linked_list, string1);
  line_list_push_tail(&line_linked_list, string2);
  line_list_push_tail(&line_linked_list, string3);

  line_list__node* line_iterator = line_linked_list.head;
  cr_expect_not_null(line_iterator);
  cr_expect_not_null(line_iterator->next);

  line_list_clear(&line_linked_list, free_chars);
  line_list__node* line_iterator2 = line_linked_list.head;

  // Cannot use line as it is a use after free
  // Undefined behavior, causes crash.
  // cr_expect_not_null(line_iterator2->next);
}

Test(link_list_init, for_each_test) {
  char* string1 = "This string is pushed first";
  char* string2 = "This string is pushed second";
  char* string3 = "This string is pushed third";

  line_list line_linked_list;
  line_list_init(&line_linked_list);

  line_list_push_front(&line_linked_list, string1);
  line_list_push_tail(&line_linked_list, string2);
  line_list_push_tail(&line_linked_list, string3);

  cr_expect_eq(count, 0);
  line_list_foreach(&line_linked_list, count_strings, NULL);
  cr_expect_eq(count, 3);

  line_list_clear(&line_linked_list, do_nothing);
//
//  count = 0;
//  line_list_foreach(&line_linked_list, count_strings, NULL);
//  cr_expect_eq(count, 0);
}

Test(line_list_api, increment_null_args) {
  line_list list;
  line_list_init(&list);

  line_list__node* it = NULL;

  // l is NULL
  cr_expect_eq(line_list_increment(&it, NULL), -1);

  // iterator pointer is NULL
  cr_expect_eq(line_list_increment(NULL, &list), -1);
}

Test(line_list_api, increment_empty_list) {
  line_list list;
  line_list_init(&list);

  line_list__node* it = NULL;

  // empty list, starting from it == NULL → should return -1 or END_OF_LIST
  cr_expect_eq(line_list_increment(&it, &list),
               -1);  // whatever your convention is
}

Test(line_list_api, increment_from_null_uses_head) {
  line_list list;
  line_list_init(&list);

  line_list_push_tail(&list, "a");
  line_list_push_tail(&list, "b");

  line_list__node* it = NULL;
  cr_expect_eq(line_list_increment(&it, &list), 0);
  cr_expect_str_eq(line_list_get_value(it), "a");
}
