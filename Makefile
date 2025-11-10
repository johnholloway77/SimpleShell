UNAME_S != uname -s
BINARY = simpleShell
SRC_DIR = ./src
CFLAGS ?= -Wall -Wextra
DEBUG ?= -g -O0

#Compiler
.if "$(UNAME_S)" == "FreeBSD"
CC = clang
.else
CC = gcc
.endif

# Source files
SOURCES != ls -1 $(SRC_DIR)/*.c 2>/dev/null || true
SOURCES += main.c

# Tool variables:
STATIC_ANALYSIS ?= cppcheck
STYLE_CHECK = cpplint
DESIGN_DIR = docs/design #not yet setup
DOXY_DIR = docs/code
COVERAGE_DIR = coverage
COVERAGE_RESULTS = results.coverage

.if "$(UNAME_S)" == "FreeBSD"
  COV = llvm
.else
  GCOV = gcov
  LCOV = lcov
.endif

# Build process
OBJECTS = $(SOURCES:.c=.o)

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

#rule to compile source files into object files
%.o:  %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:  clean
clean:
	${rm} -rf $(BINARY) $(OBJECTS)

#clean only the object files
.PHONY: clean-obj
clean-obj:
	${rm} -rf $(OBJECTS)

.PHONY: static
static:
	${STATIC_ANALYSIS} --verbose --enable=all --error-exitcode=1 ./main.c ${SRC_DIR}/*.c

.PHONY: style
style:
	${STYLE_CHECK} ./main.c ${SRC_DIR}/*.c