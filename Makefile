# ---------------------------------------
# CONFIGURATION BEGIN
# ---------------------------------------

# entry point for the program and target name
C_SRCS = src/main.c src/parsers/csv_parser.c src/parsers/numeric_parser.c util/helper_functions.c
CPP_SRCS = src/simulation.cpp

# Test source files
TEST_CPP_SRCS = test/testMainCache.cpp test/cache_layer_unit_tests.cpp

# Path to your systemc installation
SCPATH = $(SYSTEMC_HOME)

CFLAGS := -I$(SCPATH)/include -L$(SCPATH)/lib

# Binary folder
BIN_DIR := bin

# Object files
C_OBJS = $(patsubst src/%.c, $(BIN_DIR)/%.o, $(filter src/%.c,$(C_SRCS))) \
         $(patsubst util/%.c, $(BIN_DIR)/%.o, $(filter util/%.c,$(C_SRCS)))
CPP_OBJS = $(patsubst src/%.cpp, $(BIN_DIR)/%.o, $(CPP_SRCS))

TEST_CPP_OBJS = $(patsubst test/%.cpp, $(BIN_DIR)/%.o, $(TEST_CPP_SRCS))

# Ensure bin directory exists before compiling
$(BIN_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: util/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: test/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@


# assignment task file
HEADERS := helper_functions.h simulation.hpp csv_parser.h structs.h numeric_parser.h cache.hpp multiplexer.hpp cache_layer.hpp main_memory.hpp

# target name
TARGET := project

# cache tests target
CACHE_TEST_TARGET := cache_test

# Additional flags for the compiler
CXXFLAGS := -std=c++14  -I$(SCPATH)/include -L$(SCPATH)/lib -lsystemc -lm


# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# Determine if clang or gcc is available
CXX := $(shell command -v g++ || command -v clang++)
ifeq ($(strip $(CXX)),)
    $(error Neither clang++ nor g++ is available. Exiting.)
endif

CC := $(shell command -v gcc || command -v clang)
ifeq ($(strip $(CC)),)
    $(error Neither clang nor g is available. Exiting.)
endif

# Add rpath except for MacOS
UNAME_S := $(shell uname -s)

ifneq ($(UNAME_S), Darwin)
    CXXFLAGS += -Wl,-rpath=$(SCPATH)/lib
endif


# Default to release build for both app and library
all: debug

# Debug build
debug: CFLAGS += -g
debug: CXXFLAGS += -g
debug: $(TARGET) 

# Release build
release: CXXFLAGS += -O2
release: $(TARGET) 

# Rule to link object files to executable
$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $(CXXFLAGS) $(C_OBJS) $(CPP_OBJS) $(LDFLAGS) -o $(TARGET)

# Rule to link tests objects to executable
$(CACHE_TEST_TARGET): $(CPP_OBJS) $(TEST_CPP_OBJS)
	$(CXX) $(CXXFLAGS) $(CPP_OBJS) $(TEST_CPP_OBJS) $(LDFLAGS) -o $(CACHE_TEST_TARGET)


COVERAGE_FLAGS = -fprofile-arcs -ftest-coverage

coverage: CFLAGS += $(COVERAGE_FLAGS)
coverage: CXXFLAGS += $(COVERAGE_FLAGS)
coverage: LDFLAGS += $(COVERAGE_FLAGS) -lgcov
coverage: $(TARGET)

coverage-report:
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage-report

# clean up
clean:
	rm -f $(TARGET) $(CACHE_TEST_TARGET)
	rm -rf $(BIN_DIR)
	rm -f src/*.gcda src/*.gcno coverage.info
	rm -rf coverage-report


run: $(TARGET)
	./project --cycles 1000 requests.csv

run-debug: $(TARGET)
	./project -d --cycles 1000 debug.csv

run-cpp-tests: $(CACHE_TEST_TARGET)
	./bin/cache_test

run-python-tests:
	python3 test/cache_tests.py

run-tests: run-cpp-tests run-python-tests

.PHONY: all debug release clean coverage coverage-report run run-debug run-cpp-tests run-python-tests run-tests