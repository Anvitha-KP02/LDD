CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Iinclude
RELEASE_FLAGS := -O2 -DNDEBUG
DEBUG_FLAGS := -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer

SRC := src/main.c src/cli.c src/ds.c src/storage.c src/logger.c
OBJ := $(SRC:.c=.o)
TARGET := bin/task_manager

TEST_SRC := tests/test_ds.c src/ds.c src/storage.c
TEST_TARGET := bin/test_ds

.PHONY: all build debug test run clean dirs

all: build

dirs:
	mkdir -p bin logs data

build: dirs
	$(CC) $(CFLAGS) $(RELEASE_FLAGS) $(SRC) -o $(TARGET)

debug: dirs
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SRC) -o $(TARGET)

test: dirs
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(TEST_SRC) -o $(TEST_TARGET)
	./$(TEST_TARGET)

run: build
	./$(TARGET)

clean:
	rm -rf bin/*.o bin/task_manager bin/test_ds data/tasks.db logs/task_manager.log
