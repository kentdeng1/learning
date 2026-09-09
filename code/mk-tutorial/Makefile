# ============================================================
#  Makefile 教学模板 (多文件版)
#
#  用法:  make          编译
#         make run      编译并运行
#         make clean    删除产物
#         make rebuild  先清后编
#
#  建议亲手试这几个命令, 观察"只重编必要部分":
#    1) make                    首次: 编译 2 个 .o 再链接
#    2) make                    第二次: 什么都不做
#    3) touch calc.c && make    只重编 calc.o, 然后重新链接
#    4) touch calc.h && make    两个 .o 都重编 (头文件被两个 .c 包含)
# ============================================================

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -O0 -g
EXE     := .exe

TARGET  := app
SRCS    := main.c calc.c
OBJS    := $(SRCS:.c=.o)          # main.c calc.c -> main.o calc.o
DEPS    := $(OBJS:.o=.d)          # main.o calc.o -> main.d calc.d

.PHONY: all run clean rebuild shell-demo

all: $(TARGET)$(EXE)

# 链接: $@ = app.exe,  $^ = main.o calc.o
$(TARGET)$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 编译: $< = 那个 .c
# -MMD -MP 让 gcc 顺带生成 .d 依赖文件(记录它包含了哪些 .h)
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

# 把 .d 文件包含进来: 改了 .h 也能触发重编
-include $(DEPS)

run: $(TARGET)$(EXE)
	./$(TARGET)$(EXE)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)$(EXE)

rebuild: clean all

# 证明 recipe 就是普通的 shell 命令: 试试 make shell-demo
# 注意: 每一行 recipe 在独立的 shell 里执行, 所以 "cd xxx" 不会影响下一行
shell-demo:
	@echo "== this line runs in a shell =="
	pwd
	ls *.c
	@echo "== gcc is just another shell command =="
	gcc --version | head -1
