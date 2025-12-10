#include "common.h"

/* don't like this but whatever */
extern void putchar(char);

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dst;
}

void *memset(void *buf, char c, size_t n) {
    uint8_t *p = (uint8_t *)buf;
    for (size_t i = 0; i < n; i++) {
        *p = c;
    }
    /* this is returned for chaining */
    return buf;
}

char *strcpy(char *dst, const char *src) {
    char *d = dst;

    while (*src) {
        *d = *src;
        src++;
        d++;
    }

    *dst = '\0';

    return dst;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) {
            break;
        }
        s1++;
        s2++;
    }

    return *s1 - *s2; /* fuck the posix spec */
}

void printf(const char *fmt, ...) {
    va_list v_args;
    va_start(v_args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case '\0': {
                putchar('\0');
                fmt--; /* edge case */
                break;
            }
            case '%': {
                putchar('%');
                break;
            }

            case 's': {
                const char *s = va_arg(v_args, const char *);
                while (*s) {
                    putchar(*s);
                    s++;
                }
                break;
            }

            case 'd': {
                int val = va_arg(v_args, int);
                /* binary to decimal conversion */

                char out[11]; /* 32bit signed int holds 10 chars max + 1 char for null term */
                int abs_val;
                if (val >= 0) {
                    abs_val = val;
                } else {
                    abs_val = -val;
                    putchar('-');
                }

                int i = 0;
                do {
                    out[i++] = '0' + abs_val % 10; /* digit to char */
                    abs_val /= 10;
                } while (abs_val > 0);

                for (i -= 1; i >= 0; i--) {
                    putchar(out[i]);
                }
                break;
            }
            case 'x': {
                /* binary to hex conversion */
                const char *hex_table = "0123456789abcdef";
                int val = va_arg(v_args, int);
                for (int i = 7; i >= 0; i--) {
                    int blob = (val >> (4 * i)) & 0xf;
                    putchar(hex_table[blob]);
                }
                break;
            }
            }
        } else {
            putchar(*fmt);
        }

        fmt++;
    }
}