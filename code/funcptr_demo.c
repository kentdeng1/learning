/*
 * funcptr_demo.c -- function pointers in C: address, callback, ops table, jump table
 *
 * Build & run:
 *   gcc -std=c11 -Wall -Wextra -o funcptr_demo funcptr_demo.c && ./funcptr_demo
 *
 * Demonstrates the four things you actually use in embedded work:
 *   1. a function name decays to its entry address (hello == &hello)
 *   2. callback: lower layer calls upper layer when an event happens
 *   3. ops table: upper layer picks an implementation at runtime (C "polymorphism")
 *   4. jump table: array of handlers replacing a long switch-case
 */
#include <stdio.h>
#include <stdint.h>

/* ---------- 1. function name == entry address ---------- */
static void hello(int n)
{
    printf("   hello(%d) called\n", n);
}

static void demo_address(void)
{
    void (*fp)(int) = hello;

    printf("1) function name is just an address\n");
    printf("   hello=%p  &hello=%p  fp=%p\n",
           (void *)hello, (void *)&hello, (void *)fp);
    fp(1);          /* modern style  */
    (*fp)(2);       /* classic style, identical result */
}

/* ---------- 2. callback ---------- */
typedef void (*rx_cb_t)(const uint8_t *data, uint16_t len);

static rx_cb_t g_rx_cb = NULL;

static void uart_register_rx_cb(rx_cb_t cb)
{
    g_rx_cb = cb;
}

/* pretending this is an ISR in the driver layer */
static void uart_isr_simulate(const uint8_t *data, uint16_t len)
{
    if (g_rx_cb) {
        g_rx_cb(data, len);          /* always check for NULL first! */
    } else {
        printf("   (no callback registered, frame dropped)\n");
    }
}

static void on_frame(const uint8_t *data, uint16_t len)
{
    printf("   [app layer] got frame len=%u first=0x%02X\n", len, data[0]);
}

static void demo_callback(void)
{
    uint8_t frame[4] = { 0xAA, 0x01, 0x02, 0xBB };

    printf("2) callback (lower layer notifies upper layer)\n");
    uart_isr_simulate(frame, 4);      /* before registration: dropped */
    uart_register_rx_cb(on_frame);
    uart_isr_simulate(frame, 4);      /* now the app gets it */
}

/* ---------- 3. ops table ---------- */
typedef struct {
    const char *name;
    void (*init)(void);
    int  (*write)(uint8_t addr, const uint8_t *buf, int len);
} i2c_ops_t;

static void hw_init(void) { printf("   hw i2c: init done\n"); }
static int  hw_write(uint8_t addr, const uint8_t *buf, int len)
{
    printf("   hw i2c: write addr=0x%02X len=%d first=0x%02X\n", addr, len, buf[0]);
    return 0;
}

static void sw_init(void) { printf("   sw i2c: init done (bit-bang)\n"); }
static int  sw_write(uint8_t addr, const uint8_t *buf, int len)
{
    printf("   sw i2c: bit-bang write addr=0x%02X len=%d first=0x%02X\n", addr, len, buf[0]);
    return 0;
}

/* const => table lives in flash, not copied to RAM */
static const i2c_ops_t hw_ops = { "HW-I2C", hw_init, hw_write };
static const i2c_ops_t sw_ops = { "SW-I2C", sw_init, sw_write };

/* upper layer knows only the interface */
static void sensor_app(const i2c_ops_t *ops, const uint8_t *cfg, int len)
{
    printf("   sensor_app running on %s\n", ops->name);
    ops->init();
    ops->write(0x68, cfg, len);
}

static void demo_ops_table(void)
{
    uint8_t cfg[2] = { 0x10, 0x20 };

    printf("3) ops table (swap implementation, app code untouched)\n");
    sensor_app(&hw_ops, cfg, 2);
    sensor_app(&sw_ops, cfg, 2);
}

/* ---------- 4. jump table instead of switch-case ---------- */
enum { CMD_READ = 0, CMD_WRITE, CMD_RESET, CMD_MAX };

typedef void (*cmd_fn_t)(void);

static void h_read(void)  { printf("   handling READ\n"); }
static void h_write(void) { printf("   handling WRITE\n"); }
static void h_reset(void) { printf("   handling RESET\n"); }

static const cmd_fn_t cmd_table[CMD_MAX] = {
    [CMD_READ]  = h_read,
    [CMD_WRITE] = h_write,
    [CMD_RESET] = h_reset,
};

static void dispatch(int id)
{
    if (id >= 0 && id < CMD_MAX && cmd_table[id]) {
        cmd_table[id]();
    } else {
        printf("   bad command id=%d\n", id);
    }
}

static void demo_jump_table(void)
{
    printf("4) jump table (command dispatcher)\n");
    dispatch(CMD_WRITE);
    dispatch(CMD_RESET);
    dispatch(99);
}

int main(void)
{
    demo_address();
    demo_callback();
    demo_ops_table();
    demo_jump_table();
    return 0;
}
