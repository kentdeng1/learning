#include <stdio.h>
#include <stddef.h>

struct Foo {
    char a;
    int b;
    char c;
};

/* K 自己写的版本（传统 trick 写法） */
#define MY_OFFSETOF(type, member) \
    ((size_t)&(((type *)0)->member))

int main(void) {
    printf("=== 库版本 offsetof ===\n");
    printf("offsetof(Foo, a)  = %zu\n", offsetof(struct Foo, a));
    printf("offsetof(Foo, b)  = %zu\n", offsetof(struct Foo, b));
    printf("offsetof(Foo, c)  = %zu\n", offsetof(struct Foo, c));
    printf("\n=== K 自己写的 MY_OFFSETOF ===\n");
    printf("MY_OFFSETOF(Foo, a) = %zu\n", MY_OFFSETOF(struct Foo, a));
    printf("MY_OFFSETOF(Foo, b) = %zu\n", MY_OFFSETOF(struct Foo, b));
    printf("MY_OFFSETOF(Foo, c) = %zu\n", MY_OFFSETOF(struct Foo, c));
    return 0;
}