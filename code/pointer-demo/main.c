/*
 * pointer-demo —— 嵌入式 C 指针知识点学习 Demo
 *
 * 背景：从 8 位 MCU FAE 转型嵌入式软件工程师。
 * 指针是 C 语言里嵌入式最常用、也最容易出 bug 的知识点，
 * 本 demo 用"跑起来看输出"的方式，把指针相关的核心概念过一遍。
 *
 * 用法（本目录下）：
 *   make       编译
 *   make run   编译并运行
 *
 * 建议：边跑边把每个 printf 和它上面那一行代码对应起来看。
 * 注意：本 demo 在 PC（MinGW gcc）上运行，验证的是语法/语义；
 *       真正的硬件寄存器访问需要接板子，但套路和这里一模一样。
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/* 源文件是 UTF-8，而 Windows 控制台默认用 GBK 解码，
 * 强制把控制台输出代码页切成 UTF-8，避免中文乱码。 */
#ifdef _WIN32
#include <windows.h>
#endif

/* ------------------------------------------------------------------
 * 工具函数：打印分节标题，让输出好对照
 * ---------------------------------------------------------------- */
static void banner(const char *title)
{
    printf("\n========== %s ==========\n", title);
}

/* ------------------------------------------------------------------
 * 1. 指针是什么：一个保存"地址"的变量，同时带"类型"
 *    类型决定：解引用时读几个字节、指针加减时的步长
 * ---------------------------------------------------------------- */
static void sec1_basic(void)
{
    banner("1. 指针 = 地址 + 类型");

    int  x = 42;
    int *p = &x;          /* &x 取出 x 的地址，存到 p 里 */

    printf("x   的值    = %d\n", x);
    printf("&x  的地址  = %p\n", (void *)&x);
    printf("p   保存的值= %p   （和 &x 一样）\n", (void *)p);
    printf("*p  解引用  = %d   （和 x 一样）\n", *p);

    *p = 100;             /* 通过指针改内存，x 跟着变 */
    printf("执行 *p = 100 之后：x = %d\n", x);

    printf("\nsizeof(指针)由地址总线宽度决定：\n");
    printf("  sizeof(int *)  = %zu\n", sizeof(int *));
    printf("  sizeof(char *) = %zu\n", sizeof(char *));
    printf("  -> 桌面 64 位是 8 字节；32 位 MCU（STM32）是 4 字节。\n");
    printf("  指针大小和它指向什么类型无关，类型决定的是解引用读几字节：\n");
    printf("  sizeof(int)=%zu, sizeof(char)=%zu\n", sizeof(int), sizeof(char));
}

/* ------------------------------------------------------------------
 * 2. 指针与数组：数组名 = 首元素地址
 *    指针 + i 的步长是"指向类型的大小"，不是 1 字节
 *    arr[i] 本质就是 *(arr + i)
 * ---------------------------------------------------------------- */
static void sec2_array(void)
{
    banner("2. 数组与指针：数组名是首元素地址，加减按类型步长");

    int arr[4] = {10, 20, 30, 40};

    printf("arr     = %p （数组名，自动转为首元素地址）\n", (void *)arr);
    printf("&arr[0] = %p\n", (void *)&arr[0]);
    printf("arr == &arr[0] ? %s\n", arr == &arr[0] ? "是" : "否");

    printf("\n指针 + 1 跳过一个元素，而不是 1 个字节：\n");
    int *ip = arr;
    printf("  ip+0 -> %p  *(ip+0) = %d\n", (void *)(ip + 0), *(ip + 0));
    printf("  ip+1 -> %p  *(ip+1) = %d\n", (void *)(ip + 1), *(ip + 1));
    printf("  ip+2 -> %p  *(ip+2) = %d\n", (void *)(ip + 2), *(ip + 2));
    printf("  相邻地址差 = %ld 字节 = sizeof(int)\n",
           (long)((char *)(ip + 1) - (char *)ip));

    printf("\n下标就是指针运算的语法糖：arr[i] == *(arr + i)\n");
    for (int i = 0; i < 4; i++)
        printf("  arr[%d] = %d, *(arr+%d) = %d\n", i, arr[i], i, *(arr + i));
}

/* ------------------------------------------------------------------
 * 3. 值传递 vs 指针传递
 *    函数形参是普通变量时，改的是副本；想改实参，必须传地址
 *    嵌入式里驱动函数的收发缓冲、采样数据，全是指针传递
 * ---------------------------------------------------------------- */
static void swap_by_value(int a, int b)   /* 形参是副本，改不动外面 */
{
    int t = a; a = b; b = t;
}

static void swap_by_pointer(int *a, int *b)  /* 传地址，直接改内存 */
{
    int t = *a; *a = *b; *b = t;
}

