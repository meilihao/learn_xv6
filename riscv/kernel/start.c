#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void main();
void timerinit();

// entry.S needs one stack per CPU.
__attribute__ ((aligned (16))) char stack0[4096 * NCPU];

// entry.S jumps here in machine mode on stack0.
void
start()
{
  // 当前cpu处于M-Mode
  // set M Previous Privilege mode to Supervisor, for mret.
  unsigned long x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK; // 清除MPP的状态
  x |= MSTATUS_MPP_S; // 设置MPP为监督者模式
  w_mstatus(x); // 当下面的 mret 指令执行时，处理器会从机器模式切换到监管者模式

  // set M Exception Program Counter to main, for mret.
  // requires gcc -mcmodel=medany
  w_mepc((uint64)main); // 设置mepc寄存器为main函数地址，这样执行完mret指令之后就会跳转到main函数

  // disable paging for now.
  w_satp(0);

  // delegate all interrupts and exceptions to supervisor mode.
  w_medeleg(0xffff); // 将所有同步异常（如非法指令、地址未对齐）委托给 S-mode 处理
  w_mideleg(0xffff); // 将所有异步中断（如时钟中断、外部中断）委托给 S-mode 处理
  w_sie(r_sie() | SIE_SEIE | SIE_STIE); // 启用 S-mode 的外部中断（SIE_SEIE）和定时器中断（SIE_STIE）

  // configure Physical Memory Protection to give supervisor mode
  // access to all of physical memory.
  w_pmpaddr0(0x3fffffffffffffull); // 0x3FFFFFFFFFFFFFULL（即 50 个 1)???
  w_pmpcfg0(0xf); // 0xf 通常表示读、写、执行权限都启用

  // ask for clock interrupts.
  timerinit();

  // keep each CPU's hartid in its tp register, for cpuid().
  int id = r_mhartid();
  w_tp(id);

  // switch to supervisor mode and jump to main().
  asm volatile("mret"); // w_mepc((uint64)main), 触发跳转到main
}

// ask each hart to generate timer interrupts.
void
timerinit()
{
  // enable supervisor-mode timer interrupts.
  w_mie(r_mie() | MIE_STIE);
  
  // enable the sstc extension (i.e. stimecmp).
  w_menvcfg(r_menvcfg() | (1L << 63)); // 这条指令确保了在 S-mode 下能够使用 stimecmp（监督者模式定时器比较）功能
  
  // allow supervisor to use stimecmp and time.
  w_mcounteren(r_mcounteren() | 2);
  
  // ask for the very first timer interrupt.
  w_stimecmp(r_time() + 1000000); // 设置首次中断的触发时间. 在当前时间的基础上增加一个固定的值，作为下一次中断发生的时间点. 1000000代表了两次中断之间的时间间隔，通常是根据系统时钟频率来设定的
}
