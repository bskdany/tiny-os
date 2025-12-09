#include "common.h"

/* don't like this but whatever */
extern void putchar(char);

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

                char out[9]; /* 32/4 = 8 + 1*/
                int i = 0;
                do {
                    out[i++] = hex_table[val & 0xf];
                    val = val >> 4;
                } while (val > 0);

                for (i -= 1; i >= 0; i--) {
                    putchar(out[i]);
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