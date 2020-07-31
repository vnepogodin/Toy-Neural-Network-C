CC := gcc
CFLAGS := -ffast-math -Wall -Wextra -Wshadow -Wcast-align -Wunused -Wconversion -Wsign-conversion -Wdouble-promotion -Wformat=2
LDFLAGS := -lm
PATH := ../../include/c/third_party/json-c
objects := tests/init tests/nn_copy tests/nn_predict tests/nn_train tests/json_serialize tests/json_deserialize
all: $(PATH)/json_tokener.o $(PATH)/random_seed.o $(PATH)/linkhash.o $(PATH)/printbuf.o $(PATH)/json_object.o $(PATH)/arraylist.o matrix.o nn.o $(objects)

$(objects): %: %.c
	$(CC) $(CFLAGS) $< nn.o matrix.o $(PATH)/json_object.o $(PATH)/arraylist.o $(PATH)/printbuf.o $(PATH)/linkhash.o $(PATH)/random_seed.o $(PATH)/json_tokener.o $(LDFLAGS) -o $@

clean:
	/bin/rm *.o $(PATH)/json_tokener.o $(PATH)/random_seed.o $(PATH)/linkhash.o $(PATH)/printbuf.o $(PATH)/json_object.o $(PATH)/arraylist.o $(objects)
run:
	./*/init
	./*/nn_copy
	./*/nn_predict
	./*/nn_train
	./*/json_serialize
	./*/json_deserialize
