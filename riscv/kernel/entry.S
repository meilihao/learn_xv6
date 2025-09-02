        # qemu -kernel loads the kernel at 0x80000000
        # and causes each hart (i.e. CPU) to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
.global _entry
_entry:
        # 为每个 CPU 核心（hart）设置一个独立的 C 语言栈，然后跳转到 start() 函数去执行更高级的初始化工作
        # core栈顶地址 = stack0基址 + (hartid + 1) * 栈大小4096, 同时**栈向下生长**, 这也是下面`addi a1, a1, 1`的原因
        # set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + ((hartid + 1) * 4096)
        la sp, stack0 # 将 stack0 的地址加载到 sp 寄存器
        li a0, 1024*4 # 将栈的大小（4096 字节，即 1024 * 4）加载到临时寄存器 a0
        csrr a1, mhartid # csrr 指令从 CSR (Control and Status Register) 中读取当前 CPU 核心的 ID，即 hartid，并将其存入 a1 寄存器
        addi a1, a1, 1 # 将 a1 中的 hartid 加 1. 这是为了确保即使 hartid 为 0，也能得到一个非零的偏移量, 防止栈顶地址为 stack0 本身，从而为栈生长留出足够的空间
        mul a0, a0, a1 # 将每个栈的大小（a0，4096）与修正后的 hartid（a1）相乘。这个结果就是当前核心的栈在 stack0 数组中的偏移量
        add sp, sp, a0 # 将 stack0 的基地址（sp）与计算出的偏移量（a0）相加，最终得到当前 CPU 核心的栈顶地址，并将其存入 sp 寄存器
        # jump to start() in start.c
        call start
spin:
        j spin
