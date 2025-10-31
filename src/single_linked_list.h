// single_linked_list.h
#ifndef SIMPLESHELL_SINGLE_LINKED_LIST_H
#define SIMPLESHELL_SINGLE_LINKED_LIST_H

#include <stdlib.h>

#define DEFINE_SLIST(NAME, T)                                                 \
  typedef struct NAME##__node {                                               \
    struct NAME##__node* next;                                                \
    T value;                                                                  \
  } NAME##__node;                                                             \
                                                                              \
  typedef struct {                                                            \
    NAME##__node* head;                                                       \
    NAME##__node* tail;                                                       \
  } NAME;                                                                     \
                                                                              \
  static inline void NAME##_init(NAME* l) {                                   \
    l->head = NULL;                                                           \
    l->tail = NULL;                                                           \
  }                                                                           \
                                                                              \
  static inline int NAME##_push_front(NAME* l, T v) {                         \
    NAME##__node* n = (NAME##__node*)malloc(sizeof *n);                       \
    if (!n) return -1;                                                        \
    n->value = v;                                                             \
    n->next = l->head;                                                        \
    l->head = n;                                                              \
    if (l->tail == NULL) l->tail = n; /* empty list before insert */          \
    return 0;                                                                 \
  }                                                                           \
                                                                              \
  static inline int NAME##_push_tail(NAME* l, T v) {                          \
    NAME##__node* n = (NAME##__node*)malloc(sizeof *n);                       \
    if (!n) return -1;                                                        \
    n->value = v;                                                             \
    n->next = NULL;                                                           \
    if (l->tail) {                                                            \
      l->tail->next = n;                                                      \
    } else {                                                                  \
      l->head = n; /* empty list: head must point to n too */                 \
    }                                                                         \
    l->tail = n;                                                              \
    return 0;                                                                 \
  }                                                                           \
                                                                              \
  typedef void (*NAME##_visit_fn)(T * elem, void* arg);                       \
  static inline void NAME##_foreach(NAME* l, NAME##_visit_fn fn, void* arg) { \
    for (NAME##__node* p = l->head; p; p = p->next) fn(&p->value, arg);       \
  }                                                                           \
                                                                              \
  static inline void NAME##_clear(NAME* l, void (*deleter)(T*)) {             \
    while (l->head) {                                                         \
      NAME##__node* n =  l->head;                                            \
      l->head = n->next;                                                    \
      if (deleter) {                                                          \
        deleter(&n->value);                                                   \
      }                                                                       \
      free(n);                                                                \
    }                                                                         \
  }
#endif /* SIMPLESHELL_SINGLE_LINKED_LIST_H */
