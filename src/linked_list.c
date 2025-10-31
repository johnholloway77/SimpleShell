//
// Created by jholloway on 10/31/25.
//

#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

Node* createNode(char** path_args) {
  Node* new_node = (Node*)malloc(sizeof(Node));
  new_node->path_args = path_args;
  new_node->next = NULL;
  new_node->previous = NULL;

  return new_node;
}

void init_double_linked_list(double_linked_list* dll) {
  dll->front = dll->rear = NULL;
}

int isEmpty(double_linked_list* dll) {
  return dll->front == NULL;
}

void insert_end_dll(double_linked_list* dll, char** path_args) {
  Node* newNode = createNode(path_args);
  if (isEmpty(dll)) {
    dll->front = dll->rear = newNode;
  } else {
    dll->rear->next = newNode;
    dll->rear->previous = dll->rear;
    dll->rear = newNode;
  }
}

char** remove_front_dll(double_linked_list* dll) {
  if (isEmpty(dll)) {
    return NULL;
  } else {
    Node* temp = dll->front;
    char** path_args = temp->path_args;

    dll->front = dll->front->next;
    if (dll->front == NULL) {
      dll->rear = NULL;
    } else {
      dll->front->previous = NULL;
    }
    free(temp);
    return path_args;
  }
}

Node* get_next_node(Node* current) {
  return current->next;
}

Node* get_previous_node(Node* current) {
  return current->previous;
}