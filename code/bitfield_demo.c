/*
 * bitfield_demo.c -- bit-fields in embedded C: layout, packing, W1C trap
 *
 * Build & run:
 *   gcc -std=c11 -Wall -Wextra -o bitfield_demo bitfield_demo.c && ./bitfield_demo
 *
 * Key points demonstrated:
 *   1. First-declared member lands on the LSB (GCC/ARM, little-endian).
 *   2. Flag packing: 4 one-bit flags fit in 1 byte instead of 4.
 *   3. Clearing a W1C interrupt-flag the WRONG way loses other pending flags.
 */
#include <stdio.h>
#include <stdint.h>

/* ---- 1. register-style bit-field, one 32-bit word ---- */
typedef union {
    uint32_t raw;
    struct {
        uint32_t en   : 1;   /* bit 0  */
        uint32_t mode : 2;   /* bits 1..2 */
        uint32_t div  : 4;   /* bits 3..6 */
        uint32_t      : 1;   /* anonymous padding: skip bit 7 */
        uint32_t irq  : 8;   /* bits 8..15 */
        uint32_t rsv  : 16;  /* bits 16..31 */
    } b;
} REG_T;

/* ---- 2. flag packing to save RAM ---- */
typedef struct {
    uint8_t power_fail : 1;
    uint8_t over_temp  : 1;
    uint8_t comm_err   : 1;
    uint8_t cal_done   : 1;
    uint8_t            : 4;  /* anonymous: pad to 8 bits */
} FLAGS_T;                    /* sizeof == 1 */

typedef struct {              /* naive version, same info */
    uint8_t power_fail;
    uint8_t over_temp;
    uint8_t comm_err;
    uint8_t cal_done;
} FLAGS_NAIVE_T;              /* sizeof == 4 */

/* ---- 3. hardware IF register with W1C semantics ----
 * Hardware: writing 1 to a bit CLEARS that flag; writing 0 does nothing.
 * Our model: reg_write() clears every bit that is 1 in the value written.
 */
typedef struct {
    uint32_t raw;
} HW_IF_T;

typedef union {               /* overlay bit-field view on the IF register */
    uint32_t raw;
    struct {
        uint32_t f0  : 1;     /* flag 0, pending when 1 */
        uint32_t f1  : 1;     /* flag 1, pending when 1 */
        uint32_t     : 30;
    } b;
} IF_U;

static void reg_write(HW_IF_T *hw, uint32_t v)
{
    hw->raw &= ~v;            /* W1C: every 1 written clears that flag */
}

int main(void)
{
    /* --- 1. layout check --- */
    REG_T r;
    r.raw = 0;
    r.b.en   = 1;     /* bit0            -> 0x00000001 */
    r.b.mode = 3;     /* bits1..2 = 11   -> 0x00000006 */
    r.b.div  = 0xA;   /* bits3..6 = 1010 -> 0x00000050 */
    r.b.irq  = 0x55;  /* bits8..15       -> 0x00005500 */
    printf("1) layout:  raw = 0x%08X   (expect 0x00005557)\n", r.raw);
    printf("   sizeof(REG_T) = %u, first member sits on bit0\n",
           (unsigned)sizeof(r));

    /* --- 2. packing --- */
    printf("2) packing: sizeof(FLAGS_T)=%u  sizeof(FLAGS_NAIVE_T)=%u\n",
           (unsigned)sizeof(FLAGS_T), (unsigned)sizeof(FLAGS_NAIVE_T));

    FLAGS_T f;
    f.power_fail = 1;
    f.over_temp  = 0;
    f.comm_err   = 1;
    f.cal_done   = 1;
    printf("   read back: pf=%u ot=%u ce=%u cd=%u\n",
           f.power_fail, f.over_temp, f.comm_err, f.cal_done);

    /* --- 3. W1C clear-trap: IF starts with BOTH f0 and f1 pending --- */
    printf("3) W1C register, IF=0x03 (f0 and f1 both pending). Want: clear f0 only.\n");

    {
        HW_IF_T hw = { 0x03 };
        IF_U u;
        uint32_t v;

        /* WRONG way A: bit-field "clear to 0" style.
         * Compiler does: snap=read; snap&=~1; write back.
         * The write-back carries f1=1 -> W1C clears f1 too. */
        v = hw.raw; u.raw = v; u.b.f0 = 0; v = u.raw; reg_write(&hw, v);
        printf("   A bitfield f0=0   -> IF=0x%X  (f0 NOT cleared, f1 LOST!)\n",
               hw.raw);
    }
    {
        HW_IF_T hw = { 0x03 };
        IF_U u;
        uint32_t v;

        /* WRONG way B: classic |= mask. Same read-modify-write trap. */
        v = hw.raw; u.raw = v; u.b.f0 = 1; v = u.raw; reg_write(&hw, v);
        printf("   B mask     |=1    -> IF=0x%X  (f0 ok, f1 LOST!)\n", hw.raw);
    }
    {
        HW_IF_T hw = { 0x03 };
        /* RIGHT way: write ONLY the target flag directly (no RMW). */
        reg_write(&hw, 1u << 0);
        printf("   C direct  = (1<<0) -> IF=0x%X  (f0 cleared, f1 kept OK)\n",
               hw.raw);
    }

    return 0;
}
