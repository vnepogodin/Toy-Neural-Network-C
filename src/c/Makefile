CC := gcc
CFLAGS := -ffast-math -I/usr/local/include -I/usr/local/include/json-c
LDFLAGS := -lm -L/usr/local/lib -ljson-c
objects := */init */nn_predict */nn_train
all: matrix.o nn.o $(objects)

$(objects): %: %.c
	$(CC) $(CFLAGS) $< nn.o matrix.o $(LDFLAGS) -o $@

clean:
	rm *.o $(objects)
run:
	./*/init
	./*/nn_predict
	./*/nn_train
