# Makefile for aesdsocket

# Default target architecture
ifeq ($(CROSS_COMPILE),)
	CC = gcc
else
	CC = $(CROSS_COMPILE)gcc
endif

# Compiler flags
CFLAGS = -Wall -Wextra -Werror -std=c99 -D_GNU_SOURCE
LDFLAGS = 

# Target executable
TARGET = aesdsocket

# Source files
SOURCES = aesdsocket.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Default target
.PHONY: all default clean

all: $(TARGET)

default: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

# Install target (optional)
install: $(TARGET)
	install -m 755 $(TARGET) /usr/bin/

.PHONY: install