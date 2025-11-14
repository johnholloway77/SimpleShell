//
// Created by jholloway on 11/13/25.
//

#include <criterion/criterion.h>
#include <criterion/internal/test.h>

#include "../src/flags.h"
#include "../src/set_flags.h"

Test(set_flag, check_global_test) {
  cr_assert_null(app_flags);
}

Test(set_flag, set_app_flags_test) {
  cr_assert_null(app_flags);

  char* flag_str = "-xc";

  set_flags(flag_str);
  cr_assert_not_null(app_flags);

  cr_assert_eq(app_flags & X_FLAG, X_FLAG);
  cr_assert_eq(app_flags & C_FLAG, C_FLAG);
}