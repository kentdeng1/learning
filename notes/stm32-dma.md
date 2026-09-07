# STM32 DMA 笔记

> 日期:2026-09-07 | 状态:M2 外设学习
> 关联:`notes/c-bitfield-macro-cheatsheet.md`(位操作/宏)

## 一句话定义

DMA 是独立于 CPU 的数据搬运控制器。CPU 只下配置(源地址、目的地址、长度、方向、模式),传输过程完全不占用 CPU,搬完/半满时用一次中断通知。

**核心价值不是"搬得更快",是"把 CPU 从搬运中解放出来"**。

## 1. 三种传输方式对比

| 方式 | CPU 占用 | 适合场景 |
|---|---|---|
| 轮询 polling | 100% 守着 | 极少量数据、调试 |
| 中断 ISR | 每字节/每事件一次 | 中小数据量、低速 |
| DMA | 全程 1 次中断 | 大块数据、高速外设、连续流 |

## 2. 硬件架构

| | F1 | F4/F7 |
|---|---|---|
| 控制器 | DMA1(7 ch) / DMA2(5 ch) | DMA1/DMA2 各 8 Stream |
| 请求源 | 通道与外设**固定映射**(查表) | 每 Stream 用 CHSEL 选 8 源 |
| FIFO | 无 | 有(突发/阈值) |

**仲裁**:软件优先级(very high~low)+ 硬件优先级(编号小的优先)。

**传输要素 6 项**:源地址 / 目的地址 / 地址自增 / 数据宽度 / 数量(CNDTR, 上限 65535) / 方向(P2M、M2P、M2M) / 模式(Normal、Circular)。

**触发机制**:不是软件启动,而是**外设请求信号**(UART RXNE/TXE、ADC EOC、TIM 更新)。所以除了配 DMA,**必须在外设侧开 DMA 使能位**。

| 外设 | 使能位 |
|---|---|
| USART | `USART_CR3` 的 `DMAT` / `DMAR` |
| ADC | `ADC_CR2` 的 `DMA`(F1) / `ADC_CFGR` 的 `DMAEN`(F4+) |
| SPI | `SPI_CR2` 的 `TXDMAEN` / `RXDMAEN` |
| I2C | `I2C_CR2` 的 `DMAEN` |

### F1 关键寄存器

| 寄存器 | 作用 |
|---|---|
| `DMA_CPARx` | 外设地址(通常 `&USARTx->DR`) |
| `DMA_CMARx` | 内存地址(buffer) |
| `DMA_CNDTRx` | 待传输数量 |
| `DMA_CCRx` | 配置:`EN/TCIE/HTIE/TEIE/DIR/CIRC/PINC/MINC/PSIZE/MSIZE/PL` |
| `DMA_ISR` / `DMA_IFCR` | 中断状态 / 标志清除(写 1 清除) |

## 3. 寄存器级配置(F1,USART1 RX)

```c
DMA1_Channel5->CPAR  = (uint32_t)&USART1->DR;
DMA1_Channel5->CMAR  = (uint32_t)rx_buf;
DMA1_Channel5->CNDTR = RX_LEN;
DMA1_Channel5->CCR   = DMA_CCR_MINC            /* 内存自增 */
                     | DMA_CCR_CIRC            /* 循环 */
                     | DMA_CCR_TCIE | DMA_CCR_HTIE
                     | DMA_CCR_EN;
USART1->CR3 |= USART_CR3_DMAR;                 /* 别漏! */
```

要点:外设地址**不自增**(永远同一个 DR),内存地址自增。

## 4. 实战模板

### 4.1 串口 DMA + IDLE 收不定长帧(最常用)

```c
uint8_t rx_buf[256];        /* 全局/static, 禁止局部变量 */

void uart_dma_start(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buf, sizeof(rx_buf));
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1) {
        process_frame(rx_buf, Size);           /* Size = 本帧长度 */
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buf, sizeof(rx_buf));
        __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
    }
}
```

