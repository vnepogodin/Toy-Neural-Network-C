CXX := g++
CFLAGS := --std=gnu++2a -ffast-math
objects := */init
all: Matrix.o nn.o $(objects)
.PHONY := all

Matrix.o:
	$(CXX) $(CFLAGS) -c -o Matrix.o Matrix.cpp

nn.o:
	$(CXX) $(CFLAGS) -c -o nn.o nn.cpp

$(objects): %: %.cpp
	$(CXX) $(CFLAGS) $< Matrix.o nn.o -o $@

clean:
	rm *.o $(objects)
run:
	./*/init
