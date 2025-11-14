//
// Created by jholloway on 11/13/25.
//

#include <stdint.h>
#include <string.h>

#include "flags.h"

uint32_t app_flags;

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
