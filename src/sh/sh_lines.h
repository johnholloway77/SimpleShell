//
// Created by jholloway on 11/11/25.
//

#ifndef SIMPLESHELL_SH_LINES_H
#define SIMPLESHELL_SH_LINES_H

#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"
typedef struct path_struct {
  int count;
  char** path_options;
} Path_struct;

char* sh_read_line(void);
char** sh_split_line(char* line);
Path_struct* sh_parse_path_line(char* path_line);
char** update_args(char** args, char** envp);

#endif  // SIMPLESHELL_SH_LINES_H
