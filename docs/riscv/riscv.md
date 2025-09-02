# riscv

## 模式
RISC-V总共有三种模式：
- M-mode （Machine Mode）

    最高特权级，负责处理所有硬件异常和中断的最终控制权, 可访问全部的地址
- S-mode （Supervisor Mode）

    通常由操作系统内核（如 xv6-riscv）运行，处理操作系统级别的异常
- U-mode （User mode）

在系统加电之后，会处于M-mode.

RV32 和RV64 的区别在于指令集所支持的位数不同，RV32 是32 位指令集，而RV64 是64 位指令集。它们都属于 RISC-V 指令集架构，基础指令集分别为 RV32I 和 RV64I. xv6属于RV64.

## 寄存器
每个寄存器也由一个16进制的别名, 比如`0x30a=menvcfg`

- mcounteren: 用于控制在 M-mode 下，哪些计数器可以被 S-mode 访问

    - 2：代表 time（机器模式时间）计数器

- medeleg：Machine Exception Delegation Register

    用于指定被委托给更低特权级别的异常，当异常发生时，如果该异常被委托，处理器会将该异常转交给更低特权级别的异常处理程序进行处理。 如果某个位被设置为 1，表示被委托

    RISC-V 架构允许将中断和异常处理从 M-mode 委托（delegate） 给 S-mode. xv6运行在 S-mode，因此需要通过配置 w_medeleg 将相应异常委托给 S-mode 处理，以便内核能够响应和处理这些异常（如系统调用、页故障等）.

    RISC-V 设计了灵活的权限委托机制, 这种机制的目的是简化操作系统（运行在 S 模式）的实现，让其能更自主地管理中断和定时器，而无需频繁陷入 M 模式.
    
    xv6 将中断和异常处理的责任从高特权级的 M-mode 转移到了 S-mode，这是现代操作系统设计的常见做法.
- menvcfg: Machine environment configuration register

    menvcfg 的第 63 位（或 menvcfgh 的第 31 位）名为 STCE（STimecmp Enable），设置为 1 时，用于启用 S 模式下的 stimecmp
- mepc: Machine Exception Program Counter, 机器模式先前的特权模式

    mret 指令在返回时，会从 mepc 寄存器中获取下一条要执行的指令地址

    常见执行操作如下:
    1. 发生异常（中断、故障等）。
    1. 处理器将当前的PC值（即异常发生前正在执行的指令地址）写入 mepc 寄存器
    1. 进入内核处理异常
    1. 异常处理完毕后，返回到 mepc 保存的地址，继续执行
- mideleg：Machine Interrupt Delegation Register：和medeleg类似，用于委托中断
- mie: 机器模式中断使能寄存器. 类似sie, 记录处理器目前能处理和必须忽略的中断，即相应的中断使能位
- mhartid : Machine Hart ID Register

    一个控制和状态寄存器（CSR），用于存储当前硬件线程（hart）的标识符, mhartid从0开始

    在多核处理器中，每个核心可能有一个或多个硬件线程，每个硬件线程都有一个唯一的 hartid.

    在多核处理器中，每个核心（或硬件线程）都有自己独立的一套寄存器集。这意味着如果有两个核心，每个核心有 32 个通用寄存器，那么整个处理器实际上会有 64 个通用寄存器。每个核心的寄存器集是独立的，互不干扰.
- mstatus:  Machine Status

    mstatus 寄存器包含了许多重要的处理器状态位，如特权模式、中断使能等

    - MPP (Machine Previous Privilege, 机器模式先前的特权模式): 指示在发生异常或中断之前的特权级别，当使用mret返回时，处理器会切换回原来的级别，在第11 12位
        - 00: User Mode
        - 01: Supervisor Mode
        - 11: Machine Mode
    - MPIE (Machine Previous Interrupt Enable): 记录在发生异常或中断之前，中断是否被启用。
    - MIE (Machine Interrupt Enable): 允许或禁止中断。当为1时，中断被启用。
    - MIE (Machine Interrupt Enable): 记录中断是否被启用
