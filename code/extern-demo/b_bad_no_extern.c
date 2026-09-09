#include <stdio.h>

/* ---- WRONG version: forgot extern on the variable ---- */

void foo(void);

int g_count;        /* BAD: without extern this is a SECOND definition,
                       not a declaration. With -fno-common (default since
                       GCC 10) the linker reports "multiple definition". */

int main(void)
{
    foo();
    printf("g_count = %d\n", g_count);
    return 0;
}
