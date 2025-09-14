//
// low-level driver routines for 16550a UART.
//

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

// the UART control registers are memory-mapped
// at address UART0. this macro returns the
// address of one of the registers.
// Reg实质上是UART在MMIO中的基址加上对应寄存器的偏移量来实现的
// volatile: 告诉编译器，被指针指向的内存位置可能会在程序外部（例如，由硬件）发生改变。因此，编译器不能对针对该地址的读写操作进行优化
// 这对于直接操作硬件寄存器是必不可少的，因为它确保了每次读写都直接反映到硬件上
#define Reg(reg) ((volatile unsigned char *)(UART0 + (reg)))

// the UART control registers.
// some have different meanings for
// read vs write.
// see http://byterunner.com/16550.html, about `REGISTER BIT MAPS`
#define RHR 0                 // receive holding register (for input bytes)
#define THR 0                 // transmit holding register (for output bytes)
#define IER 1                 // interrupt enable register
#define IER_RX_ENABLE (1<<0)
#define IER_TX_ENABLE (1<<1)
#define FCR 2                 // FIFO control register
#define FCR_FIFO_ENABLE (1<<0)
#define FCR_FIFO_CLEAR (3<<1) // clear the content of the two FIFOs
#define ISR 2                 // interrupt status register
#define LCR 3                 // line control register
#define LCR_EIGHT_BITS (3<<0)
#define LCR_BAUD_LATCH (1<<7) // special mode to set baud rate
#define LSR 5                 // line status register
#define LSR_RX_READY (1<<0)   // input is waiting to be read from RHR
#define LSR_TX_IDLE (1<<5)    // THR can accept another character to send

#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = (v))

// for transmission.
static struct spinlock tx_lock;
static int tx_busy;           // is the UART busy sending?
static int tx_chan;           // &tx_chan is the "wait channel"

extern volatile int panicking; // from printf.c
extern volatile int panicked; // from printf.c

// 初始化串口芯片16550
void
uartinit(void)
{
  // disable interrupts.
  WriteReg(IER, 0x00);

  // special mode to set baud rate. // 进入设置波特率的特殊模式
  WriteReg(LCR, LCR_BAUD_LATCH);

  // LSB for baud rate of 38.4K.
  WriteReg(0, 0x03);

  // MSB for baud rate of 38.4K.
  WriteReg(1, 0x00);

  // leave set-baud mode,
  // and set word length to 8 bits, no parity.
  // LCR_EIGHT_BITS设置字长为8bit即一个字节, 无奇偶校验, 并退出设置波特率的模式(bit7置0了)
  WriteReg(LCR, LCR_EIGHT_BITS);

  // reset and enable FIFOs.
  // 清除 FIFO 中的所有数据，确保缓冲区为空，以避免处理之前残留的脏数据
  WriteReg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);

  // enable transmit and receive interrupts.
  // 当 UART 接收到数据或发送缓冲区变空时，CPU 将会收到中断信号
  WriteReg(IER, IER_TX_ENABLE | IER_RX_ENABLE);

  initlock(&tx_lock, "uart");
}

// transmit buf[] to the uart. it blocks if the
// uart is busy, so it cannot be called from
// interrupts, only from write() system calls.
void
uartwrite(char buf[], int n)
{
  acquire(&tx_lock);

  int i = 0;
  while(i < n){ 
    while(tx_busy != 0){
      // wait for a UART transmit-complete interrupt
      // to set tx_busy to 0.
      sleep(&tx_chan, &tx_lock);
    }   
      
    WriteReg(THR, buf[i]);
    i += 1;
    tx_busy = 1;
  }

  release(&tx_lock);
}


// write a byte to the uart without using
// interrupts, for use by kernel printf() and
// to echo characters. it spins waiting for the uart's
// output register to be empty.
void
uartputc_sync(int c)
{
  if(panicking == 0)
    push_off();

  // 如果内核已经处于恐慌状态，函数会进入一个无限循环。这是为了在系统崩溃时，防止程序继续执行，从而保证调试信息能够被正确输出
  if(panicked){
    for(;;)
      ;
  }

  // wait for Transmit Holding Empty to be set in LSR.
  // 实现同步的关键
  // 直到 LSR_TX_IDLE 位被设置为 1，这意味着 UART 已经处理完上一个字符，可以接收下一个字符了
  while((ReadReg(LSR) & LSR_TX_IDLE) == 0)
    ;
  WriteReg(THR, c);

  if(panicking == 0)
    pop_off();
}

// read one input character from the UART.
// return -1 if none is waiting.
int
uartgetc(void)
{
  if(ReadReg(LSR) & LSR_RX_READY){
    // input data is ready.
    return ReadReg(RHR);
  } else {
    return -1;
  }
}

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from devintr().
void
uartintr(void)
{
  ReadReg(ISR); // acknowledge the interrupt

  acquire(&tx_lock);
  if(ReadReg(LSR) & LSR_TX_IDLE){
    // UART finished transmitting; wake up sending thread.
    tx_busy = 0;
    wakeup(&tx_chan);
  }
  release(&tx_lock);

  // read and process incoming characters.
  while(1){
    int c = uartgetc();
    if(c == -1)
      break;
    consoleintr(c);
  }
}
