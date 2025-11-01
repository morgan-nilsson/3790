CC=gcc
CC_FLAGS=-Wall -Wextra -Iinclude

SRC_DIR=src
BUILD_DIR=build

SRC_FILES=$(SRC_DIR)/scheduler.c $(SRC_DIR)/main.c $(SRC_DIR)/logger.c
TARGET=$(BUILD_DIR)/main.out
TOY_PROG=$(BUILD_DIR)/prog.out

all: $(BUILD_DIR) $(TARGET) $(TOY_PROG)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRC_FILES)
	$(CC) $(CC_FLAGS) -o $@ $^

$(TOY_PROG): $(SRC_DIR)/toy.c
	$(CC) $(CC_FLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

make run: all
	./$(TARGET)