# mem
xv6基于qemu-system-riscv64 virt开发, 其相关的内存布局见qemu源码的[virt_memmap](https://github.com/qemu/qemu/blob/baa79455fa92984ff0f4b9ae94bed66823177a27/hw/riscv/virt.c#L82)

```bash
# --- 通过dts查看内存布局
# qemu-system-riscv64 -machine virt -machine dumpdtb=virt.dtb
# dtc -I dtb -O dts -o virt.dts virt.dtb
# --- 通过qemu monitor查看内存布局
# qemu-system-riscv64 -monitor stdio # 启动一个 RISC-V 64 位虚拟机，并在终端上打开一个 QEMU 监视器控制台, 此时host cpu负载很高, 注意及时退出
QEMU 8.2.2 monitor - type 'help' for more information
(qemu) info mtree # 提供了这个虚拟机的内存地址映射
address-space: cpu-memory-0
address-space: memory
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000000001000-000000000000ffff (prio 0, rom): riscv.spike.mrom # 机器模式 ROM（只读存储器）. 它通常存放引导加载程序（bootloader）或一小段固件，它们是虚拟机启动时首先运行的代码
    0000000001000000-000000000100000f (prio 1, i/o): riscv.htif.uart # 一个 UART（通用异步收发器）的地址范围，这是一个简单的串行通信设备。客户操作系统使用这个地址来向虚拟串行端口写入或从中读取数据。htif（主机-目标接口）表示它是一个特定的 QEMU 管理的设备
    0000000002000000-0000000002003fff (prio 0, i/o): riscv.aclint.swi
    0000000002004000-000000000200bfff (prio 0, i/o): riscv.aclint.mtimer # 是 ACLINT (Advanced Core Local Interruptor, 高级核心本地中断控制器）设备. swi 部分处理软件中断，而 mtimer 是一个机器模式定时器
    0000000080000000-0000000087ffffff (prio 0, ram): riscv.spike.ram # 主 RAM（随机存取存储器, 这里大小是128MB, 0x80000000=2G）. 客户操作系统会将内核和所有用户空间程序加载到这个区域. 地址 0x80000000 是 RISC-V 裸机和嵌入式系统中 RAM 常见的起始地址

address-space: I/O
  0000000000000000-000000000000ffff (prio 0, i/o): io # 一个用于 I/O 设备的单独地址空间，对于大多数系统来说，它通常是主内存空间的镜像
```

kernel.ld让kernel也是从0x80000000开始执行的, 它还定义了几个特殊的符号:
```bash
# readelf -s kernel | grep -E "etext|stack0|end"
readelf -s kernel/kernel | grep -E "entry|trampoline|etext|stack0|end"
    16: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS entry.o
   110: 0000000080006000     0 NOTYPE  GLOBAL DEFAULT    1 trampoline
   158: 0000000080003c14   282 FUNC    GLOBAL DEFAULT    1 end_op
   168: 0000000080007860 32768 OBJECT  GLOBAL DEFAULT    4 stack0
   188: 0000000080020b68     0 NOTYPE  GLOBAL DEFAULT    4 end
   192: 0000000080007000     0 NOTYPE  GLOBAL DEFAULT    1 etext
   197: 0000000080006000     0 NOTYPE  GLOBAL DEFAULT    1 _trampoline
   217: 0000000080000000     0 NOTYPE  GLOBAL DEFAULT    1 _entry
```

layout:
1. KERNBASE: 0x80000000

  内核起始位置，内核加载到这个部分开始执行
1. trampoline: 0x80006000
1. etext:      0x80007000
1. stack0:     0x80007860
1. end:        0x80020b68
1. PHYSTOP:    0x88000000

  （系统认为的）物理地址终止地址，所有代码、进程内存不会超过这个范围，即可用的部分为KERNBASE直到PHYSTOP的空间

end~PHYSTOP=free ram space

## mmu
- [Virtual Memory Layout on RISC-V Linux](https://www.kernel.org/doc/html/v6.6/riscv/vm-layout.html)

	目前最新的64位linux支持Sv39和Sv48两种格式. 内核起始地址也是0x80000000
- [RISC-V 内存虚拟化简析（二）](https://tinylab.org/riscv-kvm-mem-virt-2/)

  含SvX va/pa的说明

  SvX 表示存储系统中使用 X 位的虚拟地址，不同位数的虚拟地址对应的物理地址的位宽也会有所不同（Sv32 的 PA 位宽为 34，其它均为 56 位），除去 VA 和 PA 中的 12 位 Page Offset，每个页表项（PTE）实际存储的是物理页号（PPN）。对应页表项中 PPN 的位宽为 22（Sv32）或 44（Sv39，Sv48，Sv57）

[riscv特权文档 - The RISC-V Instruction Set Manual Volume II: Privileged Architecture](https://riscv.atlassian.net/wiki/spaces/HOME/pages/16154769/RISC-V+Technical+Specifications) 12.x章节中定义了Sv32、Sv39、Sv48和Sv57 这几种虚拟地址, sv后面的数字表示支持多少位的虚拟地址. 其中Sv32是用于32位系统的，Sv39、Sv48和Sv57则是用于64位系统. Sv39、Sv48、Sv57分别也就对应三级页表，四级页表和五级页表.

xv6-riscv采用的即为sv39的方式:
![sv39](/docs/riscv/misc/img/mem/sv39.png)

## mem
![内存架构](/docs/riscv/misc/img/mem_arch.png)