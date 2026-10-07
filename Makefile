CC      = gcc
CFLAGS  = -O2 -Wall -Wextra
TARGET   = kbdusb
SRC      = kbdusb.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
