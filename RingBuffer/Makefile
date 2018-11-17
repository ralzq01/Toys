CXX=g++

.PHONY: all clean

all: test

test: RingBuff.cc RingBuff.hpp test.cc
	g++ test.cc RingBuff.cc -std=c++11 -pthread -o test

clean:
	rm -f test

