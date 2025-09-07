#include "asm.h"
#include "memlayout.h"
#include "mmu.h"

# Start the first CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

# 引导处理器BP(boot strap processor)启动
.code16                       # Assemble for 16-bit mode
.globl start
start:
  cli                         # BIOS enabled interrupts; disable

  # Zero data segment registers DS, ES, and SS.
  xorw    %ax,%ax             # Set %ax to zero
  movw    %ax,%ds             # -> Data Segment
  movw    %ax,%es             # -> Extra Segment
  movw    %ax,%ss             # -> Stack Segment

  # Physical address line A20 is tied to zero so that the first PCs 
  # with 2 MB would run software that assumed 1 MB.  Undo that.
  # 为了访问1MB以上的内存空间，需要使能A20地址线. 不过某些BIOS, 包括模拟器（qemu, Bochs等）会默认打开A20地址线, 不需要手动开启
  # 这里是通过键盘控制器的8042芯片开启A20
  # 0x64(read=Status Register, write=Command Register, 两者共用一个端口)
  # 0x60(read=Output Buffer，write=Input Buffer, 两者共用一个端口 0x60)
seta20.1:
  # 开启A20流程
  # 1. 等待, 直到8042输入缓冲为空为止
  inb     $0x64,%al               # Wait for not busy # 等待直到键盘不忙, 取0x64端口状态值
  testb   $0x2,%al                # 测试寄存器第二个bit(2 = 10)是否为0. 0=输入缓冲为空, 1=输入缓冲不为空
  jnz     seta20.1                # 如果是1,重复循环

  # 2. 发送写8042输出端口命令0xd1到8042的port 0x64
  movb    $0xd1,%al               # 0xd1 -> port 0x64 # d1=write next byte to controller output port
  outb    %al,$0x64

seta20.2:
  # 3. 等待, 直到8042输入缓冲为空为止, 即是否完成处理
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  # 4. 向port 0x60写入0xdf, 表示将A20设置为1, 即开启A20
  movb    $0xdf,%al               # 0xdf -> port 0x60 # DF为11011111, A20位置1, 开启A20线; 0x60=Data Port
  outb    %al,$0x60

  # Switch from real to protected mode.  Use a bootstrap GDT that makes
  # virtual addresses map directly to physical addresses so that the
  # effective memory map doesn't change during the transition.
  lgdt    gdtdesc
  movl    %cr0, %eax     # 将 cr0 寄存器的当前值复制到 eax 寄存器中，以便对其进行修改
  orl     $CR0_PE, %eax  # 使能cr0 PE 位（Protection Enable，保护模式使能）
  movl    %eax, %cr0

//PAGEBREAK!
  # Complete the transition to 32-bit protected mode by using a long jmp
  # to reload %cs and %eip.  The segment descriptors are set up with no
  # translation, so that the mapping is still the identity mapping.
  ljmp    $(SEG_KCODE<<3), $start32 # SEG_KCODE from mmu.h; 会设置cs=SEG_KCODE<<3(TI=0, RPL=0)和eip=start32

.code32  # Tell assembler to generate 32-bit code now.
start32:
  # Set up the protected-mode data segment registers
  movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %ss                # -> SS: Stack Segment
  movw    $0, %ax                 # Zero segments not ready for use
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS

  # Set up the stack pointer and call into C.
  movl    $start, %esp # esp = 0x7C00
  call    bootmain

  # If bootmain returns (it shouldn't), trigger a Bochs
  # breakpoint if running under Bochs, then loop.
  movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
  movw    %ax, %dx
  outw    %ax, %dx
  movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
  outw    %ax, %dx
spin:
  jmp     spin

# Bootstrap GDT
.p2align 2                                # force 4 byte alignment # .p2align 2 是一条汇编指令，用于在汇编代码中控制内存对齐. 参数是对齐的幂次方, 2 表示 2^2，即 4 字节
gdt:
  SEG_NULLASM                             # null seg # from asm.h
  SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg 0xffffffff=4G # SEG_ASM from asm.h
  SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg # 它不需要额外的“可读”标志，因为在x86架构中，可写的数据段总是可读的
# 代码段和数据段的基地址=0, 限长=0xffffffff, 涵盖了整个4GB的虚拟空间, 相当于绕开了x86的分段机制

gdtdesc:
  .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1 # gdtdesc - gdt: 计算GDT的字节大小; - 1: GDT大小必须减1，因为x86规范要求大小是最后一个有效字节的偏移量
  .long   gdt                             # address gdt

