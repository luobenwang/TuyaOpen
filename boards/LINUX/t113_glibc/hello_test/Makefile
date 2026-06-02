# Minimal hello-world build for the bundled T113 cross toolchain.
# Run from this directory: make

WRAPPER_DIR := ../toolchain_wrapper
CC          := $(WRAPPER_DIR)/t113-gcc
TARGET      := hello_world
SRC         := main.c

CFLAGS      := -O2 -Wall -Wextra -std=c99

.PHONY: all clean info

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

info:
	@file $(TARGET) 2>/dev/null || true
	@$(WRAPPER_DIR)/t113-gcc -dumpmachine 2>/dev/null || true

clean:
	rm -f $(TARGET)
