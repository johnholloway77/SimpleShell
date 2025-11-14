//
// Created by jholloway on 11/13/25.
//

#ifndef SIMPLESHELL_FLAGS_H
#define SIMPLESHELL_FLAGS_H

/*
 * -c flag
 * Execute the given command.
 * eg: ./simpleShell ls -l
 * Will execute only the ls -l command and then exit
 */
#define C_FLAG 0x1

/*
 * -x flag
 * Will display the command executed when the shell is running
 * eg. ./simpleShell -x
 * JHsh$: ls -l
 * -ls
 *  [ls then displays its output]
 */
#define X_FLAG 0x2

#endif  // SIMPLESHELL_FLAGS_H
