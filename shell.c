#include "user.h"

void main(void) {

    /* page fault beacuse that address is not user mode accesible */
    // *((volatile int *)0x80200000) = 0x1234;

    // for (;;)
    //     ;

    printf("hello world from the shell\n");
}