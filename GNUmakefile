SHELL := /bin/sh
UNAME_S := $(shell uname -s)

BINARY       := simpleShell
TEST_BINARY  := test_$(BINARY)

SRC_DIR  := ./src
LIBS     :=
LDFLAGS += -L/usr/local/lib -Wl,-rpath,/usr/local/lib
CPPFLAGS += -I/usr/local/include

CFLAGS ?= -Wall -Wextra
CFLAGS += -D__BSD_VISIBLE=1
DEBUG  ?= -g -O0

###############################################################################
# Compiler selection
###############################################################################

ifeq ($(UNAME_S),FreeBSD)

LLVM_BIN     ?= /usr/local/llvm19/bin
CLANG        ?= $(LLVM_BIN)/clang
LLVMPROFDATA ?= $(LLVM_BIN)/llvm-profdata
LLVMCOV      ?= $(LLVM_BIN)/llvm-cov
CC           ?= $(CLANG)

COV := llvm

else  # non-FreeBSD (Linux, etc.)

CC   ?= gcc
GCOV ?= gcov
LCOV ?= lcov
LIBS += -lbsd

endif

###############################################################################
# Sources / Objects
###############################################################################

# All C files under src plus main.c
SOURCES := $(shell find $(SRC_DIR) -type f -name '*.c' 2>/dev/null || true)
SOURCES += main.c
OBJECTS := $(SOURCES:.c=.o)

# Tests: all test/*.c plus all sources except main.c
TEST_DIR      := test
TEST_SOURCES  := $(shell ls -1 $(TEST_DIR)/*.c 2>/dev/null || true)
TEST_SOURCES  += $(filter-out main.c,$(SOURCES))
TEST_OBJECTS  := $(TEST_SOURCES:.c=.o)
TEST_LIBS     := -lcriterion -lpthread
TEST_MACRO_DEFINE += -D__TESTING=1

# Tools
STATIC_ANALYSIS ?= cppcheck
STYLE_CHECK     ?= cpplint
DOXYFILE        := ?docs
DOXY_OUTPUT     := docs/code/html
COVERAGE_DIR    := coverage
COVERAGE_RESULTS := results.coverage

###############################################################################
# Rules
###############################################################################

.SUFFIXES: .c .o
.c.o:
        $(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG) -c -o $@ $<

$(BINARY): $(OBJECTS)
        $(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(TEST_BINARY): $(TEST_OBJECTS)
        $(CC) $(DEBUG) $(LDFLAGS) -o $@ $(TEST_OBJECTS) $(TEST_LIBS) $(TEST_MACRO_DEFINE)

###############################################################################
# Test targets
###############################################################################

.PHONY: test
test: $(TEST_BINARY)
        ./$(TEST_BINARY) --verbose=1

.PHONY: check-deps
check-deps:
        @set -e; missing=; \
        for t in "$(STYLE_CHECK)" "$(STATIC_ANALYSIS)" "doxygen"; do \
          if ! command -v $$t >/dev/null 2>&1; then \
            printf "Missing dependency: %s\n" "$$t"; missing=1; \
          else \
            printf "Found: %s " "$$t"; ($$t --version 2>/dev/null || $$t -v 2>/dev/null || echo OK) | head -n1; \
          fi; \
        done; \
        if [ -n "$$missing" ]; then \
          printf "Error: Missing dependency\n"; \
          exit 1; \
        else \
          printf "All dependencies present\n"; \
        fi

.PHONY: static
static: check-deps
        $(STATIC_ANALYSIS) --verbose --enable=all --error-exitcode=1 ./main.c $(SRC_DIR)/*.c

.PHONY: style
style: check-deps
        $(STYLE_CHECK) ./main.c $(SRC_DIR)/*.c

###############################################################################
# Documentation
###############################################################################

.PHONY: docs-html
docs-html: $(DOXYFILE) $(SRC_DIR) main.c check-deps
        doxygen docs/doxyfile

###############################################################################
# Coverage (FreeBSD vs Linux)
###############################################################################

ifeq ($(UNAME_S),FreeBSD)

# Coverage flags (clang/llvm)
COV_CFLAGS  := -fprofile-instr-generate -fcoverage-mapping -mllvm -enable-name-compression=false
COV_LDFLAGS := -fprofile-instr-generate
TEST_LIBS   += -lcriterion -lpthread

.PHONY: coverage
coverage: clean-exec clean-cov
        # Build the test runner *with coverage instrumentation* and *without main.c*.
        $(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG) $(COV_CFLAGS) \
              $(INCLUDE) $(LDFLAGS) \
              -o $(TEST_BINARY) $(TEST_SOURCES) $(TEST_LIBS) $(COV_LDFLAGS)

        # One .profraw per process (Criterion forks)
        LLVM_PROFILE_FILE="coverage-%p.profraw" ./$(TEST_BINARY)

        # Merge all per-process profiles
        $(LLVMPROFDATA) merge -sparse coverage-*.profraw -o coverage.profdata

        # Render HTML
        $(LLVMCOV) show ./$(TEST_BINARY) \
          -instr-profile=coverage.profdata \
          -format=html -output-dir=$(COVERAGE_DIR) \
          -show-branches=count \
          --show-expansions \
          -ignore-filename-regex="/usr/local/include/.*" \
          -ignore-filename-regex="$(TEST_DIR)/*.c"

        $(MAKE) clean-temp
        rm -f coverage-*.profraw

.PHONY: clean-cov
clean-cov:
        rm -rf *.profdata *profraw $(COVERAGE_RESULTS) $(COVERAGE_DIR)

.PHONY: clean-temp
clean-temp:
        rm -rf *.profdata *profraw

else  # non-FreeBSD (Linux, etc.)

# GNU toolchain path (fallback)
LCOV ?= lcov
GCOV ?= gcov

.PHONY: coverage
coverage: clean-exec clean-cov
        @echo "Coverage not yet implemented for GNU/Linux"
        @exit 1

.PHONY: clean-cov
clean-cov:
        rm -rf *.gcov *.gcda *.gcno $(COVERAGE_RESULTS) $(COVERAGE_DIR)

.PHONY: clean-temp
clean-temp:
        rm -rf *~ \#* .\#* \
        $(SRC_DIR)/*~ $(SRC_DIR)/\#* $(SRC_DIR)/.\#* \
        $(GTEST_INCLUDE_DIR)/*~ $(GTEST_INCLUDE_DIR)/\#* $(GTEST_INCLUDE_DIR)/.\#* \
        $(SRC_INCLUDE_DIR)/*~ $(SRC_INCLUDE_DIR)/\#* $(SRC_INCLUDE_DIR)/.\#* \
        $(PROJECT_SRC_DIR)/*~ $(PROJECT_SRC_DIR)/\#* $(PROJECT_SRC_DIR)/.\#* \
        $(DESIGN_DIR)/*~ $(DESIGN_DIR)/\#* $(DESIGN_DIR)/.\#* \
        *.gcov *.gcda *.gcno

endif

###############################################################################
# Clean targets
###############################################################################

.PHONY: clean
clean:
        rm -rf $(BINARY) $(OBJECTS) $(libs)

.PHONY: clean-exec
clean-exec:
        rm -f $(TEST_BINARY) $(BINARY)

.PHONY: clean-objs
clean-objs:
        rm -rf $(OBJECTS) $(TEST_OBJECTS)

.PHONY: clean-docs
clean-docs:
        rm -rf $(DOXY_OUTPUT)

.PHONY: clean-tests
clean-tests:
        rm -rf $(TEST_BINARY) $(TEST_OBJECTS)
