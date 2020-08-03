CC := gcc
CXX := g++
CXXFLAGS := -std=c++11
all : c cpp
.PHONY : all

c:
	cmake -Wdev -S src/c -B src/c/cmake-build-debug
	@make -C src/c/cmake-build-debug CC=$(CC)

cpp:
	@make -C src/c++ CXX=$(CXX) CXXFLAGS=$(CXXFLAGS)

clean:
	@make -C src/c/cmake-build-debug clean
	@make -C src/c++ clean
run:
	@make -C src/c/cmake-build-debug run
	@make -C src/c++ run
