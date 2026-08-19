/*
 * Chapter 6.18 编程练习
 *
 * 练习 1：find_char - 在 source 中找 chars 任意一个字符的首次出现
 * 练习 2：del_substr - 删除 str 中第一次出现的 substr
 *
 * 约束：
 *   - 不用任何库函数（strlen/strstr/strcpy/strcmp/strchr 全部自己写）
 *   - 不用下标，全部用指针（关键训练目标）
 */

#include <stdio.h>

/* ============================================================
 *  自己写的 strlen / strchr / strstr / strcpy（工具函数）
 *  上一节练习已经会了，这里直接复用
 * ============================================================ */

static size_t my_strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

/* 自己的简易 strchr（备用，本节练习不直接用，但留作工具） */
#if 0
static char *my_strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char *)s;
        s++;
    }
    return NULL;
}
#endif

/* 自己的简易 strstr，只用于辅助 del_substr */
static char *my_strstr(const char *haystack, const char *needle) {
    if (*needle == '\0') return (char *)haystack;  /* 空串 = 在头 */

    for (const char *p = haystack; *p; p++) {
        const char *h = p;
        const char *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (*n == '\0') return (char *)p;  /* 全部匹配 */
    }
    return NULL;
}

/* ============================================================
 *  练习 1：find_char
 *  在 source 中查找 chars 中任意字符的首次出现
 *  返回指向匹配位置的指针；找不到返回 NULL
 *  任一参数为 NULL 或指向空串，返回 NULL
 * ============================================================ */
char *find_char(char const *source, char const *chars) {
    /* 参数防御 */
    if (source == NULL || chars == NULL) return NULL;
    if (*source == '\0' || *chars == '\0') return NULL;

    /* 外层：遍历 source 的每个位置 p
     * 内层：检查 chars 中是否有字符 == *p */
    for (const char *p = source; *p != '\0'; p++) {
        for (const char *q = chars; *q != '\0'; q++) {
            if (*p == *q) {
                return (char *)p;   /* 找到！返回 source 中的位置 */
            }
        }
    }

    return NULL;   /* 全 source 走完都没匹配 */
}

/* ============================================================
 *  练习 2：del_substr
 *  删除 str 中第一次出现的 substr
 *  返回值：
 *    1 - 成功删除（str 被修改）
 *    0 - substr 不在 str 中（或边界情况）
 * ============================================================ */
int del_substr(char *str, char const *substr) {
    /* 参数防御 */
    if (str == NULL || substr == NULL) return 0;

    /* 找 substr 在 str 中的位置 */
    char *pos = my_strstr(str, substr);
    if (pos == NULL) return 0;   /* 不在 */

    /* 关键操作：
     *   把 pos + len(substr) 开始的所有字节（包含 '\0'）
     *   复制到 pos 位置
     *   → substr 被覆盖掉，str 缩短
     */
    size_t len_sub = my_strlen(substr);
    char *src = pos + len_sub;     /* 读指针：substr 后面的内容 */
    char *dst = pos;               /* 写指针：从 substr 位置开始覆盖 */

    while (1) {
        *dst = *src;
        if (*src == '\0') break;   /* 复制到 '\0' 为止 */
        dst++;
        src++;
    }

    return 1;
}

/* ============================================================
 *  测试
 * ============================================================ */

static void test_find_char(void) {
    printf("=== Exercise 1: find_char ===\n");

    /* 验证方式：
     *   - expected=NULL：检查返回 NULL
     *   - expected 是单字符：检查 *got == expected（指针位置和 source 中匹配字符的偏移）
     *   - expected 是字符串：从 source + offset 算，指针值要等于 source + offset
     */
    struct {
        const char *source;
        const char *chars;
        int   expected_offset;   /* 在 source 中的偏移；-1 表示 NULL */
        const char *desc;
    } cases[] = {
        {"ABCDEF",   "XYZ",     -1, "no match"},
        {"ABCDEF",   "JURY",    -1, "no match"},
        {"ABCDEF",   "QQQQ",    -1, "no match"},
        {"ABCDEF",   "XRCQEF",   2, "match at C (offset 2)"},
        {"Hello, K!", "K",       7, "match at K (offset 7)"},
        {"Hello, K!", "!,",      5, "first match is ',' at offset 5"},
        {"",         "abc",     -1, "empty source"},
        {"abc",      "",        -1, "empty chars"},
        {NULL,       "abc",     -1, "NULL source"},
        {"abc",      NULL,      -1, "NULL chars"},
    };
    size_t n = sizeof(cases) / sizeof(cases[0]);

    for (size_t i = 0; i < n; i++) {
        char *got = find_char(cases[i].source, cases[i].chars);
        int  offset = (got == NULL) ? -1 : (int)(got - cases[i].source);
        int  pass   = (offset == cases[i].expected_offset);
        printf("  [%s] case %2zu: %-35s | offset=%d (exp %d) char='%c'\n",
               pass ? "PASS" : "FAIL", i, cases[i].desc,
               offset, cases[i].expected_offset,
               (got != NULL && *got != '\0') ? *got : ' ');
    }
}

static void test_del_substr(void) {
    printf("\n=== Exercise 2: del_substr ===\n");

    /* 用临时缓冲区测试，因为函数会修改 str */
    char buf[64];

    struct {
        const char *input;
        const char *substr;
        const char *expected;
        int         expected_ret;
        const char *desc;
    } cases[] = {
        {"ABCDEFG",   "FGH", "ABCDEFG",   0, "substr not in str"},
        {"ABCDEFG",   "CDF", "ABCDEFG",   0, "substr not in str"},
        {"ABCDEFG",   "XABC","ABCDEFG",   0, "substr not in str"},
        {"ABCDEFG",   "CDE", "ABFG",      1, "del middle"},
        {"ABCDEFG",   "FG",  "ABCDE",     1, "del tail"},
        {"ABCDEFG",   "AB",  "CDEFG",     1, "del head"},
        {"AAA",       "A",   "AA",        1, "del one of many"},
        {"ABCABC",    "ABC", "ABC",       1, "del first occurrence only"},
        {"ABC",       "",    "ABC",       1, "empty substr (no-op)"},
        {"",          "ABC", "",          0, "empty str"},
    };
    size_t n = sizeof(cases) / sizeof(cases[0]);

    for (size_t i = 0; i < n; i++) {
        /* 复制到 buf（因为函数会修改） */
        size_t j = 0;
        while (cases[i].input[j] && j < sizeof(buf) - 1) {
            buf[j] = cases[i].input[j];
            j++;
        }
        buf[j] = '\0';

        int ret = del_substr(buf, cases[i].substr);
        const char *ok = (ret == cases[i].expected_ret &&
                          my_strlen(buf) == my_strlen(cases[i].expected)) ? "PASS" : "FAIL";
        printf("  [%s] case %2zu: %-30s | ret=%d (exp %d) str=\"%s\" (exp \"%s\")\n",
               ok, i, cases[i].desc,
               ret, cases[i].expected_ret,
               buf, cases[i].expected);
    }
}

int main(void) {
    test_find_char();
    test_del_substr();
    return 0;
}