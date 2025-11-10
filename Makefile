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
.if "${UNAME_S}" == "FreeBSD"

LLVM_BIN        ?= /usr/local/llvm19/bin
CLANG           ?= ${LLVM_BIN}/clang
LLVMPROFDATA    ?= ${LLVM_BIN}/llvm-profdata
LLVMCOV         ?= ${LLVM_BIN}/llvm-cov

# Use the pinned clang as your C compiler
CC = ${CLANG}

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
	${CC} ${CPPFLAGS} ${CFLAGS} ${DEBUG} -c -o $@ $<

${BINARY}: ${OBJECTS}
	${CC} ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

${TEST_BINARY}: ${TEST_OBJECTS}
	${CC} ${DEBUG} ${LDFLAGS} -o $@ ${TEST_OBJECTS} ${TEST_LIBS}

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
	    printf "Error: Missing dependency\n"; \
	    exit 1; \
	  else \
	    printf "All dependencies present\n"; \
	  fi \


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


# To perform the code coverage checks
.if "${UNAME_S}" == "FreeBSD"



# Coverage flags (clang/llvm)
COV_CFLAGS  = -fprofile-instr-generate -fcoverage-mapping
COV_LDFLAGS = -fprofile-instr-generate
TEST_LIBS  += -lcriterion -lpthread

.PHONY: coverage
coverage: clean-exec clean-cov
	# Build the test runner *with coverage instrumentation* and *without main.c*.
	${CC} ${CPPFLAGS} ${CFLAGS} ${DEBUG} ${COV_CFLAGS} \
	      ${INCLUDE} ${LDFLAGS} \
	      -o ${TEST_BINARY} ${TEST_SOURCES} ${TEST_LIBS} ${COV_LDFLAGS}

	# Run to produce .profraw
	LLVM_PROFILE_FILE=coverage.profraw ./${TEST_BINARY}

	# Merge & render HTML
	${LLVMPROFDATA} merge -sparse coverage.profraw -o coverage.profdata
	${LLVMCOV} show ./${TEST_BINARY} \
	  -instr-profile=coverage.profdata \
	  -format=html -output-dir=${COVERAGE_DIR} \
	  -ignore-filename-regex="/usr/local/include/.*"

	${MAKE} clean-temp

.PHONY: clean-cov
clean-cov:
	rm -rf *.profdata *profraw ${COVERAGE_RESULTS} ${COVERAGE_DIR}


.PHONY: clean-temp
clean-temp:
	rm -rf *.profdata *profraw


.else
.PHONY: coverage
coverage: clean-exec clean-cov
# GNU toolchain path (fallback)
LCOV    ?= lcov
GCOV    ?= gcov

.PHONY: coverage
coverage: clean-exec clean-cov
	${CC} ${CPPFLAGS} ${CFLAGS} ${INCLUDE} -L/usr/local/lib \
		-o ${TEST_BINARY} ${TEST_DIR}/*.c ${SRC_DIR}/*.c ${LIBS}
	./${TEST_BINARY}
	${LCOV} --capture --gcov-tool ${GCOV} --directory . \
		--output-file ${COVERAGE_RESULTS} --rc lcov_branch_coverage=1
	${LCOV} --extract ${COVERAGE_RESULTS} "*/${SRC_DIR}/*" "*/${TEST_DIR}/*" \
		-o ${COVERAGE_RESULTS}
	genhtml ${COVERAGE_RESULTS} --output-directory ${COVERAGE_DIR}

.PHONY: clean-cov
clean-cov:
	rm -rf ${COVERAGE_RESULTS} ${COVERAGE_DIR}

.PHONY: clean-cov
clean-cov:
	rm -rf *.gcov *.gcda *.gcno ${COVERAGE_RESULTS} ${COVERAGE_DIR}

.PHONY: clean-temp
clean-temp:
	rm -rf *~ \#* .\#* \
	${SRC_DIR}/*~ ${SRC_DIR}/\#* ${SRC_DIR}/.\#* \
	${GTEST_INCLUDE_DIR}/*~ ${GTEST_INCLUDE_DIR}/\#* ${GTEST_INCLUDE_DIR}/.\#* \
	${SRC_INCLUDE_DIR}/*~ ${SRC_INCLUDE_DIR}/\#* ${SRC_INCLUDE_DIR}/.\#* \
	${PROJECT_SRC_DIR}/*~ ${PROJECT_SRC_DIR}/\#* ${PROJECT_SRC_DIR}/.\#* \
	${DESIGN_DIR}/*~ ${DESIGN_DIR}/\#* ${DESIGN_DIR}/.\#* \
	*.gcov *.gcda *.gcno

.endif



################################################################################
# Clean-up targets
################################################################################
.PHONY:  clean
clean:
	rm -rf $(BINARY) $(OBJECTS) $(libs)

# Clean only the executable files
.PHONY: clean-exec
clean-exec:
	rm -f $(TEST_BINARY) $(BINARY)

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