# riscv

## 规范
ref:
- [ISRC-CAS/riscv-isa-manual-cn](https://github.com/ISRC-CAS/riscv-isa-manual-cn)

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

- ra: Return Address. 当一个函数被调用时，RA寄存器会被自动保存下来，以便在函数执行完毕后能够返回调用该函数的位置. 因此，在函数执行过程中，RA寄存器也可以被用来存储函数的返回地址(返回后继续执行的指令的地址)。它也时常被用来函数传参

- mcounteren: 用于控制在 M-mode 下，哪些计数器可以被 S-mode 访问

    - 2：代表 time（机器模式时间）计数器

- medeleg：Machine Exception Delegation Register

    用于指定被委托给更低特权级别的异常，当异常发生时，如果该异常被委托，处理器会将该异常转交给更低特权级别的异常处理程序进行处理。 如果某个位被设置为 1，表示被委托

    RISC-V 架构允许将中断和异常处理从 M-mode 委托（delegate） 给 S-mode. xv6运行在 S-mode，因此需要通过配置 w_medeleg 将相应异常委托给 S-mode 处理，以便内核能够响应和处理这些异常（如系统调用、页故障等）.

    RISC-V 设计了灵活的权限委托机制, 这种机制的目的是简化操作系统（运行在 S 模式）的实现，让其能更自主地管理中断和定时器，而无需频繁陷入 M 模式.
    
    xv6 将中断和异常处理的责任从高特权级的 M-mode 转移到了 S-mode，这是现代操作系统设计的常见做法.
- menvcfg: Machine environment configuration register

    menvcfg 的第 63 位（或 menvcfgh 的第 31 位）名为 STCE（STimecmp Enable），设置为 1 时，用于启用 S 模式下的 stimecmp
- mepc: Machine Exception Program Counter, 机器异常程序计数器. 当发生异常或中断时, cpu会将当前的pc值保存到这个寄存器中, 以便中断处理结束后能返回

    mret 指令在返回时，会从 mepc 寄存器中获取下一条要执行的指令地址

    常见执行操作如下:
    1. 发生异常（中断、故障等）。
    1. 处理器将当前的PC值（即异常发生前正在执行的指令地址）写入 mepc 寄存器
    1. 进入内核处理异常
    1. 异常处理完毕后，返回到 mepc 保存的地址，继续执行
- mideleg：Machine Interrupt Delegation Register：和medeleg类似，用于委托中断
- mie: 机器模式中断使能寄存器. 类似sie, 记录处理器目前能处理和必须忽略的中断，即相应的中断使能位
- mhartid : Machine Hart ID Register, 硬件线程id, 从0开始

    在多核处理器中，每个核心可能有一个或多个硬件线程，每个硬件线程都有一个唯一的 hartid, 通过读取它就可以知道代码运行在哪个core上

    在多核处理器中，每个核心（或硬件线程）都有自己独立的一套寄存器集。这意味着如果有两个核心，每个核心有 32 个通用寄存器，那么整个处理器实际上会有 64 个通用寄存器。每个核心的寄存器集是独立的，互不干扰.
- mstatus:  Machine Status, 机器状态寄存器

    mstatus 寄存器包含了许多重要的处理器状态位，如比如全局中断使能位, 先前的特权级, 虚拟内存模式等, kernel通过它来切换特权级(M->S)

    - MPP (Machine Previous Privilege, 机器模式先前的特权模式): 指示在发生异常或中断之前的特权级别，当使用mret返回时，处理器会切换回原来的级别，在第11 12位
        - 00: User Mode
        - 01: Supervisor Mode
        - 11: Machine Mode
    - MPIE (Machine Previous Interrupt Enable): 记录在发生异常或中断之前，中断是否被启用。
    - MIE (Machine Interrupt Enable): 允许或禁止中断。当为1时，中断被启用。
    - MIE (Machine Interrupt Enable): 记录中断是否被启用
- mcauese/scause : Machine/Supervisor Exception Cause, 机器/监管者异常原因寄存器. 记录发生异常或中断的原因, 比如缺页异常, 非法指令或外部中断
- mscratch/sscratch: Machine/Supervisor Scratch, 机器/监管者临时寄存器

    在陷阱处理的早期阶段, 这是一个非常有用的临时寄存器. 通常内核会把指向当前进程/cpu的内核栈顶的指针放在sscratch中, 这样陷阱处理程序一进来就可以立即切换到内核栈, 而不用破坏任何用户寄存器
- mtvec/stvec : Machine/Supervisor trap vector base address, 机器/监管者陷阱向量基地址寄存器, 指向陷阱程序的入口地址. 当异常发生时, cpu会跳转到这个地址
- mtval/stval : Machine/Supervisor trap value, 机器/监管者陷阱值寄存器，提供与异常相关的附加信息. 例如, 在缺页异常时, 它会保存导致异常的虚拟地址; 在非法指令异常时, 它会保存导致异常的指令
- PMP 控制寄存器/PMP 地址寄存器: pmpaddrN/pmpcfgN

    物理内存保护（PMP, Physical Memory Protection, 在 RISC-V 特权指令集手册 的第 3.7 节）是一种硬件级别的安全特性，主要在机器模式（M-mode）或监管者模式（S-mode）下提供细粒度的内存访问控制, 提供对物理内存的保护和隔离。它通过定义一组规则来限制处理器对特定物理地址范围的访问权限.

    RISC-V架构下的PMP模块允许配置多个独立的区域描述符，这些描述符用于指定某个物理地址范围及其对应的访问权限。具体来说，每个PMP条目由以下几个部分组成:
    1. 地址匹配模式
    1. 大小：受控区域的尺寸
    1. 访问权限：包括读取、写入和执行权限等

    当CPU尝试访问某段物理地址时，PMP单元会逐一检查所有已启用的PMP条目，判断该操作是否违反设定的规则。如果发现违规行为，则触发相应的异常处理程序.

    每一个 PMP entry定义了一个访问控制区域. 一个 PMP 项包括一个 8 位控制寄存器和一个MXLEN-bit长度地址寄存器，支持的条目数量和不同CPU硬件实现相关，通常entry数量是0、16、或64不等，具体数量需要根据对应CPU的手册确定，软件不能使用超过CPU规定的entry数量

    对于 RV32，十六个 CSR（pmpcfg0–pmpcfg15）保存 64 个 PMP 条目的配置 pmp0cfg–pmp63cfg，如图 30 所示。对于 RV64，八个偶数 CSR（pmpcfg0、pmpcfg2、...、pmpcfg14）保存 64 个 PMP 条目的配置. 对于 RV64，奇数配置寄存器 pmpcfg1、pmpcfg3、...、pmpcfg15 是非法的.

    每个 PMP 地址寄存器编码 RV32 的 34 位物理地址的第 33-2 位. 对于 RV64，每个 PMP 地址寄存器编码 56 位物理地址的第 55-2 位. 并非所有物理地址位都能实现，因此 pmpaddr 寄存器是 WARL.

    > RISC-V 规范规定，PMP 匹配的内存区域必须以 4 字节为单位对齐, 最低两位始终为 00，可以被隐式地假定. 这简化了硬件设计，减少了寄存器的位数，同时强制了对齐要求.

    - pmpaddr0-pmpaddrN：地址寄存器（设置内存区域的结束地址或地址范围）

        设置pmpaddr的时候需要将传入的实际物理内存地址左移2bit，即phy_addr << 2. TOR模式可以通过2个entry之间的pmpaddr得到受保护的内存大小，而NA4是固定的4bytes大小，故不用在pmpaddr中传入内存大小

    - pmpcfg0-pmpcfgN：配置寄存器（定义访问权限和模式）

    PMP每一个entry的配置主要有8bit组成，其定义如下：

        bit             width          field_name         desc note
        0                 1                    R                  Read
        1                 1                    W                 Write
        2                 1                    X                  Execute
        3-4              2                    A                   Address matching                                                                - 00:  OFF(disable)
                                                                  - 01：TOR (top of range)
                                                                  - 10：NA4
                                                                  - 11:  NAPOT
        5-6              2                                          Reserved Fix 0
        7                 1                    L                     Lock

        1. R (Read)
            当R==1时，表示该PMP entry对应的地址区域可读，反之不可读（如果读则触发exception）；
        2. W (Write)
            当W==1时，表示该PMP entry对应的地址区域可写，反之不可写（如果写则触发exception）；
        3. X (Execute)
            当X==1时，表示该PMP entry对应的地址区域可执行，反之不可执行（如果执行则触发exception）；
        4. A (Address matching)
            1. 00（OFF）：该条目是disable的，不会去匹配任何地址
            1. 01 TOR（Top of Range，上边界）

               需要使用2个PMP条目定义一块内存保护区域，比如entry[i-1]和entry[i]，同时entry[i-1]的pmpaddr[i-1]必须小于entry[i]的pmpaddr[i],仅仅需要把entry[i]的pmpcfg[i]设置为TOR模式即可（不用考虑entry[i-1]中pmpcfg[i-1]的权限配置），如果i=0，则小于pmpaddr[0]的任何内存地址的访问权限都受entry[0]的配置影响，如果pmpaddr[i-1]>= pmpaddr[i],并且pmpcfg[i]设置TOR模式，则entry[i]匹配不到任何地址

               该地址区域可表示为 [pmpaddr(X-1), pmpaddrX)。特别的是，PMP 条目 0 表示的地址区域为 [0, pmpaddr0)。如果 pmpaddri−1 ≥ pmpaddri，则 PMP 条目 i 不匹配任何地址，即区域为空

            1. 02 NA4（naturally aligned four-byte region, 4 字节对齐的精确匹配）

                NA4是NAPOT的特殊情况，既受保护的内存大小刚好为4byte
            1. 03 NAPOT（Naturally Aligned Power-of-Two, 任意幂次大小的对齐区域）

                内存大小必须是2的次幂大小，且最小值为8bytes

                PMP 地址寄存器的低位来用来表示区域的大小，高位表示以 4 字节为单位的基址，中间用一个 0 隔开. 如果用 G 表示 0 的下标，则 PMP 访问控制区域大小为 2^(G+3) 字节.

                公式: pmpaddr = (phy_addr << 2) | ((phy_size>>3) - 1);//only for NAPOT mode

        5. L（Lock）：如果Lock位被置1，则PMP的entry就会被锁定，写PMP的配置寄存器和地址寄存器都会被忽略，只有复位硬件才能解除锁定，当lock为0时，只有在用户模式和超级管理员模式下访问PMP配置的内存区域才会受影响，机器模式不受影响，此时机器模式可以访问任何内存区域，但是当lock为1时，所有模式下访问PMP配置的内存区域都会受影响，包括机器模式

    ```c
    w_pmpaddr0(0x3fffffffffffffull); // 0x3FFFFFFFFFFFFFULL = 54位全 1, addr=0x3FFFFFFFFFFFFFULL<<2, 已接近riscv规范中的pa的max值(pa为56位)
    w_pmpcfg0(0xf) // 0xF = 1111b = R=1, W=1, X=1, A1:A0=1 (NAPOT（地址范围压缩表示方式）模式)
    ```

- satp : Supervisor Address Translation and Protection Register

    监督地址转换和保护寄存器，虚拟内存的核心. 包含了页表的物理基地址和当前使用的虚拟地址模式(比如sv39). cpu的mmu靠它来查找页表进行地址转换. 它用于控制和配置页表的地址. 它定义了当前页表的位置和格式，以及一些与地址翻译和保护相关的信息
- sepc: Supervisor Exception Program Counter, 监管者异常程序计数器. 当发生异常或中断时, cpu会将当前的pc值保存到这个寄存器中, 以便中断处理结束后能返回
- sie:Supervisor Interrupt Enable：用于控制哪些中断可以在 Supervisor 模式下被启用或禁用

    - SEIE (Supervisor External Interrupt Enable):

        外部中断。当 SEIE 位被设置为 1 时，允许外部中断在 Supervisor 模式下触发

    - STIE (Supervisor Timer Interrupt Enable):

        定时器中断。当 STIE 位被设置为 1 时，允许定时器中断在 Supervisor 模式下触发

    - SSIE (Supervisor Soft Interrupt Enable):

        软中断。当 SSIE 位被设置为 1 时，允许软中断在 Supervisor 模式下触发

- sstatus: supervisor status register, 监管者状态寄存器, 是mstatus的子集, 拥有S-Mode, 包含监管者模式下的中断使能位(SIE), 以及用户模式(U-Mode)的中断是否使能(SPIE)

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

## ABI
RISC-V ABI规定寄存器和堆栈用于函数参数传递，并且参数从左到右的顺序是从寄存器到堆栈：前几个参数按顺序存储在通用寄存器（a0到a7）中，如果参数过多，剩余参数将依次存储在堆栈中