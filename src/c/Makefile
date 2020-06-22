CC := gcc
CFLAGS := -ffast-math -Wall -Wextra
LDFLAGS := -lm -L/usr/local/lib -ljson-c
objects := tests/init tests/nn_copy tests/nn_predict tests/nn_train tests/json_serialize
all: matrix.o nn.o $(objects)

$(objects): %: %.c
	$(CC) $(CFLAGS) $< nn.o matrix.o $(LDFLAGS) -o $@

clean:
	rm *.o $(objects)
run:
	./tests/init
	./tests/nn_copy
	./tests/nn_predict
	./tests/nn_train
	./tests/json_serialize
