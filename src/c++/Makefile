CXX := g++
CXXFLAGS := -std=c++11 -ffast-math -msse4 -mavx2 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2
objects := tests/init tests/nn_copy tests/nn_predict tests/nn_train tests/json_serialize tests/json_deserialize
all: Matrix.o nn.o $(objects)
.PHONY := all

Matrix.o:
	$(CXX) $(CXXFLAGS) -ffast-math -c -o Matrix.o Matrix.cpp

nn.o:
	$(CXX) $(CXXFLAGS) -ffast-math -c -o nn.o nn.cpp

$(objects): %: %.cpp
	$(CXX) $(CXXFLAGS) $< nn.o Matrix.o -o $@

clean:
	rm *.o $(objects)
run:
	./*/init
	./*/nn_copy
	./*/nn_predict
	./*/nn_train
	./*/json_serialize
	./*/json_deserialize
