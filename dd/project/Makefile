CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11 -Iinclude
DEBUG_FLAGS := -DDEBUG -g
SRC := src/main.c src/stack.c src/queue.c src/list.c src/utils.c
OBJ := $(SRC:.c=.o)
BIN := ds_cli

.PHONY: all build clean run test debug

all: build

build: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(BIN)

run: build
	./$(BIN)

test: build
	bash tests/test.sh

clean:
	rm -f $(OBJ) $(BIN) app.log tests/*.tmp
