#include "kernel.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

/* get the addresses declared in the kernel linker script, [] is used to avoid
 * getting the value */
extern char __bss[], __bss_end[], __stack_top[], __free_ram_start[], __free_ram_end[];

struct process procs[PROCS_MAX];

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

void handle_trap(struct trap_frame *f) {
    /* scause - cause of exception  */
    uint32_t scause = READ_CSR(scause);
    /* stval - additional information (mem addr that caused the exception )*/
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
}

/* init core kernel functions */
__attribute__((naked)) __attribute__((aligned(4))) void kernel_entry(void) {

    /* saves the state of the registers on the stack but keeps stack poiter where it is, calls exeption handler,
     * restores registers back */
    __asm__ __volatile__("csrw sscratch, sp\n"
                         "addi sp, sp, -4 * 31\n"
                         "sw ra,  4 * 0(sp)\n"
                         "sw gp,  4 * 1(sp)\n"
                         "sw tp,  4 * 2(sp)\n"
                         "sw t0,  4 * 3(sp)\n"
                         "sw t1,  4 * 4(sp)\n"
                         "sw t2,  4 * 5(sp)\n"
                         "sw t3,  4 * 6(sp)\n"
                         "sw t4,  4 * 7(sp)\n"
                         "sw t5,  4 * 8(sp)\n"
                         "sw t6,  4 * 9(sp)\n"
                         "sw a0,  4 * 10(sp)\n"
                         "sw a1,  4 * 11(sp)\n"
                         "sw a2,  4 * 12(sp)\n"
                         "sw a3,  4 * 13(sp)\n"
                         "sw a4,  4 * 14(sp)\n"
                         "sw a5,  4 * 15(sp)\n"
                         "sw a6,  4 * 16(sp)\n"
                         "sw a7,  4 * 17(sp)\n"
                         "sw s0,  4 * 18(sp)\n"
                         "sw s1,  4 * 19(sp)\n"
                         "sw s2,  4 * 20(sp)\n"
                         "sw s3,  4 * 21(sp)\n"
                         "sw s4,  4 * 22(sp)\n"
                         "sw s5,  4 * 23(sp)\n"
                         "sw s6,  4 * 24(sp)\n"
                         "sw s7,  4 * 25(sp)\n"
                         "sw s8,  4 * 26(sp)\n"
                         "sw s9,  4 * 27(sp)\n"
                         "sw s10, 4 * 28(sp)\n"
                         "sw s11, 4 * 29(sp)\n"

                         "csrr a0, sscratch\n"
                         "sw a0, 4 * 30(sp)\n"

                         "mv a0, sp\n"
                         "call handle_trap\n"

                         "lw ra,  4 * 0(sp)\n"
                         "lw gp,  4 * 1(sp)\n"
                         "lw tp,  4 * 2(sp)\n"
                         "lw t0,  4 * 3(sp)\n"
                         "lw t1,  4 * 4(sp)\n"
                         "lw t2,  4 * 5(sp)\n"
                         "lw t3,  4 * 6(sp)\n"
                         "lw t4,  4 * 7(sp)\n"
                         "lw t5,  4 * 8(sp)\n"
                         "lw t6,  4 * 9(sp)\n"
                         "lw a0,  4 * 10(sp)\n"
                         "lw a1,  4 * 11(sp)\n"
                         "lw a2,  4 * 12(sp)\n"
                         "lw a3,  4 * 13(sp)\n"
                         "lw a4,  4 * 14(sp)\n"
                         "lw a5,  4 * 15(sp)\n"
                         "lw a6,  4 * 16(sp)\n"
                         "lw a7,  4 * 17(sp)\n"
                         "lw s0,  4 * 18(sp)\n"
                         "lw s1,  4 * 19(sp)\n"
                         "lw s2,  4 * 20(sp)\n"
                         "lw s3,  4 * 21(sp)\n"
                         "lw s4,  4 * 22(sp)\n"
                         "lw s5,  4 * 23(sp)\n"
                         "lw s6,  4 * 24(sp)\n"
                         "lw s7,  4 * 25(sp)\n"
                         "lw s8,  4 * 26(sp)\n"
                         "lw s9,  4 * 27(sp)\n"
                         "lw s10, 4 * 28(sp)\n"
                         "lw s11, 4 * 29(sp)\n"
                         "lw sp,  4 * 30(sp)\n"
                         "sret\n");
}

