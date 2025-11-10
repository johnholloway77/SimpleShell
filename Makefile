UNAME_S != uname -s

BINARY = simpleShell
TEST_BINARY = test_${BINARY}

SRC_DIR = ./src
LIBS =
LDFLAGS += -L/usr/local/lib -Wl,-rpath,/usr/local/lib
CPPFLAGS += -I/usr/local/include

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
OBJECTS = $(SOURCES:.c=.o)

# For tests: compile all src/*.c but NOT main.c, plus all tests
TEST_DIR = test
TEST_SOURCES != ls -1 $(TEST_DIR)/*.c 2>/dev/null || true
TEST_SOURCES += ${SOURCES:Nmain.c}
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_LIBS = -lcriterion -lpthread $(LIBS)

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

#rule to compile source files into object files
.SUFFIXES: .c .o
.c.o:
	$(CC)  $(CPPFLAGS) -c -o $@ $<

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

$(TEST_BINARY): $(TEST_OBJECTS)
	$(CC) $(DEBUG) -o $@ $(TEST_OBJECTS) $(LDFLAGS) $(TEST_LIBS)

################################################################################
# test targets
################################################################################
.PHONY: test
test: ${TEST_BINARY}
	./${TEST_BINARY}  --verbose=1

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
	    return 0 \
	  fi


.PHONY: static
static: check-deps
	${STATIC_ANALYSIS} --verbose --enable=all --error-exitcode=1 ./main.c ${SRC_DIR}/*.c

.PHONY: style
style: check-deps
	${STYLE_CHECK} ./main.c ${SRC_DIR}/*.c

################################################################################
# Documentation targets
################################################################################

.phony: docs-html
docs-html: ${DOXYFILE} ${SRC_DIR} main.c check-deps
	doxygen docs/doxyfile


################################################################################
# Clean-up targets
################################################################################
.PHONY:  clean
clean:
	rm -rf $(BINARY) $(OBJECTS) $(libs)

#clean only the object files
.PHONY: clean-objs
clean-objs:
	rm -rf $(OBJECTS) $(TEST_OBJECTS)

# clean only docs
.PHONY: clean-docs
clean-docs:
	rm -rf ${DOXY_OUTPUT}

.PHONY: clean-tests
clean-tests:
	rm -rf ${TEST_BINARY} $(TEST_OBJECTS)