# 最小 Makefile 模板 —— 嵌入式练习专用
#
# 用法（在 demo 目录下）：
#   make            编译
#   make run        编译并运行
#   make clean      清理产物
#
# Windows 下产物是 hello.exe，Linux/Mac 是 hello

CC      := gcc
CFLAGS  := -Wall -Wextra -O0 -std=c11 -g
TARGET  := hello
SRC     := main.c

# 默认目标
all: $(TARGET)

# 链接规则（注意下面这行必须用 TAB 缩进，不能用空格！）
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

# 运行
run: $(TARGET)
	./$(TARGET)

# 清理
clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all run clean