static void sec3_swap(void)
{
    banner("3. 值传递 vs 指针传递：想改实参必须传地址");

    int a = 1, b = 2;

    swap_by_value(a, b);
    printf("值传递后   ：a=%d b=%d   （没变，函数内改的是副本）\n", a, b);

    swap_by_pointer(&a, &b);
    printf("指针传递后 ：a=%d b=%d   （变了，拿到地址直接改内存）\n", a, b);

    printf("\n嵌入式场景：串口收发、传感器采集、协议解析，都是把缓冲区/结构体的\n");
    printf("地址传给驱动函数，否则函数内填的数据全部丢失。\n");
}

/* ------------------------------------------------------------------
 * 4. const 的三种位置：const 修饰谁，谁就只读
 *    嵌入式里读外设寄存器/读 Flash 数据时习惯加 const 防误写
 * ---------------------------------------------------------------- */
static void sec4_const(void)
{
    banner("4. const 与指针：const 修饰谁，谁只读");

    int x = 10, y = 20;

    const int *p1 = &x;      /* ① 目标只读：*p1 不可写，p1 可以改指向 */
    /* *p1 = 99; */          /* 这行要是去掉注释，编译报错 */
    p1 = &y;                 /* 可以 */

    int *const p2 = &x;      /* ② 指针本身只读：p2 不可改，*p2 可以写 */
    *p2 = 99;                /* 可以 */
    /* p2 = &y; */           /* 这行要是去掉注释，编译报错 */

    const int *const p3 = &x;/* ③ 双 const：两个都只读 */

    printf("  ① const int *p1      ：目标只读、可改 p1    -> p1 指向 %p\n", (void *)p1);
    printf("  ② int *const p2      ：p2 只读、可改 *p2    -> *p2=99 已生效，x=%d\n", x);
    printf("  ③ const int *const p3：双只读               -> p3 指向 %p\n", (void *)p3);

    printf("\n嵌入式场景：形参写成 const 指针 = 告诉调用者\"我只读不改\"，\n");
    printf("编译器还会帮你拦下误写，是驱动接口的规范写法。\n");
}

/* ------------------------------------------------------------------
 * 5. 结构体指针：传地址避免整块拷贝，还能直接改实参
 *    结构体往往几十字节，传值每次都要整块拷贝，传指针只传 4 字节
 * ---------------------------------------------------------------- */
typedef struct {
    uint16_t adc_raw;      /* ADC 原始采样值     */
    int16_t  temp_c;       /* 温度，单位 0.1 ℃   */
    uint8_t  valid;        /* 数据有效标志       */
} sensor_data_t;

static void process_by_value(sensor_data_t s)
{
    s.temp_c += 10;        /* 改的是副本 */
}

static void process_by_pointer(sensor_data_t *s)
{
    s->temp_c += 10;       /* 改的是实参，-> 等价于 (*s).temp_c */
}

static void sec5_struct(void)
{
    banner("5. 结构体指针：传地址而非整块拷贝");

    sensor_data_t s = {1024, 250, 1};

    printf("结构体大小 = %zu 字节\n", sizeof(sensor_data_t));
    printf("处理前 temp_c = %d\n", s.temp_c);

    process_by_value(s);
    printf("传值后   temp_c = %d  （没变，拷了一份改）\n", s.temp_c);

    process_by_pointer(&s);
    printf("传址后   temp_c = %d  （变了）\n", s.temp_c);

    printf("\n嵌入式场景：传感器数据结构体常有大数组/长结构，传指针只要 4 字节。\n");
}

/* ------------------------------------------------------------------
 * 6. 二级指针：函数想改"指针变量本身"，就要传指针的地址
 *    驱动里初始化句柄、释放内存置空，都用它
 * ---------------------------------------------------------------- */
static void change_int(int *p)     /* 一级指针：只能改 p 指向的内容 */
{
    *p = 999;
}

static void set_null_via_pp(int **pp)  /* 二级指针：改外面的指针本身 */
{
    *pp = NULL;
}

static void sec6_pp(void)
{
    banner("6. 二级指针：改指针本身要用指针的地址");

    int  x = 5;
    int *p = &x;

    change_int(p);                   /* 改内容：有效 */
    printf("change_int(p) 后：x = %d\n", x);

    printf("\n想让函数把 q 本身置成 NULL，只传 q 不够：\n");
    int *q = &x;
    printf("  函数调用前 q = %p\n", (void *)q);

    set_null_via_pp(&q);             /* 传 q 的地址（即 int**） */
    printf("  set_null_via_pp(&q) 后 q = %p %s\n",
           (void *)q, q == NULL ? "（已经是 NULL）" : "");

    printf("\n嵌入式场景：驱动里的\"设备句柄\"常常就是指针，\n");
    printf("init 函数要真正把句柄写出去，形参就得是二级指针。\n");
}

