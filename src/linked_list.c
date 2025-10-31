//
// Created by jholloway on 10/31/25.
//

#include <stdlib.h>

#include "linked_list.h"

single_string_node* create_single_node(char* string) {
  single_string_node* new_node =
      (single_string_node*)malloc(sizeof(single_string_node));
  new_node->string = string;
  new_node->next = NULL;

  return new_node;
}

double_node* create_double_node(char** path_args) {
  double_node* new_node = (double_node*)malloc(sizeof(double_node));
  new_node->path_args = path_args;
  new_node->next = NULL;
  new_node->previous = NULL;

  return new_node;
}

void init_single_string_list(single_linked_list* sll) {
  sll->head = sll->tail = NULL;
}

void init_double_linked_list(double_linked_list* dll) {
  dll->front = dll->tail = NULL;
}

int isEmpty(double_linked_list* dll) {
  return dll->front == NULL;
}

void insert_end_dll(double_linked_list* dll, char** path_args) {
  double_node* newNode = create_double_node(path_args);
  if (isEmpty(dll)) {
    dll->front = dll->tail = newNode;
  } else {
    dll->tail->next = newNode;
    dll->tail->previous = dll->tail;
    dll->tail = newNode;
  }
}

char** remove_front_dll(double_linked_list* dll) {
  if (isEmpty(dll)) {
    return NULL;
  } else {
    double_node* temp = dll->front;
    char** path_args = temp->path_args;

    dll->front = dll->front->next;
    if (dll->front == NULL) {
      dll->tail = NULL;
    } else {
      dll->front->previous = NULL;
    }
    free(temp);
    return path_args;
  }
}

double_node* get_next_node(double_node* current) {
  return current->next;
}

double_node* get_previous_node(double_node* current) {
  return current->previous;
}