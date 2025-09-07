//
// assembler macros to create x86 segments
//

// gdt第一项为0. intel为了防止加电后段寄存器未被初始化就进入保护模式而规定的
#define SEG_NULLASM                                             \
        .word 0, 0;                                             \
        .byte 0, 0, 0, 0

// The 0xC0 means the limit is in 4096-byte units
// and (for executable segments) 32-bit mode.
// 0x90=1001 0000: P=1, in memery; S=1, code or data
// 0xC0=1100 0000: G=1, 4k; D/B=1, 32bit segment
// seg_len = seg_limit*g = 0xfffff(20b) * 4k
// seg_limit = (lim) >> 12
//
// base=0: 这个段从物理内存地址0开始
// lim=0xffffffff: 是 32 位地址空间的最大值4GB. `((0xffffffff) >> 28) & 0xf`取出 Limit 的高 4 位, `(0xffffffff) >> 12) & 0xffff`取出 Limit 次高 的 16 位, 共取出前20位
// 根据 G=1 的计算公式，段的实际大小 = (Limit + 1) * 4KB = (0xfffff + 1) * 4KB = 4G
// 将基地址设置为0、大小设置为4GB的设计是一种常见的技巧，用于在保护模式下实现平坦内存模型（flat memory model）。
// 这意味着虚拟地址和物理地址之间是1:1的直接映射，简化了地址转换，便于操作系统启动和内核初始化
// 0x90 = 0b10010000 = (P=1|DPL=0(内核级)|S=1(代码或数据段))
// 0xC0 = 0b11000000 = (G=1(4KB)|D/B=1(32位)|L=0|AVL=0)
// SEG_ASM() 宏的实现是适配 小端 x86 架构(低位字节会被放在前面)
#define SEG_ASM(type,base,lim)                                  \
        .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);      \
        .byte (((base) >> 16) & 0xff), (0x90 | (type)),         \
                (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

#define STA_X     0x8       // Executable segment
#define STA_W     0x2       // Writeable (non-executable segments)
#define STA_R     0x2       // Readable (executable segments)
