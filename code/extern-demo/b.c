#include <stdio.h>

/* ---- using things defined in a.c ---- */

/* function: extern is OPTIONAL, a declaration without a body is extern by default */
void foo(void);

/* variable: extern is MANDATORY here */
extern int g_count;

int main(void)
{
    foo();
    printf("g_count seen from b.c = %d\n", g_count);
    return 0;
}
