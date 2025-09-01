# mem
```bash
# qemu-system-riscv64 -monitor stdio # 启动一个 RISC-V 64 位虚拟机，并在终端上打开一个 QEMU 监视器控制台
QEMU 8.2.2 monitor - type 'help' for more information
(qemu) info mtree # 提供了这个虚拟机的内存地址映射
address-space: cpu-memory-0
address-space: memory
  0000000000000000-ffffffffffffffff (prio 0, i/o): system
    0000000000001000-000000000000ffff (prio 0, rom): riscv.spike.mrom # 机器模式 ROM（只读存储器）. 它通常存放引导加载程序（bootloader）或一小段固件，它们是虚拟机启动时首先运行的代码
    0000000001000000-000000000100000f (prio 1, i/o): riscv.htif.uart # 一个 UART（通用异步收发器）的地址范围，这是一个简单的串行通信设备。客户操作系统使用这个地址来向虚拟串行端口写入或从中读取数据。htif（主机-目标接口）表示它是一个特定的 QEMU 管理的设备
    0000000002000000-0000000002003fff (prio 0, i/o): riscv.aclint.swi
    0000000002004000-000000000200bfff (prio 0, i/o): riscv.aclint.mtimer # 是 ACLINT (Advanced Core Local Interruptor, 高级核心本地中断控制器）设备. swi 部分处理软件中断，而 mtimer 是一个机器模式定时器
    0000000080000000-0000000087ffffff (prio 0, ram): riscv.spike.ram # 主 RAM（随机存取存储器, 这里是128MB）. 客户操作系统会将内核和所有用户空间程序加载到这个区域. 地址 0x80000000 是 RISC-V 裸机和嵌入式系统中 RAM 常见的起始地址

address-space: I/O
  0000000000000000-000000000000ffff (prio 0, i/o): io # 一个用于 I/O 设备的单独地址空间，尽管对于大多数系统来说，它通常是主内存空间的镜像
```