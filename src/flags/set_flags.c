//
// Created by jholloway on 11/13/25.
//

#include <stdint.h>
#if defined(__linux__)
#include <bsd/string.h>
#else
#include <string.h>
#endif

#include "flags.h"

uint32_t app_flags = 0;

void set_flags(const char* flag_str) {
  int length = strlen(flag_str);

  for (int i = 1; i < length; i++) {
    switch (flag_str[i]) {
      case 'c':
        app_flags |= C_FLAG;
        break;
      case 'x':
        app_flags |= X_FLAG;
        break;
      default:
        continue;
    }
  }
}

void reset_flags(void) {
  app_flags = 0;
}
