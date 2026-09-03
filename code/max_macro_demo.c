/*
 * max_macro_demo.c -- the classic MAX(x++, y++) double-evaluation trap
 *
 * Build & run (GNU dialect required for the statement-expression version):
 *   gcc -std=gnu11 -Wall -Wextra -o max_macro_demo max_macro_demo.c && ./max_macro_demo
 *
 * Demo of:
 *   1. Naive MAX: argument appearing twice in the text is evaluated twice.
 *   2. GCC statement-expression + typeof: each argument evaluated once.
 *   3. static inline function: standard C, type-safe, one evaluation.
 */
#include <stdio.h>

#define MAX_NAIVE(a,b) ((a)>(b)?(a):(b))

#define MAX_ONCE(a,b) ({                        \
    __typeof__(a) _a = (a);                     \
    __typeof__(b) _b = (b);                     \
    _a > _b ? _a : _b;                          \
})

static inline int imax(int a, int b)
{
    return a > b ? a : b;
}

int main(void)
{
    int m, x, y;

    /* [1] naive: losing arg incremented once, WINNING arg TWICE */
    x = 5; y = 8;
    m = MAX_NAIVE(x++, y++);
    printf("[1] naive MAX(x++,y++) -> m=%d x=%d y=%d\n", m, x, y);
    printf("    expected no-sides:  m=8 x=6 y=9  -> y was ++'ed twice!\n");

    /* [2] statement-expression: every argument evaluated exactly once */
    x = 5; y = 8;
    m = MAX_ONCE(x++, y++);
    printf("[2] MAX_ONCE(x++,y++)  -> m=%d x=%d y=%d   (each ++ once, OK)\n",
           m, x, y);

    /* [3] static inline: standard, same guarantee */
    x = 5; y = 8;
    m = imax(x++, y++);
    printf("[3] imax(x++,y++)      -> m=%d x=%d y=%d   (each ++ once, OK)\n",
           m, x, y);

    /* [4] and the hidden cost even WITHOUT side effects: a slow function
     *     argument is still called twice by the naive macro */
    return 0;
}
