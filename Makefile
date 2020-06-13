CC := gcc
CXX := g++
SUBDIRS := */c */c++
all : c cpp
.PHONY : all

c:
	$(MAKE) -C */c CC=$(CC)

cpp:
	$(MAKE) -C */c++ CXX=$(CXX)

clean:
	$(MAKE) -C */c clean
	$(MAKE) -C */c++ clean
run:
	$(MAKE) -C */c run
	$(MAKE) -C */c++ run
