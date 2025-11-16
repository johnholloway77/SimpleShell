////
//// Created by jholloway on 11/11/25.
////
//

#include <criterion/criterion.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include "../src/sh/sh_src.h"

static char* read_all_fd(int fd) {
  size_t cap = 8192, len = 0;
  char* buf = malloc(cap);
  for (;;) {
    if (len + 4096 > cap) {
      cap <<= 1;
      buf = realloc(buf, cap);
    }
    ssize_t n = read(fd, buf + len, cap - len);
    if (n <= 0) break;  // EOF or error
    len += n;
  }
  buf[len] = '\0';
  return buf;
}

Test(sh_loop, keep_test) {
  int in_pipe[2], out_pipe[2];
  cr_assert_eq(pipe(in_pipe), 0);
  cr_assert_eq(pipe(out_pipe), 0);

  int saved_stdin = dup(STDIN_FILENO);
  int saved_stdout = dup(STDOUT_FILENO);
  int saved_stderr = dup(STDERR_FILENO);  // optional but recommended

  // Redirect: stdin <- in_pipe[0]; stdout/stderr -> out_pipe[1]
  cr_assert(dup2(in_pipe[0], STDIN_FILENO) >= 0);
  cr_assert(dup2(out_pipe[1], STDOUT_FILENO) >= 0);
  cr_assert(dup2(out_pipe[1], STDERR_FILENO) >= 0);

  close(in_pipe[0]);   // now stdin
  close(out_pipe[1]);  // now stdout/stderr

  // Feed script and close stdin write-end so sh_loop() sees EOF
  const char* script = "echo This is a test echo!\nexit\n";
  cr_assert_eq(write(in_pipe[1], script, strlen(script)),
               (ssize_t)strlen(script));
  close(in_pipe[1]);  // <-- important

  // Run shell
  char** mock_envp = malloc(64 * sizeof(char *));
  memset(mock_envp, 0, 64 * sizeof(char *));

  mock_envp[0] = (char *)malloc(MAXPATHLEN);
  char* mock_env = "SHELL=/bin/bsh";
  strlcpy(mock_envp[0], mock_env, MAXPATHLEN);

  sh_loop(mock_envp);

  // Restore std fds so our process no longer holds the pipe write-end
  cr_assert(dup2(saved_stdin, STDIN_FILENO) >= 0);
  cr_assert(dup2(saved_stdout, STDOUT_FILENO) >= 0);
  cr_assert(dup2(saved_stderr, STDERR_FILENO) >= 0);
  close(saved_stdin);
  close(saved_stdout);
  close(saved_stderr);

  // Now it’s safe to read until EOF
  char* output = read_all_fd(out_pipe[0]);
  close(out_pipe[0]);

  char* expected = "This is a test echo!";

  cr_expect_not_null(output);
  cr_expect_str_eq(output, expected);
  free(output);
  free(mock_envp[0]);
}
