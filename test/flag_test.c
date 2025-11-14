//
// Created by jholloway on 11/13/25.
//

#include <criterion/criterion.h>
#include <criterion/internal/test.h>

#include "../src/flags/flags.h"
#include "../src/flags/set_flags.h"

void reset_flags(void);

Test(set_flag, check_macro_test) {
#ifdef __TESTING
  cr_assert_not_null(1);
#endif
}

Test(set_flag, reset_app_flags_test) {
  cr_assert_null(app_flags);

  char* flag_str = "-xc";

  set_flags(flag_str);
  cr_assert_not_null(app_flags);

  reset_flags();

  cr_assert_null(app_flags);
}

Test(set_flag, check_global_test) {
  cr_assert_null(app_flags);
}

Test(set_flag, set_app_flags_test) {
  cr_assert_null(app_flags);

  char* x_str = "-x";
  char* c_str = "-c";
  char* xc_str = "-xc";
  char* other = "-other";

  set_flags(x_str);
  cr_assert_not_null(app_flags);
  cr_assert_eq(app_flags & X_FLAG, X_FLAG);
  reset_flags();

  set_flags(c_str);
  cr_assert_eq(app_flags & C_FLAG, C_FLAG);
  reset_flags();

  set_flags(xc_str);
  cr_assert_eq(app_flags & X_FLAG, X_FLAG);
  cr_assert_eq(app_flags & C_FLAG, C_FLAG);
  reset_flags();

  set_flags(other);
  cr_assert_null(app_flags);
  //  cr_assert_neq(app_flags & X_FLAG, X_FLAG);
  //  cr_assert_neq(app_flags & C_FLAG, C_FLAG);
  reset_flags();
}