- PMP 控制寄存器/PMP 地址寄存器

    在 RISC-V 特权指令集手册 的第 3.7 节中介绍了内存保护机制, 是 RISC-V 的一种硬件机制，用于在机器模式（M-mode）或监管者模式（S-mode）下定义物理内存区域的访问权限（读、写、执行）. 每一个 PMP(Physical Memory Protection) 项定义了一个访问控制区域. 一个 PMP 项包括一个 8 位控制寄存器和一个地址寄存器，处理器支持最多 64 个 PMP 项。PMP 访问控制区域的大小是可设置的，最小可支持 4 字节大小的区域.

    - mpaddr0-pmpaddrN：地址寄存器（设置内存区域的结束地址或地址范围）
    - pmpcfg0-pmpcfgN：配置寄存器（定义访问权限和模式）

        模式
        1. NAPOT（Naturally Aligned Power-of-Two, 任意幂次大小的对齐区域）:
            1. pmpaddrN表示一个内存区域的大小和基地址基地址为 0（因为 NAPOT 要求自然对齐，且该值对应地址 0）
        1. TOR（Top of Range，上边界）
        1. NA4（4 字节对齐的精确匹配）

    ```c
    w_pmpaddr0(0x3fffffffffffffull); // 0x3FFFFFFFFFFFFFULL = 50位全 1, 它是一个特殊的 NAPOT 模式值，它表示一个极大的连续内存区域（几乎覆盖整个64位地址空间）
    w_pmpcfg0(0xf) // 0xF = 1111b = R=1, W=1, X=1, A1:A0=1 (NAPOT（地址范围压缩表示方式）模式)
    ```

- satp : Supervisor Address Translation and Protection Register

    监督地址转换和保护寄存器，用于控制和配置页表的地址. 它定义了当前页表的位置和格式，以及一些与地址翻译和保护相关的信息
- sie:Supervisor Interrupt Enable：用于控制哪些中断可以在 Supervisor 模式下被启用或禁用

    - SEIE (Supervisor External Interrupt Enable):

        外部中断。当 SEIE 位被设置为 1 时，允许外部中断在 Supervisor 模式下触发

    - STIE (Supervisor Timer Interrupt Enable):

        定时器中断。当 STIE 位被设置为 1 时，允许定时器中断在 Supervisor 模式下触发

    - SSIE (Supervisor Soft Interrupt Enable):

        软中断。当 SSIE 位被设置为 1 时，允许软中断在 Supervisor 模式下触发

## 指令
- csrr: Control and Status Register Read

    `csrr a1, mhartid`: 从 CSR (Control and Status Register) 中读取当前 CPU 核心的 ID，即 hartid，并将其存入 a1 寄存器
- csrw: Control and Status Register Write(控制状态寄存器写入）
- la : load address

    `la sp, stack0`: 将 stack0 的地址加载到 sp 寄存器
- li : load immediate

    `li a0, 1024*4`: 将栈的大小（4096 字节，即 1024 * 4）加载到临时寄存器 a0

## 内联汇编
asm: 关键字，表示这是一条汇编指令
volatile: 这是一个修饰符，告诉编译器不要优化这段汇编代码，即使它看起来没有任何副作用（例如，如果只读取了一个值而不使用它）。这对于涉及硬件状态的读写操作至关重要。

- `asm volatile("csrr %0, mstatus" : "=r" (x) );`

    - `"csrr %0, mstatus"` : 汇编指令模板
    - `%0` : 这是一个占位符，代表一个输出操作数. 它会根据下面的约束被替换为相应的寄存器
    - `:` : 分隔符，用于分隔汇编指令和操作数列表
    - `"=r" (x)`: 输出操作数列表

        - `=r`: 这是一个约束 (constraint)

            - `=`: 表示这个操作数是只写的
            - `r`: 表示编译器应该选择一个通用寄存器 (general purpose register) 来存放这个值

        - `(x)`: x是 C 语言变量，编译器会将 x 映射到它选择的寄存器中。最终，csrr 指令读取 mstatus 的值，并将其存入这个寄存器，然后编译器将该寄存器的值赋给 C 变量 x