老版本 HAL 无 `ReceiveToIdle` 时:手动开 `__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE)`,在 `USART1_IRQHandler` 里清 IDLE(读 SR 再读 DR),长度 = `sizeof(buf) - __HAL_DMA_GET_COUNTER(hdma)`。

### 4.2 ADC 多通道连续采样

```c
uint16_t adc_buf[CH_NUM * N];      /* 必须 uint16_t, half-word */

HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, CH_NUM * N);
```

CubeMX:ADC = Scan + Continuous;DMA = Circular、Data Width = Half Word。
效果:`adc_buf` 被自动循环刷新,CPU 随时读最新值,零干预。

### 4.3 Circular 双缓冲(ping-pong)

- CNDTR 自动重装,永不停
- HT(半传输)中断 → 处理前半;TC(完成)中断 → 处理后半
- **硬约束**:CPU 处理半个 buffer 的时间 < DMA 填满另半个的时间,否则覆盖

## 5. 运用场景判断

| 场景 | 配置 | 价值 |
|---|---|---|
| ADC 多通道连续采样 | circular + half-word | 采样率稳定 |
| 串口收不定长 | circular + IDLE | 不丢字节、帧边界清晰 |
| 串口发大块数据 | normal + TC 中断 | 发送不阻塞 |
| SPI 刷屏/读 Flash | TX/RX DMA | 帧率提升 |
| DAC 波形/音频 | circular from 表 | 自动循环 |

**判断标准**:数据量大(>几十字节)或速率高(>几十 kHz)才值得上 DMA;偶尔几个字节用轮询更简单。

## 6. 坑清单(重点)

1. **外设侧 DMA 使能位必须开** — 只配 DMA 通道永远不动
2. **buffer 不能是局部变量** — 栈失效后 DMA 写野地址 → HardFault
3. **中断标志要清** — F1 用 `DMA_IFCR` 写 1 清;不清 = 中断风暴
4. **CNDTR 上限 65535** — 更大批量分批或双缓冲
5. **RS485 方向切换** — DMA TC 只代表最后一字节进了 DR,要等 USART 的 TC 再切 DE
6. **Normal 模式搬完 EN 自动清零** — 重启需重设 CNDTR
7. **宽度与对齐** — half-word 需 2 字节对齐;12 位 ADC 别用 uint8_t 收
8. **Cache 一致性(F7/H7)** — `SCB_CleanDCache()`(发送前)/ `SCB_InvalidateDCache()`(接收后),或 MPU 设 non-cacheable
9. **H7 的 DTCM(0x20000000)不能被通用 DMA 访问** — buffer 放 AXI SRAM 或用 MDMA
10. **DMA 不是零成本** — 与 CPU 争总线矩阵,高频小批量反而更慢
11. **M2M 与 CIRC 不能同时使能(F1)**
12. **volatile / 内存屏障** — DMA 写的 buffer 被 CPU 读,注意优化与 `__DMB()`

## 7. 面试问答

**Q: DMA 相比中断方式好在哪?**
A: 中断是每字节进一次 ISR(压栈/跳转开销 × N),DMA 是整块数据只进一次。CPU 占用从 O(N) 降到 O(1),且期间可以 sleep。

**Q: DMA 传输完全不需要 CPU 吗?**
A: 配置阶段需要;传输期间不需要,但 DMA 与 CPU 共享总线矩阵,存在总线争用,并非零成本。

**Q: 怎么知道 DMA 收到了多少字节?**
A: `已收 = 设定总长 - CNDTR`(HAL 用 `__HAL_DMA_GET_COUNTER()`)。配合 IDLE 中断即可确定不定长帧边界。

**Q: Circular 和 Normal 区别?**
A: Normal 搬完 EN 位自动清零,一次结束;Circular 自动重装 CNDTR 并循环,用于连续流(ADC/音频)。

**Q: 为什么配了 DMA 数据不动?**
A: 三查:① 外设侧 DMA 使能位开了吗 ② 时钟(DMA 控制器时钟)开了吗 ③ buffer 地址和 CNDTR 设对了吗
