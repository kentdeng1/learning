#include <stdio.h>

/* 值传递：交换的是副本，调用方的 x/y 不变 */
void swap_by_value(int a, int b) {
    int t = a; a = b; b = t;
}

/* 传指针：解引用后交换，调用方可见 */
void swap_by_ptr(int *a, int *b) {
    int t = *a; *a = *b; *b = t;
}

int main(void) {
    int x = 1, y = 2;
    printf("before:           x=%d y=%d\n", x, y);
    swap_by_value(x, y);
    printf("swap_by_value 后: x=%d y=%d  (没变)\n", x, y);
    swap_by_ptr(&x, &y);
    printf("swap_by_ptr 后:    x=%d y=%d  (交换成功)\n", x, y);
    return 0;
}