// /* linear allocator, mem can't be freed */
paddr_t alloc_pages(uint32_t n) {
    static paddr_t next_paddr = (paddr_t)__free_ram_start;
    paddr_t paddr = next_paddr;
    next_paddr += n * PAGE_SIZE;

    if (next_paddr > (paddr_t)__free_ram_end) PANIC("out of memory");

    memset((void *)paddr, 0, n * PAGE_SIZE);
    return paddr;
}

__attribute__((naked)) void switch_context(uint32_t *prev_sp, uint32_t *next_sp) {
    __asm__ __volatile__("addi sp, sp, -13 * 4\n"
                         "sw ra,  0  * 4(sp)\n" /* store word from ra into sp at offset */
                         "sw s0,  1  * 4(sp)\n"
                         "sw s1,  2  * 4(sp)\n"
                         "sw s2,  3  * 4(sp)\n"
                         "sw s3,  4  * 4(sp)\n"
                         "sw s4,  5  * 4(sp)\n"
                         "sw s5,  6  * 4(sp)\n"
                         "sw s6,  7  * 4(sp)\n"
                         "sw s7,  8  * 4(sp)\n"
                         "sw s8,  9  * 4(sp)\n"
                         "sw s9,  10 * 4(sp)\n"
                         "sw s10, 11 * 4(sp)\n"
                         "sw s11, 12 * 4(sp)\n"

                         /* stack pointers are swapped*/
                         "sw sp, (a0)\n"
                         "lw sp, (a1)\n"

                         "lw ra,  0  * 4(sp)\n" /* load word into register ra from sp at offset */
                         "lw s0,  1  * 4(sp)\n"
                         "lw s1,  2  * 4(sp)\n"
                         "lw s2,  3  * 4(sp)\n"
                         "lw s3,  4  * 4(sp)\n"
                         "lw s4,  5  * 4(sp)\n"
                         "lw s5,  6  * 4(sp)\n"
                         "lw s6,  7  * 4(sp)\n"
                         "lw s7,  8  * 4(sp)\n"
                         "lw s8,  9  * 4(sp)\n"
                         "lw s9,  10 * 4(sp)\n"
                         "lw s10, 11 * 4(sp)\n"
                         "lw s11, 12 * 4(sp)\n"
                         "addi sp, sp, 13 * 4\n"
                         "ret\n"

    );
}

struct process *create_proces(uint32_t proc_entrypoint) {

    struct process *proc = NULL;

    int i = 0;
    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].state == PROC_UNUSED) {
            proc = &procs[i];
            break;
        }
    }

    if (!proc) {
        PANIC("no leftover processes");
    }

    uint32_t *sp = (uint32_t *)&(proc->stack[sizeof(proc->stack)]);
    for (int i = 0; i < 12; i++) {
        *--sp = 0; /* zero */
    }
    *--sp = (uint32_t)proc_entrypoint; /* return address set to the proc entrypoint */

    /* init proc struct */
    proc->pid = i + 1;
    proc->state = PROC_RUNNABLE;
    proc->sp = (vaddr_t)sp;
    return proc;
}

void delay(void) {
    for (int i = 0; i < 300000000; i++)
        __asm__ __volatile__("nop"); // do nothing
}

struct process *proc_a;
struct process *proc_b;

void proc_a_entry(void) {
    printf("Process A started\n");
    while (1) {
        putchar('A');
        switch_context(&proc_a->sp, &proc_b->sp);
        delay();
    }
}

void proc_b_entry(void) {
    printf("Process B started\n");
    while (1) {
        putchar('B');
        switch_context(&proc_b->sp, &proc_a->sp);
        delay();
    }
}

void kernel_main(void) {
    memset(__bss, 0, (size_t)__bss_end - (size_t)__bss);

    /* stvec - supervisor trap vector base address register, holds the address of the kernel trap handler function */
    WRITE_CSR(stvec, (uint32_t)kernel_entry);

    proc_a = create_proces((uint32_t)proc_a_entry);
    proc_b = create_proces((uint32_t)proc_b_entry);
    proc_a_entry();

    PANIC("Unreachable state");
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