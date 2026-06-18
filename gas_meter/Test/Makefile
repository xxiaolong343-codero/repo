# Unit test Makefile — runs on host (gcc)
CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c99 -DPLATFORM_HOST=1 \
          -I../Application \
          -I../HAL/Include \
          -I../Bsp/BoardA_RL78 \
          -Iunity

UNITY_DIR = unity
CMOCK_DIR = cmock

TEST_SRC = test_task_manager.c test_io_manager.c \
           $(UNITY_DIR)/unity.c \
           ../Application/TaskManager.c \
           ../HAL/IO/IOManager.c

all: test

test: $(TEST_SRC)
	$(CC) $(CFLAGS) -o $@ $^
	./test

clean:
	rm -f test *.o

.PHONY: all test clean
