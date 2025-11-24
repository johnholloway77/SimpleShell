// single_linked_list.h
#ifndef SIMPLESHELL_MACRO_LINKED_LIST_H
#define SIMPLESHELL_MACRO_LINKED_LIST_H

#include <stdlib.h>

#define END_OF_LIST 2

#define DEFINE_SLIST(NAME, T)                                                 \
  typedef struct NAME##__node {                                               \
    struct NAME##__node* next;                                                \
    struct NAME##__node* previous;                                            \
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
  static inline int NAME##_increment(NAME##__node** iterator, NAME* l) {      \
    if (!l || !iterator) {                                                    \
      return -1;                                                              \
    }                                                                         \
                                                                              \
    if (!*iterator) {                                                         \
      if (!l->head) {                                                         \
        return -1;                                                            \
      }                                                                       \
      *iterator = l->head;                                                    \
      return 0;                                                               \
    }                                                                         \
                                                                              \
    if (iterator && *iterator) {                                              \
      *iterator = (*iterator)->next;                                          \
      if (!*iterator) {                                                       \
        return END_OF_LIST;                                                   \
      }                                                                       \
      return 0;                                                               \
    }                                                                         \
    return -1;                                                                \
  }                                                                           \
                                                                              \
  static inline T NAME##_get_value(NAME##__node* iterator) {                  \
    if (!iterator) {                                                          \
      return NULL;                                                            \
    }                                                                         \
    return iterator->value;                                                   \
  }                                                                           \
                                                                              \
  static inline int NAME##_decrement(NAME##__node** iterator, NAME* l) {      \
    if (!l || !iterator) {                                                    \
      return -1;                                                              \
    }                                                                         \
                                                                              \
    if (!*iterator) {                                                         \
      if (!l->tail) {                                                         \
        return -1;                                                            \
      }                                                                       \
      *iterator = l->tail;                                                    \
      return 0;                                                               \
    }                                                                         \
                                                                              \
    if (iterator && *iterator) {                                              \
      *iterator = (*iterator)->previous;                                      \
      if (!*iterator) {                                                       \
        return END_OF_LIST;                                                   \
      }                                                                       \
      return 0;                                                               \
    }                                                                         \
    return -1;                                                                \
  }                                                                           \
                                                                              \
  static inline int NAME##_push_front(NAME* l, T v) {                         \
    NAME##__node* temp = l->head;                                             \
    NAME##__node* n = (NAME##__node*)malloc(sizeof *n);                       \
    if (!n) return -1;                                                        \
                                                                              \
    n->value = v;                                                             \
    n->next = l->head;                                                        \
    n->previous = NULL;                                                       \
                                                                              \
    if (l->head) {                                                            \
      l->head->previous = n;                                                  \
    } else {                                                                  \
      l->head = n;                                                            \
    }                                                                         \
    if (l->tail == NULL) l->tail = n; /* empty list before insert */          \
    return 0;                                                                 \
  }                                                                           \
                                                                              \
  static inline int NAME##_push_tail(NAME* l, T v) {                          \
    NAME##__node* n = (NAME##__node*)malloc(sizeof *n);                       \
    if (!n) return -1;                                                        \
    n->value = v;                                                             \
    n->next = NULL;                                                           \
    n->previous = l->tail;                                                    \
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
      NAME##__node* n = l->head;                                              \
      l->head = n->next;                                                      \
      if (deleter) {                                                          \
        deleter(&n->value);                                                   \
      }                                                                       \
      free(n);                                                                \
    }                                                                         \
    l->tail = NULL;                                                           \
  }

DEFINE_SLIST(line_list, char*)

#endif /* SIMPLESHELL_MACRO_LINKED_LIST_H */
