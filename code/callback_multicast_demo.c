/*
 * callback_multicast_demo.c -- multi-subscriber callback (publish / subscribe)
 *
 * Build & run:
 *   gcc -std=c11 -Wall -Wextra -o callback_multicast_demo callback_multicast_demo.c
 *   ./callback_multicast_demo
 *
 * One event source, N subscribers. Static array instead of malloc because
 * this is the pattern used on MCUs: fixed capacity, deterministic timing.
 */
#include <stdio.h>
#include <stdint.h>

#define MAX_SUB 4

typedef void (*rx_cb_t)(const uint8_t *data, uint16_t len);

static rx_cb_t g_subs[MAX_SUB];        /* NULL slot = free */

/* returns slot index on success, -1 on failure (NULL cb or list full) */
static int uart_subscribe(rx_cb_t cb)
{
    if (!cb) return -1;

    for (int i = 0; i < MAX_SUB; i++)          /* already subscribed? */
        if (g_subs[i] == cb) return i;

    for (int i = 0; i < MAX_SUB; i++)          /* find a free slot */
        if (g_subs[i] == NULL) { g_subs[i] = cb; return i; }

    return -1;                                  /* full */
}

/* returns slot index that was freed, -1 if not found */
static int uart_unsubscribe(rx_cb_t cb)
{
    for (int i = 0; i < MAX_SUB; i++)
        if (g_subs[i] == cb) { g_subs[i] = NULL; return i; }
    return -1;
}

static void uart_notify(const uint8_t *data, uint16_t len)
{
    for (int i = 0; i < MAX_SUB; i++)
        if (g_subs[i]) g_subs[i](data, len);    /* skip NULL slots */
}

/* ---------- three independent subscriber modules ---------- */
static void on_log(const uint8_t *data, uint16_t len)
{
    printf("   [log]     store %u bytes to flash, head=0x%02X\n", len, data[0]);
}

static void on_parse(const uint8_t *data, uint16_t len)
{
    printf("   [parse]   cmd=0x%02X len=%u\n", data[0], len);
}

static void on_ui(const uint8_t *data, uint16_t len)
{
    printf("   [ui]      refresh screen, arg=0x%02X (%u bytes)\n", data[1], len);
}

static void on_debug(const uint8_t *data, uint16_t len)
{
    printf("   [debug]   head=0x%02X len=%u\n", data[0], len);
}

static void on_monitor(const uint8_t *data, uint16_t len)
{
    printf("   [monitor] forward %u bytes to PC, head=0x%02X\n", len, data[0]);
}

/* this one never gets in: the list is already full */
static void on_extra(const uint8_t *data, uint16_t len)
{
    printf("   [extra]   SHOULD NOT PRINT: head=0x%02X len=%u\n", data[0], len);
}

int main(void)
{
    uint8_t frame[4] = { 0xA5, 0x01, 0x02, 0x5A };
    int slot;

    printf("1) no subscriber yet\n");
    uart_notify(frame, 4);
    printf("   (nobody listens, frame dropped)\n");

    printf("2) three modules subscribe\n");
    printf("   on_log   -> slot %d\n", uart_subscribe(on_log));
    printf("   on_parse -> slot %d\n", uart_subscribe(on_parse));
    printf("   on_ui    -> slot %d\n", uart_subscribe(on_ui));
    uart_notify(frame, 4);

    printf("3) subscribing twice does NOT duplicate\n");
    printf("   on_log again -> slot %d (same slot)\n", uart_subscribe(on_log));
    uart_notify(frame, 4);

    printf("4) unsubscribe on_parse\n");
    printf("   on_parse removed from slot %d\n", uart_unsubscribe(on_parse));
    uart_notify(frame, 4);

    printf("5) capacity limit (MAX_SUB = %d)\n", MAX_SUB);
    printf("   on_debug   -> slot %d\n", uart_subscribe(on_debug));
    printf("   on_monitor -> slot %d\n", uart_subscribe(on_monitor));
    slot = uart_subscribe(on_extra);
    printf("   on_extra   -> slot %d (list full, rejected)\n", slot);
    uart_notify(frame, 4);

    return 0;
}
