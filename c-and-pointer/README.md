# c-and-pointer —— 《C和指针》练习工程

配套《C和指针》(Pointers on C, Kenneth A. Reek) 的动手练习骨架，Makefile 驱动。

## 目录结构

```
c-and-pointer/
├── Makefile          # 构建脚本（每个 src/*.c 编译成独立可执行文件）
├── src/              # 你的练习代码放这里（ex01/ex02 是示例）
├── include/          # 头文件
├── build/            # 编译产物（git 忽略）
└── book-code/        # 官方配套代码（按章节 ch1..ch19）
    ├── programs.zip  # 官方下载的压缩包
    └── extracted/    # 解压后的源码，按章节组织
```

## 构建与运行

Windows（已装 mingw64）：
```bash
mingw32-make          # 编译 src/ 下所有 .c -> build/
mingw32-make run      # 编译并逐个运行
mingw32-make clean    # 清 build/
```

macOS / Linux：
```bash
make
make run
```

## 编译官方示例代码

官方代码在 `book-code/extracted/chXX/` 下，各自有 `main`，单独编译即可：
```bash
gcc book-code/extracted/ch7/parity.c -o build/parity && ./build/parity
```

## 阅读计划

见同项目《阅读作战图.md》：P0 = Ch3 数据(const) / Ch6 指针 / Ch7 函数(值传递) / Ch8 数组 / Ch14 预处理器(宏)。