/* ------------------------------------------------------------------
 * 7. 函数指针 + 回调：把函数当参数传
 *    中断向量表本质就是函数指针数组；RTOS 任务、串口/DMA 回调全靠它
 * ---------------------------------------------------------------- */
typedef void (*irq_handler_t)(void);

static void uart_isr(void)
{
    printf("    [UART_ISR] 串口收到一帧数据，开始解析\n");
}

static void timer_isr(void)
{
    printf("    [TIMER_ISR] 1ms 定时到，调度器被唤醒\n");
}

static void install_irq(uint32_t irq_no, irq_handler_t handler)
{
    const char *who = (handler == uart_isr) ? "uart_isr" : "timer_isr";
    printf("    注册 IRQ%lu -> %s\n", (unsigned long)irq_no, who);
}

static void sec7_funcptr(void)
{
    banner("7. 函数指针：回调与中断向量表");

    printf("  (1) 直接调用：\n");
    uart_isr();

    printf("  (2) 赋值给函数指针再调用：\n");
    irq_handler_t cb = uart_isr;      /* 函数名就是它的地址 */
    cb();                             /* cb 和 uart_isr 是同一个函数 */

    printf("  (3) 当参数传 = 注册回调（驱动最常用）：\n");
    install_irq(38, uart_isr);        /* USART1_IRQn   */
    install_irq(29, timer_isr);       /* TIM2_IRQn     */

    printf("\n嵌入式场景：中断向量表 = 函数指针数组；按键扫描、串口空闲中断+DMA 的\n");
    printf("回调、RTOS 任务函数，全是函数指针。\n");
}

/* ------------------------------------------------------------------
 * 8. 寄存器访问 = 固定地址 + 指针 + volatile
 *    硬件寄存器指针必须加 volatile，防止编译器"优化"掉真实读写
 *
 *    MCU 上的真实写法（STM32F4，仅示意，PC 上不能真跑）：
 *      #define GPIOA_ODR  (*(volatile uint32_t *)0x40020014UL)
 *      GPIOA_ODR = 0x00000001;        // PA0 输出高
 *    即：把地址常量强转成"指向 volatile uint32_t 的指针"，再解引用。
 * ---------------------------------------------------------------- */
static void sec8_register(void)
{
    banner("8. 寄存器访问：指针 + volatile（嵌入式灵魂）");

    static uint32_t      fake_uart_DR;      /* 假装这是一块寄存器内存 */
    volatile uint32_t   *reg = &fake_uart_DR; /* 驱动里的寄存器指针都这么写 */

    printf("  读寄存器 -> %u\n", (unsigned)*reg);
    *reg = 0xA5U;
    printf("  写寄存器后 -> 0x%02X\n", (unsigned)*reg);

    printf("\n  volatile 的意义：告诉编译器\"这块内存随时会被硬件/中断改\"，\n");
    printf("  每次访问都必须真读真写，不许缓存到寄存器、不许乱序。\n");
    printf("  漏掉 volatile 是驱动里最经典的坑（-O2 下行为才会暴露）。\n");
}

/* ------------------------------------------------------------------
 * 9. 空指针防御：解引用前先判空
 *    FreeRTOS 判句柄、链表判 NULL 结束、回调判空再调用，都是日常
 * ---------------------------------------------------------------- */
static void sec9_null(void)
{
    banner("9. 空指针防御");

    int  x = 7;
    int *p = &x;
    int *n = NULL;

    if (p)
        printf("  p 非空，安全解引用：*p = %d\n", *p);

    if (n)
        printf("  这行永远不会打印\n");
    else
        printf("  n 是 NULL，跳过解引用 -> 避免了段错误\n");

    printf("\n嵌入式场景：驱动入参判空是基本功；链表遍历到 NULL 结束；\n");
    printf("回调未注册时判空再调用，都是日常防御。\n");
}

/* ------------------------------------------------------------------
 * 主程序：按顺序跑各节
 * ---------------------------------------------------------------- */
int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   /* 修 Windows 控制台中文乱码 */
#endif
    printf("=== 嵌入式 C 指针学习 Demo ===\n");
    printf("提示：边跑边对照每节代码注释，把 printf 输出和代码对应起来。\n");

    sec1_basic();     /* 指针 = 地址 + 类型 */
    sec2_array();     /* 数组名 / 指针步长 / 下标 */
    sec3_swap();      /* 值传递 vs 指针传递 */
    sec4_const();     /* const 三种位置 */
    sec5_struct();    /* 结构体指针 */
    sec6_pp();        /* 二级指针 */
    sec7_funcptr();   /* 函数指针 / 回调 */
    sec8_register();  /* 寄存器访问 + volatile */
    sec9_null();      /* 空指针防御 */

    printf("\n=== 全部跑完。每节对应的知识点，去 notes/ 里总结成自己的话 ===\n");
    return 0;
}
