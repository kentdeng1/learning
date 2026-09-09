/*
 * funcptr_typematch_demo.c -- function pointer type must match EXACTLY
 *
 * Build & run (expect zero warnings):
 *   gcc -std=c11 -Wall -Wextra -o funcptr_typematch funcptr_typematch_demo.c
 *   ./funcptr_typematch
 *
 * The buggy original (kept for reference, do NOT enable):
 *   typedef int (*g_cb_t)(unsigned int *data);   <-- expects a POINTER
 *   int add_p(unsigned int data) { ... }         <-- takes a VALUE
 *   g_cb = add_p;   // warning: incompatible pointer type
 *   g_val = g_cb(5);// warning: makes pointer from integer
 * It "works" (prints 15) only because both sides read the same integer
 * register. It is undefined behaviour: change the body to *data + 10 and
 * it tries to read address 0x5 -> HardFault on STM32, segfault on PC.
 */
#include <stdio.h>

/* =========================================================
 * Version A: make the typedef match a value parameter
 * ========================================================= */
typedef int (*cb_val_t)(unsigned int data);

static int add_val(unsigned int data)
{
    return data + 10;
}

static void demo_version_a(void)
{
    cb_val_t cb = NULL;
    unsigned int val;

    cb = add_val;            /* types match exactly, no cast, no warning */
    val = cb(5);

    printf("A) value parameter:  cb(5) = %u\n", val);
}

/* =========================================================
 * Version B: make the function match a pointer parameter
 * ========================================================= */
typedef int (*cb_ptr_t)(unsigned int *data);

static int add_ptr(unsigned int *data)
{
    return *data + 10;       /* real pointer: dereference it */
}

static void demo_version_b(void)
{
    cb_ptr_t cb = NULL;
    unsigned int x = 5;
    unsigned int val;

    cb = add_ptr;            /* types match exactly */
    val = cb(&x);            /* pass an ADDRESS, not a number */

    printf("B) pointer parameter: cb(&x) = %u  (x is still %u)\n", val, x);
}

/* =========================================================
 * What the buggy version would do if it dereferenced
 * (shown with a harmless print, NOT executed)
 * ========================================================= */
static void show_hazard(void)
{
    unsigned int *p = (unsigned int *)5;   /* the '5' from g_cb(5) */
    printf("C) hazard: as a pointer, 5 becomes address %p -> deref = crash\n",
           (void *)p);
}

int main(void)
{
    printf("rule: return type + every parameter type must match exactly\n");
    printf("      unsigned int  !=  unsigned int *\n\n");

    demo_version_a();
    demo_version_b();
    show_hazard();

    return 0;
}
