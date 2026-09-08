# C 函数指针、回调与接口设计

> 日期:2026-09-08 | 主题:C 核心机制
> 配套 demo:`code/funcptr_demo.c`、`code/callback_multicast_demo.c`
> 关联:`notes/c-bitfield-macro-cheatsheet.md`

## 一句话定义

函数指针存的是**函数的入口地址**,它让"调用哪个函数"从编译期推迟到运行期。这是 C 做分层解耦与"多态"的唯一手段。

## 1. 本质:函数名就是地址

```c
void (*fp)(int) = hello;    /* hello / &hello / *hello 全是同一个入口地址 */
fp(1);                      /* 调用 */
(*fp)(2);                   /* 老式写法, 完全等价 */
```

| 写法 | 含义 |
|---|---|
| `void (*fp)(int)` | 函数指针 |
| `void *fp(int)` | **返回指针的函数**(括号位置不同, 完全不同) |

工程里一律 typedef,裸写不可读:

```c
typedef void (*rx_cb_t)(uint8_t *data, uint16_t len);
rx_cb_t g_cb = NULL;
if (g_cb) g_cb(buf, len);       /* 调用前必须判空 */
```

## 2. 两种工程模式(必须区分)

| | 回调 callback | 接口表 ops / vtable |
|---|---|---|
| 本质 | 控制反转 IoC | C 的多态 |
| 谁决定 | 下层决定"**何时**调用" | 上层决定"调用**哪个**实现" |
| 方向 | 上层把函数交给下层 | 上层持有一组函数指针 |
| 典型 | ISR 通知、事件上报 | 驱动抽象、可替换硬件 |

两者常一起用:底层用 ops 表抽象硬件,同时向上提供 `register_cb()`。

### 2.1 回调模式

```c
/* ---- 下层 driver: 不知道上层存在 ---- */
static rx_cb_t g_rx_cb = NULL;

void uart_register_rx_cb(rx_cb_t cb) { g_rx_cb = cb; }   /* 只提供登记窗口 */

void USART1_IRQHandler(void)
{
    if (g_rx_cb) g_rx_cb(rx_buf, len);   /* 判空! 空指针调用 = HardFault */
}

/* ---- 上层 app ---- */
static void on_frame(uint8_t *d, uint16_t len) { parse_protocol(d, len); }

int main(void)
{
    uart_register_rx_cb(on_frame);       /* 注册: 上层主动做, 在 init 阶段 */
    while (1) { /* 主循环不用管串口 */ }
}
```

**关键点:注册是上层自己做的,不是下层自动发生。**

- **t0 注册**:上层在 main/init 调用 `register_xxx()`,只把地址存进变量,**此刻不发生任何回调**
- **t1 触发**:事件到来,下层按存下的地址跳过去;下层**根本不知道**那是哪个函数

为什么绕这一圈?不用回调就得:

```c
extern void on_frame(uint8_t*, uint16_t);   /* 驱动被迫知道应用层函数名 */
void USART1_IRQHandler(void) { on_frame(buf, len); }   /* 写死 */
```

后果:换项目要改驱动、驱动无法独立复用、只能通知一个地方。

类比:注册 = 把手机号留给快递员(你主动留);回调 = 快递到了他打给你。

### 2.2 ops 接口表

```c
typedef struct {
    void (*init)(void);
    int  (*write)(uint8_t addr, const uint8_t *buf, uint16_t len);
} i2c_ops_t;

static const i2c_ops_t hw_i2c_ops = { .init = hw_i2c_init, .write = hw_i2c_write };
static const i2c_ops_t sw_i2c_ops = { .init = sw_i2c_init, .write = sw_i2c_write };

void sensor_app(const i2c_ops_t *ops)      /* 上层只认接口 */
{
    ops->init();
    ops->write(0x68, cfg, sizeof cfg);
}
/* 换硬件 = 换一张表, 上层一行不动 */
```

`const` 别漏:大表加 const 才放 Flash,否则启动时整表被拷进 RAM。

## 3. 多订阅(发布-订阅 pub/sub)

单个指针 → 订阅者数组,一个事件通知 N 个模块。

