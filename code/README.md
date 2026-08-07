# 练习代码

放学习过程的 demo 代码、知识点验证、算法练习。

## 建议子目录

- `pointer/` - 指针 demo（值传递、二级指针、const）
- `struct/` - 结构体对齐、union、位域
- `macro/` - 宏练习（MAX、ARRAY_SIZE、container_of）
- `stm32-hal/` - HAL 库练习（GPIO、UART、TIMER）
- `stm32-register/` - 寄存器级操作
- `freertos/` - FreeRTOS 任务、信号量、队列

## 每个 demo 的结构

```
demo-name/
├── main.c          # 主程序
├── README.md       # 一句话说明这个 demo 验证什么
└── Makefile        # 或 cubeIDE 工程文件
```

## 原则

- **能跑就行** —— 不追求完美
- **每个 demo 验证一个知识点** —— 别贪多
- **注释写清楚为什么这么写** —— 以后回来看不用重新理解