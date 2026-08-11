/*
 * strlen 详解 + 3 个扩展版本
 * 对应《C 和指针》第 6.12 节
 *
 * 编译: mingw32-make
 * 运行: ./string_search.exe
 */

#include <stdio.h>
#include <stddef.h>

/* ============================================
 * 1. 标准 strlen（书上原版，K 截图里的）
 * 模式: while (*string++) 计数
 * ============================================ */
size_t my_strlen(const char *string) {
    int length = 0;

    /* 关键: *string++ 不等于 '\0' 时计数
       拆解: (1) *string 取当前字符 (旧 cp)
             (2) string 自增到下一位
             (3) 判断当前字符是不是 '\0' */
    while (*string++ != '\0') {
        length += 1;
    }
    return (size_t)length;
}

/* 优化版：用指针差，O(1) 减法不用计数器 */
size_t my_strlen_v2(const char *s) {
    const char *p = s;
    while (*p) p++;          /* p 一直走到 '\0' */
    return (size_t)(p - s);  /* 两个指针的距离 = 长度 */
}

/* ============================================
 * 2. 查找字符：strchr
 * 返回指向字符的指针，找不到返回 NULL
 * ============================================ */
char *my_strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char *)s;  /* 找到, 返回当前位置 */
        }
        s++;
    }
    /* 这里要单独检查 c == '\0'，因为 while (*s) 跳过了 */
    if ((char)c == '\0') return (char *)s;
    return NULL;
}

/* ============================================
 * 3. 查找子串：strstr（K 想要的"在字符串中查找变量"）
 * 返回首次匹配位置的指针，找不到 NULL
 * ============================================ */
char *my_strstr(const char *haystack, const char *needle) {
    /* 空 needle 永远匹配开头 */
    if (*needle == '\0') return (char *)haystack;

    /* 遍历 haystack 每一个字符作为起点 */
    for (const char *p = haystack; *p; p++) {
        const char *h = p;       /* haystack 当前试探位置 */
        const char *n = needle;   /* needle 当前位置 */

        /* 从 p 开始, 逐字符比较 */
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }

        /* 如果 needle 全部匹配完 (*n == '\0'), 找到了 */
        if (*n == '\0') {
            return (char *)p;
        }
    }
    return NULL;
}

/* ============================================
 * 4. 简化版：是否包含子串（返回 1/0）
 * ============================================ */
int contains(const char *haystack, const char *needle) {
    return my_strstr(haystack, needle) != NULL;
}

/* ============================================
 * 5. 嵌入式实战：UART 指令解析
 * ============================================ */
int parse_command(const char *uart_rx, const char *cmd) {
    /* 收到 "AT+SET_TEMP=25\r\n" 这种, 判断是不是某指令 */
    return contains(uart_rx, cmd);
}

int main(void) {
    const char *s = "Hello, K! Today is 2026-08-11.";

    /* ==== 1. strlen ==== */
    printf("=== 1. strlen ===\n");
    printf("strlen(\"%s\") = %zu\n", s, my_strlen(s));
    printf("strlen_v2       = %zu\n", my_strlen_v2(s));
    printf("strlen(\"\")     = %zu\n", my_strlen(""));
    printf("strlen(\"abc\")  = %zu\n", my_strlen("abc"));
    printf("\n");

    /* ==== 2. strchr 找字符 ==== */
    printf("=== 2. strchr (find char) ===\n");
    char *p = my_strchr(s, 'K');
    if (p) {
        printf("Found 'K' at pos %td, rest: \"%s\"\n", p - s, p);
    }

    p = my_strchr(s, 'Z');
    printf("Found 'Z': %s\n", p ? "yes" : "no");

    p = my_strchr(s, '\0');
    printf("Found '\\0' at end: %s (pos %td)\n", p ? "yes" : "no", p ? p - s : -1);
    printf("\n");

    /* ==== 3. strstr 找子串 ==== */
    printf("=== 3. strstr (find substring) ===\n");
    p = my_strstr(s, "Today");
    if (p) {
        printf("Found \"Today\" at pos %td: \"%s\"\n", p - s, p);
    }

    p = my_strstr(s, "2026");
    if (p) {
        printf("Found \"2026\" at pos %td: \"%s\"\n", p - s, p);
    }

    p = my_strstr(s, "9999");
    printf("Found \"9999\": %s\n", p ? "yes" : "no");

    /* K 重点要看的：变量查找 */
    const char *today = "Today";
    p = my_strstr(s, today);
    printf("\nK 想要的\"在字符串中查找变量\":\n");
    printf("  const char *today = \"Today\";\n");
    printf("  my_strstr(s, today) = \"%s\"\n", p);
    printf("\n");

    /* ==== 4. contains 简化版 ==== */
    printf("=== 4. contains ===\n");
    printf("contains \"2026\": %d\n", contains(s, "2026"));
    printf("contains \"K!\":   %d\n", contains(s, "K!"));
    printf("contains \"xyz\":  %d\n", contains(s, "xyz"));
    printf("\n");

    /* ==== 5. 嵌入式实战：UART 指令解析 ==== */
    printf("=== 5. Embedded: UART command parsing ===\n");
    const char *commands[] = {
        "AT+VER?",
        "AT+RESET",
        "AT+SET_TEMP=25",
        "AT+GET_STATUS",
        "AT+SET_TEMP=30",
        "ERROR: timeout"
    };
    const char *target = "SET_TEMP";
    printf("Looking for \"%s\" in 6 commands:\n", target);
    for (int i = 0; i < 6; i++) {
        if (contains(commands[i], target)) {
            printf("  [%d] %s -> matched (pos %td)\n", i, commands[i], my_strstr(commands[i], target) - commands[i]);
        }
    }

    /* 找具体的温度值 */
    printf("\nFind temperature value \"25\":\n");
    p = my_strstr(commands[2], "25");
    if (p) {
        printf("  Found at pos %td: \"%s\"\n", p - commands[2], p);
    }

    return 0;
}