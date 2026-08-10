# Makefile —— 《C和指针》练习工程
# Windows(mingw):  mingw32-make
# macOS/Linux:     make
CC       = gcc
CFLAGS   ?= -std=c11 -Wall -Wextra -g -O0
SRC_DIR   = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
BINS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%,$(SRCS))

.PHONY: all run clean

all: $(BINS)

$(BUILD_DIR)/%: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# 逐个编译并运行所有练习
run: all
	@for bin in $(BINS); do echo ">> $$bin"; $$bin; echo; done

clean:
	rm -rf $(BUILD_DIR)
