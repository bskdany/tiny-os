#include "kernel.h"
#include "common.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

/* get the addresses declared in the kernel linker script, [] is used to avoid
 * getting the value */
extern char __bss[], __bss_end[], __stack_top[];

void *memset(void *buf, char c, size_t n) {
    uint8_t *p = (uint8_t *)buf;
    for (size_t i = 0; i < n; i++) {
        *p = c;
    }
    /* this is returned for chaining */
    return buf;
}

/*
    On RISC-V ISA the CPU can have the following privilege modes
    - U -> user mode (lower privilege)
    - S -> supervisor mode (kernel)
    - M -> machine mode (low level cpu control)

    The M level is an area where direct CPU configiration and operations are performed. I'm using OpenSBI to control
   that environment, a RISC-V specific "bios" that also acts as a HAL for different CPUs integrating the ISA. SBI stands
   for Supervisor Binary Interface and it executes operations in the SEE (Supervisor Execution Environment).
   Communication to the SEE is done through the SBI, the SBI declares an interface over asm called ecal (environment
   call)

    a0 -> error code
    a1 -> return value
    a2-a6 -> arguments
    a7 -> SBI extension id (timer/interrupt/etc)
    a6 -> SBI function id (specific function in the extension)
*/
struct sbi_ret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long fid, long eid) {
    /* I'm not sure why we use long here, I thought this would be a 32bit system */
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    /*
       mode this call changes the values of the external memory, the memory values held in registers need to be
       invalidated and re-fetche, the "memory" keyword is an instruction to the compiler to do so
    */
    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbi_ret){.err = a0, .val = a1};
}

void putchar(char ch) { sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */); }

void kernel_main(void) {
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

    printf("\nHello %s\n", "World!");
    printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);
    int x = 123456;
    printf("123456 to hex = %x\n", x);
    for (;;)
        ;
}

/* the attributes set the function address to what we declared in the linker script and tell the compiler to avoid
 * generating unnecessary code before and after the function body to the function body nothing more than the inline
 * assembly */
__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void) {
    /*
        __asm__ compiler extension to allow inline assembly
        move the stack pointer to the stack top, then jump to the kernel main
       function
    */
    __asm__ __volatile__("mv sp, %[stack_top]\n"
                         "j kernel_main\n"
                         :
                         : [stack_top] "r"(__stack_top));
}