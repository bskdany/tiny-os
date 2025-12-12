#include "user.h"

void main(void) {

    /* page fault beacuse that address is not user mode accesible */
    // *((volatile int *)0x80200000) = 0x1234;

    // printf("hello world from the shell\n");

    while (1) {

    prompt:
        printf("> ");

        char cmdline[128];

        for (int i = 0;; i++) {
            char ch = getchar();
            putchar(ch);

            if (i == sizeof(cmdline) - 1) {
                printf("command line too long\n");
                goto prompt;
            } else if (ch == '\r') {
                printf("\n");
                cmdline[i] = '\0';
                break;
            } else {
                cmdline[i] = ch;
            }
        }

        if (strcmp(cmdline, "hello") == 0) {
            printf("hello to you\n");
        } else {
            printf("unknown command %s\n", cmdline);
        }
    }
}