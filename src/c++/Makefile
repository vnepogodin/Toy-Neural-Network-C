CXX := g++
CXXFLAGS := -std=gnu++2a
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
