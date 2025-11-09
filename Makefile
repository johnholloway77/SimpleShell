UNAME_S != uname -s
BINARY = simpleShell
SRC_DIR = ./src
CFLAGS ?= -Wall -Wextra
DEBUG ?= -g -O0

#Compiler
.if "$(UNAME_S)" == "FreeBSD"
    CC := clang
.else
    CC := gcc
.endif

# Source files
SOURCES != ls -1 $(SRC_DIR)/*.c 2>/dev/null || true
SOURCES += main.c

OBJECTS = $(SOURCES:.c=.o)

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

#rule to compile source files into object files
%.o:  %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:  clean
clean:
	rm -rf $(BINARY) $(OBJECTS)

#clean only the object files
.PHONY: clean-obj
clean-obj:
	rm -rf $(OBJECTS)