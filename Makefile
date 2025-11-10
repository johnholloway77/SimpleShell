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
STYLE_CHECK ?= cpplint
DOXYFILE = ?docs
DOXY_OUTPUT = docs/code/html
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
	rm -rf $(BINARY) $(OBJECTS)

#clean only the object files
.PHONY: clean-obj
clean-obj:
	rm -rf $(OBJECTS)

# clean only docs
.PHONY: clean-docs
clean-docs:
	rm -rf ${DOXY_OUTPUT}

.PHONY: static
static: check-deps
	${STATIC_ANALYSIS} --verbose --enable=all --error-exitcode=1 ./main.c ${SRC_DIR}/*.c

.PHONY: style
style: check-deps
	${STYLE_CHECK} ./main.c ${SRC_DIR}/*.c

.phony: docs-html
docs-html: ${DOXYFILE} ${SRC_DIR} main.c check-deps
	doxygen docs/doxyfile

check-deps:
	@set -e; missing=; \
	for t in "${STYLE_CHECK}" "${STATIC_ANALYSIS}" "doxygen"; do \
	  if ! command -v $$t >/dev/null 2>&1; then \
	    printf "Missing dependency: %s\n" "$$t"; missing=1; \
	  else \
	    printf "Found: %s " "$$t"; ($$t --version 2>/dev/null || $$t -v 2>/dev/null || echo OK) | head -n1; \
	  fi; \
	  done; \
	  if [ -n "$$missing" ]; then \
	    printf "Error: Missing dependency\n" \
	    exit 1; \
	  else \
	    printf "All dependencies present\m"; \
	  fi

