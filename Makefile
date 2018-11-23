CXX := g++

# debug info: 1 for enabling debug
BUILD_DIR := ./build
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	BUILD_DIR := $(BUILD_DIR)/Debug
	CXXFLAGS += -g
else
	BUILD_DIR := $(BUILD_DIR)/Release
	CXXFLAGS += -O3
endif

CXXFLAGS := -std=c++11 -pthread
INCLUDE_DIRS := ./include
SRC_DIRS := ./src
TEST_DIRS := ./test

COMMON_FLAGS := -I$(INCLUDE_DIRS)


.PHONY: all clean

all: ringbuff

ringbuff: $(INCLUDE_DIRS)/ringbuff.hpp $(INCLUDE_DIRS)/safequeue.hpp $(SRC_DIRS)/ringbuff.cc
	mkdir -p $(BUILD_DIR)
	g++ $(COMMON_FLAGS) $(TEST_DIRS)/test_ringbuff.cc $(SRC_DIRS)/ringbuff.cc $(CXXFLAGS) -o $(BUILD_DIR)/ringbuff

clean:
	rm -rf BUILD_DIR/

