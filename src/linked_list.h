//
// Created by jholloway on 10/31/25.
//

#ifndef SIMPLESHELL_LINKED_LIST_H
#define SIMPLESHELL_LINKED_LIST_H

typedef struct single_string_node {
  struct single_string_node* next;
  char* string;
} single_string_node;

typedef struct single_linked_list {
  single_string_node* head;
  single_string_node* tail;
} single_linked_list;

typedef struct double_node {
  struct double_node* next;
  struct double_node* previous;
  char** path_args;
} double_node;

typedef struct double_linked_list {
  double_node* front;
  double_node* tail;
} double_linked_list;

single_string_node* create_single_node(char* string);

double_node* create_double_node(char** path_args);

void init_single_string_list(single_linked_list* sll);

void init_double_linked_list(double_linked_list* dll);

int isEmpty(double_linked_list* dll);

void insert_end_dll(double_linked_list* dll, char** path_args);

char** remove_front_dll(double_linked_list* dll);

double_node* get_next_node(double_node* current);

double_node* get_previous_node(double_node* current);

#endif  // SIMPLESHELL_LINKED_LIST_H
