/* 宏练习 - K 的 5 题
 *
 * 编译: mingw32-make
 * 运行: ./macro.exe
 *
 * 要求: 实现下面 5 个宏，让 main() 输出符合预期
 */

#include <stdio.h>
#include <stddef.h>   // offsetof
#include <stdint.h>

/* ============================================
 * 第 1 题：基础 - MIN
 * 类似 MAX，但要避开多次求值
 *
 * 测试预期:
 *   MIN(3, 5)              -> 3
 *   MIN(2+1, 4)            -> 3
 *   MIN(x++, y++)          -> 不会让 x/y 多加
 * ============================================ */
#define MIN(a, b)    (a)<(b)?(a):(b)/* TODO: K 实现 */


/* ============================================
 * 第 2 题：位操作 - BIT(n)
 * 返回第 n 位是 1 的数。例如 BIT(3) = 0b1000 = 8
 *
 * 测试预期:
 *   BIT(0)                 -> 1
 *   BIT(3)                 -> 8
 *   BIT(31)                -> 0x80000000 (注意符号!)
 * ============================================ */
#define BIT(n)   return  1<<n/* TODO: K 实现 */


/* ============================================
 * 第 3 题：数组大小 - ARRAY_SIZE(arr)
 * 计算数组元素个数。**传指针会算错**（陷阱点）
 *
 * 测试预期:
 *   int arr[] = {1,2,3,4,5};
 *   ARRAY_SIZE(arr)        -> 5
 *   ARRAY_SIZE("hello")    -> 6 (含 '\0')
 * ============================================ */
#define ARRAY_SIZE(arr) return sizeof(arr)/sizeof((arr)[0]) /* TODO: K 实现 */


/* ============================================
 * 第 4 题：字符串化 - STRINGIFY(x)
 * 把宏参数变成字符串字面量
 *
 * 测试预期:
 *   STRINGIFY(123)         -> "123"
 *   STRINGIFY(MAX)         -> "MAX"
 *   STRINGIFY(3.14)        -> "3.14"
 *
 * 进阶：用 2 层宏展开（直接 #x 不能展开宏参数）
 * ============================================ */
#define STRINGIFY(x) /* TODO: K 实现 */


/* ============================================
 * 第 5 题：Linux 内核经典 - container_of
 * 给定结构体成员的指针，反推结构体本身的首地址
 *
 * 测试预期见 main() 里 struct Foo 的例子
 * ============================================ */
#define container_of(ptr, type, member) \
    /* TODO: K 实现 */


struct Foo {
    int a;
    int b;
    int c;
};


int main(void) {
    int x = 5, y = 10;

    /* ==== 测试 1: MIN ==== */
    printf("MIN(3, 5)        = %d (expect 3)\n", MIN(3, 5));
    printf("MIN(2+1, 4)      = %d (expect 3)\n", MIN(2+1, 4));
    int z = MIN(x++, y++);
    printf("MIN(x++,y++) z=%d, x=%d, y=%d (expect z=5, x=6, y=11)\n", z, x, y);
    //                     x 起始 5, y 起始 10
    //                     x++ 返回 5, y++ 返回 10 → 5<10 → 取 x++ 返回的 5
    //                     然后 x 自增到 6, y 自增到 11
    //                     注意：这里因为已经++ 过 x/y 了，x=6, y=11，z=5

    /* ==== 测试 2: BIT ==== */
    printf("BIT(0)           = %u (expect 1)\n", BIT(0));
    printf("BIT(3)           = %u (expect 8)\n", BIT(3));
    printf("BIT(31)          = 0x%X (expect 0x80000000, 注意符号位!)\n", BIT(31));

    /* ==== 测试 3: ARRAY_SIZE ==== */
    int arr[] = {1, 2, 3, 4, 5};
    printf("ARRAY_SIZE(arr)  = %zu (expect 5)\n", ARRAY_SIZE(arr));
    printf("ARRAY_SIZE(str)  = %zu (expect 6)\n", ARRAY_SIZE("hello"));

    /* ==== 测试 4: STRINGIFY ==== */
    printf("STRINGIFY(123)   = %s (expect 123)\n", STRINGIFY(123));
    printf("STRINGIFY(MAX)   = %s (expect MAX)\n", STRINGIFY(MAX));
    printf("STRINGIFY(3.14)  = %s (expect 3.14)\n", STRINGIFY(3.14));

    /* ==== 测试 5: container_of ==== */
    struct Foo foo = { .a = 10, .b = 20, .c = 30 };
    int *p_c = &foo.c;   // 已知 c 的指针
    struct Foo *p_foo = container_of(p_c, struct Foo, c);
    printf("container_of: foo.a=%d (expect 10)\n", p_foo->a);
    printf("container_of: foo.b=%d (expect 20)\n", p_foo->b);
    // 验证 p_foo 真的指向 foo
    printf("container_of: ptr match = %d (expect 1)\n", p_foo == &foo);

    return 0;
}