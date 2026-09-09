#include <stdio.h>

/* ---- definitions in a.c (both have external linkage by default) ---- */

int g_count = 42;          /* global variable DEFINITION: storage allocated */

void foo(void)             /* function DEFINITION: has a body */
{
    printf("foo() in a.c, g_count=%d\n", g_count);
}
