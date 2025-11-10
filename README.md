### Please note: this assignment is not yet complete!

# SimpleShell

This program is based in part on the final assignment for the class [CS631 Advanced Programming in the UNIX Environment](https://stevens.netmeister.org/631/) by Jan Schaumann.

## Assignment Goals

The goal is to create a simple POSIX-compliant shell. Details of the assignment can be found [here](https://stevens.netmeister.org/631/f25-sish.html). The focus of this is to combine the concepts taught in the aforementioned class and integrate them into and application which properly handles piping, job control, directory navigation, forking, and more in accordance with the practices of proper UNIX programming.

## Building the Program

To build the program, follow these steps:

1. Navigate to the main directory of the program.
2. Run the following commands:

```sh
make
```

To remove the only object files after compiling the binary run the following command

```sh
make clean-obj
```

To remove all files created during the make process including the binary run the following

```sh
make clean
```

## Running the Program

```sh
./simpleShell [option] [command] ...
```

Replace [options] with any applicable flags and [command] with the commands you wish to run.

Flags include:

- -c command Execute the given command.
- -x Enable Tracing: Write each command to standard error, preceded by "+"

## Build Documentation

To build the corresponding doxygen html document run the command
```shell
make docs-html
```

The documents can be deleted with
```shell
make clean-docs
```
## Memory Management

This program has been carefully developed to handle memory management correctly, ensuring no memory leaks. Valgrind was used extensively to check for and fix any memory issues.
