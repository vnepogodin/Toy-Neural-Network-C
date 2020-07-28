CC := gcc
CXX := g++
CXXFLAGS := -std=c++11
all : c cpp
.PHONY : all

c:
	$(MAKE) -C src/c CC=$(CC)

cpp:
	$(MAKE) -C src/c++ CXX=$(CXX) CXXFLAGS=$(CXXFLAGS)

clean:
	$(MAKE) -C src/c clean
	$(MAKE) -C src/c++ clean
run:
	$(MAKE) -C src/c run
	$(MAKE) -C src/c++ run
