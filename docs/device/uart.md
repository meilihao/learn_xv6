# uart
ref:
- [TECHNICAL DATA ON 16550](http://byterunner.com/16550.html)

   `REGISTER BIT MAPS`的`A2	A1	A0`:表示寄存器位置的偏移地址

## 寄存器
1. IER

    IER寄存器控制着芯片上所有的中断的使能, IER=0相当于关闭了所有UART可能发出的中断

    bit:
    - 0: 管理receive holding register中断, 即接收
    - 1: 管理transmit holding register中断, 即发送
    - 2: 管理receiver line status register中断
    - 3: 管理modem status register中断
    - 4-7: 不使用一直为0
1. FCR

    bit:
    - 0: 是否使能transmit and receive FIFO
    - 1: 是否清空接收FIFO
    - 2: 是否清空发送FIFO
1. LCR

    bit:
    - 0/1:

        这两位指定要发送或接收的字长

        BIT-1 BIT-0 字长
        0 0 5
        0 1 6
        1 0 7
        1 1 8
    - 3: 是否开启奇偶校验
    - 7: divisor latch enable, 置1允许设置UART的波特率, 置0回到正常模式

        启用PROGRAMMING TABLE, 通过为波特率发生器的 MSB(寄存器地址偏移=1) 和 LSB(寄存器地址偏移=0) 选择合适的除数值，可以实现自定义波特率, 见[BAUD RATE GENERATOR PROGRAMMING TABLE](http://byterunner.com/16550.html)

        LSB=3,MSB=0时波特率为38400. 之所以设置为38.4K的波特率，与qemu的具体实现代码有关, 可参考qemu/hw/riscv/virt.c/create_fdt_uart函数

        波特率 = (时钟频率) / (16 * 分频值) = 1.8432 MHZ / (16*3) = 38.4K