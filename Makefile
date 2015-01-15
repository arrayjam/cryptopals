CC = clang
CFLAGS = -O0 -g
SOURCE_FILE = challenges.cpp

BUILD_DIR = build
BUILD_FILE = challenges
BUILD_PATH = $(BUILD_DIR)/$(BUILD_FILE)

all: build run

clean:
	rm -rf $(BUILD_DIR)

run:
	./$(BUILD_PATH)

build:
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_PATH) $(SOURCE_FILE)

valgrind: clean build
	valgrind --leak-check=full --track-origins=yes ./$(BUILD_PATH)

.PHONY: all clean run build valgrind
