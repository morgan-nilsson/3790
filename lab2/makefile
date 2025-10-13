CC=gcc
CC_FLAGS=-Wall -Wextra
LDFLAGS=

SRC_DRIR=src
BUILD_DIR=build
WORKER_EXE=$(BUILD_DIR)/worker
BOSS_EXE=$(BUILD_DIR)/boss

WORKER_SRC=$(SRC_DRIR)/worker.c
BOSS_SRC=$(SRC_DRIR)/boss.c

WORKER_OBJ=$(BUILD_DIR)/worker.o
BOSS_OBJ=$(BUILD_DIR)/boss.o

all: $(WORKER_EXE) $(BOSS_EXE)

worker: $(WORKER_EXE)
	./$(WORKER_EXE)

boss: $(BOSS_EXE)
	./$(BOSS_EXE)


$(WORKER_EXE): $(WORKER_OBJ)
	$(CC) $(LDFLAGS) -o $@ $<

$(BOSS_EXE): $(BOSS_OBJ)
	$(CC) $(LDFLAGS) -o $@ $<

$(WORKER_OBJ): $(WORKER_SRC) | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) -c -o $@ $<

$(BOSS_OBJ): $(BOSS_SRC) | $(BUILD_DIR)
	$(CC) $(CC_FLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)