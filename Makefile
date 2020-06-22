CC := gcc
CXX := g++
CXXFLAGS := -std=c++11
all : c cpp
.PHONY : all

c:
	$(MAKE) -C */c CC=$(CC)

cpp:
	$(MAKE) -C */c++ CXX=$(CXX) CXXFLAGS=$(CXXFLAGS)

clean:
	$(MAKE) -C */c clean
	$(MAKE) -C */c++ clean
run:
	$(MAKE) -C */c run
	$(MAKE) -C */c++ run
