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
- [XV6实验(2020)](https://blog.csdn.net/weixin_47037146/article/details/128859088)

## labs
ref:
- [schedule](https://pdos.csail.mit.edu/6.S081/2025/schedule.html)
- [book-riscv-rev5.pdf](https://pdos.csail.mit.edu/6.S081/2025/xv6/book-riscv-rev5.pdf)
- [Cyrus-iwnl/xv6-labs-2022](https://github.com/Cyrus-iwnl/xv6-labs-2022)
	- [**讲课**](https://mit-public-courses-cn-translatio.gitbook.io/mit6-s081/)
	- [**实验和手册**](http://xv6.dgs.zone/)
- [TJOS-xv6-labs-2024](https://github.com/xing05188/TJOS-xv6-labs-2024)

[`git clone git://g.csail.mit.edu/xv6-labs-2025`](https://pdos.csail.mit.edu/6.S081/2025/labs/util.html), 较2024有较多变化.

## 环境
ref:
- [xv6-riscv环境搭建](https://groverzhu.github.io/2021/08/17/xv6-riscv%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA/)

```
# apt install gcc-riscv64-unknown-elf gdb-multiarch qemu-system-misc
# riscv64-unknown-elf-gcc --version
# qemu-system-riscv64 --version
# cd xv6-riscv && make qemu
```

调试: 执行make qemu-gdb, 再在同一文件夹下另开一个窗口输入 gdb-multiarch -q kernel/kernel

## 启动过程
ref:
- [Virtual Memory Layout on RISC-V Linux](https://www.kernel.org/doc/html/v6.6/riscv/vm-layout.html)

	2G以上空间

1. 在RISC-V机器上电之后，它会初始化自己并运行一个在只读内存中的bootloader. 该bootloader将xv6的内核加载到虚拟地址为0x80000000(2G)內存中，至于为什么不从0x0开始，那是因为在0x0~0x80000000的地址范围里包含了I/O设备