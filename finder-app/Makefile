CC ?= gcc
CROSS_COMPILE ?=
CFLAGS = -Wall -Wextra -std=c99

TARGET = writer
SOURCES = writer.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: all clean