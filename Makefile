CC=gcc
CFLAGS=-Wall -Werror -g -std=c99
TARGET=aesdsocket
SRCDIR=server
SRC=$(SRCDIR)/aesdsocket.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
	rm -f /var/tmp/aesdsocketdata