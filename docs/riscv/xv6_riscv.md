# xv6_riscv
## 源码
ref:
- [xv6 makefile详解](https://blog.csdn.net/m0_61567378/article/details/128357706)
- [xv6 book](https://pdos.csail.mit.edu/6.1810/2023/xv6/)

	riscv最新一个版本是book-riscv-rev3.pdf
- [xv6-book](https://github.com/deyuhua/xv6-book-chinese)
- [source](https://github.com/mit-pdos/xv6-riscv)
- [S1-Boot XV6](https://zhuanlan.zhihu.com/p/573032543)
- [XV6 启动过程](https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/lec03-os-organization-and-system-calls/3.9-xv6-qi-dong-guo-cheng)
- [XV6：操作系统组成](https://zhuanlan.zhihu.com/p/624607389)
- [**MIT 6.S081**](https://blog.csdn.net/zzy980511/category_11740137.html)

## labs
ref:
- [schedule](https://pdos.csail.mit.edu/6.S081/2025/schedule.html)
- [book-riscv-rev5.pdf](https://pdos.csail.mit.edu/6.S081/2025/xv6/book-riscv-rev5.pdf)
- [Cyrus-iwnl/xv6-labs-2022](https://github.com/Cyrus-iwnl/xv6-labs-2022)
	- [**讲课**](https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/)

		`book-riscv-rev<X>.pdf`的课堂讲解
	- [**实验和手册**](http://xv6.dgs.zone/)

		`book-riscv-rev<X>.pdf`的翻译
- [Xv6 Labs 实验记录](https://ttzytt.com/categories/%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95/)
- [XV6实验(2020)](https://blog.csdn.net/weixin_47037146/article/details/128859088)
- [TJOS-xv6-labs-2024](https://github.com/xing05188/TJOS-xv6-labs-2024)

[`git clone git://g.csail.mit.edu/xv6-labs-2025`](https://pdos.csail.mit.edu/6.S081/2025/labs/util.html), 较2024有较多变化.

## 环境
ref:
- [xv6-riscv环境搭建](https://groverzhu.github.io/2021/08/17/xv6-riscv%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA/)
- [Tools Used in 6.1810](https://pdos.csail.mit.edu/6.S081/2025/tools.html)

```
# apt install gcc-riscv64-unknown-elf gdb-multiarch qemu-system-misc
# riscv64-unknown-elf-gcc --version
# qemu-system-riscv64 --version
# cd xv6-riscv && make qemu
```

gcc-riscv64-unknown-elf: riscv跨平台编译工具链
qemu-system-misc: 按照riscv模拟器
gdb-multiarch: gdb调试器, 支持调试其他arch

调试: 执行make CPUS=1 qemu-gdb(退出:按下 Ctrl+a，然后按下 x. 使用`CPUS=1`避免多核干扰调试), 再在同一文件夹下另开一个窗口输入:
```bash
# gdb-multiarch -q kernel/kernel
(gdb) source .gdbinit # gdbinit没有生效时可手动加载, 比如因"auto-load safe-path"被拒绝时
(gdb) target remote localhost:26000 # 添加到gdb server
Remote debugging using localhost:26000
0x0000000000001000 in ?? ()
(gdb) info registers
ra             0x0      0x0
sp             0x0      0x0
gp             0x0      0x0
tp             0x0      0x0
t0             0x0      0
t1             0x0      0
t2             0x0      0
fp             0x0      0x0
s1             0x0      0
a0             0x0      0
a1             0x0      0
a2             0x0      0
a3             0x0      0
a4             0x0      0
a5             0x0      0
a6             0x0      0
a7             0x0      0
s2             0x0      0
s3             0x0      0
s4             0x0      0
s5             0x0      0
s6             0x0      0
s7             0x0      0
s8             0x0      0
s9             0x0      0
s10            0x0      0
s11            0x0      0
t3             0x0      0
t4             0x0      0
t5             0x0      0
t6             0x0      0
pc             0x1000   0x1000 # riscv在启动时，pc被默认设置为0x1000，而OpenSBI 的入口点通常就是 0x1000，OpenSBI完成后跳转到0x80000000
(gdb) break *0x80000000
(gdb) c # 跳转到_entry
(gdb) layout split # 同时查看汇编和源码
```

## 启动过程
ref:
- [xv6启动流程](https://www.cnblogs.com/INnoVationv2/p/18148489)

1. 在RISC-V机器上电之后，它会初始化自己并执行ROM上的固件. 该固件将xv6的内核加载到物理地址为0x80000000(2G)內存中，至于为什么不从0x0开始，那是因为在0x0~0x80000000的地址范围里包含了I/O设备

	qemu-system-riscv64 的 PC 从 0x1000 开始是一个设计上的选择和约定，旨在模拟一个真实的、包含引导加载程序的 RISC-V 系统。0x1000 是 OpenSBI 这个特权级固件的入口点，它负责后续的初始化工作，并最终将控制权交给更高层级的软件（如操作系统内核），而这些内核通常被加载到 0x80000000 处.

	0x80000000 在 RISC-V 架构中是一个约定俗成的物理基地址 (Physical Base Address). 它通常被指定为主系统 DRAM（动态随机存取内存）的起始地址