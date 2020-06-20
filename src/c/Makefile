CC := gcc
CFLAGS := -ffast-math -Wall -Wextra
LDFLAGS := -lm -L/usr/local/lib -ljson-c
objects := tests/init tests/nn_copy tests/nn_predict tests/nn_train
all: matrix.o nn.o $(objects)

$(objects): %: %.c
	$(CC) $(CFLAGS) $< nn.o matrix.o $(LDFLAGS) -o $@

clean:
	rm *.o $(objects)
run:
	./*/init
	./*/nn_copy
	./*/nn_predict
	./*/nn_train