```c
#define MAX_SUB 4
static rx_cb_t g_subs[MAX_SUB];        /* NULL = 空位 */

int uart_subscribe(rx_cb_t cb)
{
    if (!cb) return -1;
    for (int i = 0; i < MAX_SUB; i++)          /* 去重 */
        if (g_subs[i] == cb) return i;
    for (int i = 0; i < MAX_SUB; i++)          /* 找空位 */
        if (g_subs[i] == NULL) { g_subs[i] = cb; return i; }
    return -1;                                  /* 满了 */
}

int uart_unsubscribe(rx_cb_t cb)
{
    for (int i = 0; i < MAX_SUB; i++)
        if (g_subs[i] == cb) { g_subs[i] = NULL; return i; }
    return -1;
}

void uart_notify(const uint8_t *d, uint16_t len)
{
    for (int i = 0; i < MAX_SUB; i++)
        if (g_subs[i]) g_subs[i](d, len);       /* 跳过 NULL */
}
```

**工程要点**:
- MCU 上用静态数组(无 malloc、时长确定);数量不定才用链表/静态池
- **注销比注册更重要**:模块停用必须 unsubscribe,否则表里留野指针
- 容量满要明确策略(返回错误,别静默覆盖)
- 通知顺序 = slot 顺序,模块间**不要依赖顺序**
- **ISR 上下文**:notify 若在 ISR 里跑 → 订阅/注销要临界区保护;回调一多会拉长 ISR → **推荐 ISR 只置标志/推队列,主循环再 notify**
- RTOS 下更推荐**事件组 / 队列**(`xEventGroupSetBitsFromISR()`)替代多播回调:逻辑跑在任务上下文,可阻塞、不受 ISR 约束

## 4. 嵌入式里的真实例子

| 例子 | 属于哪种 |
|---|---|
| 中断向量表 `__Vectors` | 函数指针数组(最底层应用) |
| `xTaskCreate(task_func, ...)` | 回调(OS 调度它) |
| 软件定时器回调、Idle Hook | 回调 |
| HAL `HAL_UART_RegisterCallback()` | 回调(真·函数指针) |
| RT-Thread `rt_device_ops`、Linux `file_operations` | ops 表 |
| `qsort` 的 comparator | 回调 |
| 命令表 `cmd_table[id]()` / 状态机表 | 跳转表(替代 switch-case) |

⚠️ **加分细节**:HAL 传统的 `HAL_UART_RxCpltCallback` **不是**函数指针,是 `__weak` 弱定义 + 链接期覆盖;新版 `HAL_UART_RegisterCallback()` 才是真回调。

## 5. 坑清单

1. 括号:`void (*fp)(int)` 是指针,`void *fp(int)` 是返回指针的函数
2. 类型必须完全匹配(返回值 + 全部参数),`-Wincompatible-pointer-types` 别忽略
3. **判空**:调 NULL = 跳地址 0 = HardFault
4. **无法内联**:间接调用编译器通常内联不了 → 高频路径(每字节处理)慎用
5. ISR 里的回调:不能阻塞、不能 malloc、不能调非可重入函数(如 printf)
6. const 表放 Flash;大表漏 const 会占 RAM
7. 生命周期:注册的函数不能是即将失效的代码
8. 回调执行时间计入调用者(在 ISR 里就是计入 ISR)

## 6. 面试问答

**Q: 函数指针是什么?工程上用来做什么?**
A: 存函数入口地址,把"调用谁"推迟到运行期。两种用法:回调实现控制反转(下层通知上层),函数指针表实现 C 的多态(上层可替换实现)。代价是间接调用无法内联。

**Q: 回调是谁注册的?什么时候触发?**
A: 上层代码在 init 阶段主动调用下层提供的 register 接口,把地址存进下层的函数指针变量(此刻不调用);之后事件到来时,下层按该地址跳过去执行,下层并不知道那是哪个函数。

**Q: 为什么下层不直接调用上层函数?**
A: 那样下层必须 extern/include 上层,造成反向依赖:换项目要改驱动、驱动无法独立复用、只能通知一处。回调让驱动零依赖上层。

**Q: 多个模块都要收到通知怎么办?**
A: 订阅表(数组/链表),notify 时遍历;注意注销防野指针、容量策略。若在 RTOS 中且回调较重,用事件组/队列替代,把处理放到任务上下文。
