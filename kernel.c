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

void kernel_main(void) {

    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

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