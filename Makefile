CC ?= cc
CFLAGS ?= -std=c11 -O2 -g
WARNINGS := -Wall -Wextra -Wpedantic -Werror
TARGET := bin/fanwatch
SOURCE := src/fanwatch.c

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SOURCE)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(WARNINGS) $(SOURCE) -o $(TARGET)

test: all
	./tests/smoke.sh

clean:
	rm -rf bin
