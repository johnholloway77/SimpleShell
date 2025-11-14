//
// Created by jholloway on 11/13/25.
//

#ifndef SIMPLESHELL_SET_FLAGS_H
#define SIMPLESHELL_SET_FLAGS_H

#include <stdint.h>

#ifndef __TESTING

extern const uint32_t app_flags;

#else
extern uint32_t app_flags;
#endif

int set_flags(const char* flag_str);

#ifdef __TESTING
#endif

#endif  // SIMPLESHELL_SET_FLAGS_H
