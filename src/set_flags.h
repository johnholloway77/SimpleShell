//
// Created by jholloway on 11/13/25.
//

#ifndef SIMPLESHELL_SET_FLAGS_H
#define SIMPLESHELL_SET_FLAGS_H

#include <stdint.h>

extern const uint32_t app_flags;

int set_flags(const char* flag_str);

#endif  // SIMPLESHELL_SET_FLAGS_H
