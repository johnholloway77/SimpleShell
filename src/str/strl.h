//
// Created by jholloway on 11/12/25.
//
// This file exists solely to get rid of CLion/Clangd's warning about strlcpy
// and strlcat not being declared, despite it existing on FreeBSD's string.h
// header files.
//
// Implementation of the two functions are already included in the BSD standard
// library

#ifndef SIMPLESHELL_STRL_H
#define SIMPLESHELL_STRL_H

#include <unistd.h>

size_t strlcpy(char* dst, const char* origin, size_t length);
size_t strlcat(char* dst, const char* origin, size_t length);

#endif  // SIMPLESHELL_STRL_H
