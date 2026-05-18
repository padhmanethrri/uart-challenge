CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = uart

all: $(TARGET)

$(TARGET): uart.c
	$(CC) $(CFLAGS) -o $(TARGET) uart.c

clean:
	rm -f $(TARGET)
