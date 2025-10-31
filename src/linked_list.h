//
// Created by jholloway on 10/31/25.
//

#ifndef SIMPLESHELL_LINKED_LIST_H
#define SIMPLESHELL_LINKED_LIST_H

typedef struct Node {
  struct Node* next;
  struct Node* previous;
  char** path_args;
} Node;

typedef struct double_linked_list {
  Node* front;
  Node* rear;
} double_linked_list;

Node* createNode(char** path_args);

void init_double_linked_list(double_linked_list* dll);

int isEmpty(double_linked_list* dll);

void insert_end_dll(double_linked_list* dll, char** path_args);

char** remove_front_dll(double_linked_list* dll);

Node* get_next_node(Node* current);

Node* get_previous_node(Node* current);

#endif  // SIMPLESHELL_LINKED_LIST_H
