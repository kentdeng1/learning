CC      := gcc
CFLAGS  := -Wall -Wextra -O0 -std=c11 -g -D_WIN32_WINNT=0x0601 -D__USE_MINGW_ANSI_STDIO=1
TARGET  := test_offsetof
SRC     := main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all run clean
