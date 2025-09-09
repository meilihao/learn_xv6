#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

// 设置全局的中断源优先级
void
plicinit(void)
{
  // set desired IRQ priorities non-zero (otherwise disabled).
  // 根据 RISC-V PLIC 规范，中断优先级寄存器的值决定了该中断的优先级.
  // 如果优先级为 0，则该中断被禁用, 任何非零的值都会启用该中断
  // 优先级寄存器是连续排列的，第 N 号中断的优先级寄存器位于基地址 PLIC + N*4
  *(uint32*)(PLIC + UART0_IRQ*4) = 1;
  *(uint32*)(PLIC + VIRTIO0_IRQ*4) = 1;
}

// 配置每个核心的本地中断使能和阈值
void
plicinithart(void)
{
  int hart = cpuid();
  
  // set enable bits for this hart's S-mode
  // for the uart and virtio disk.
  *(uint32*)PLIC_SENABLE(hart) = (1 << UART0_IRQ) | (1 << VIRTIO0_IRQ);

  // set this hart's S-mode priority threshold to 0.
  // 设置了当前核心的 S-mode 优先级阈值
  // 根据 RISC-V PLIC 规范，只有当中断源的优先级（在 plicinit() 中设置）高于当前核心的优先级阈值时，该中断才会被实际发送到核心.
  // 将阈值设置为 0 意味着该核心会接收所有优先级大于 0 的中断，从而确保了所有已启用的中断都能被处理
  *(uint32*)PLIC_SPRIORITY(hart) = 0;
}

// ask the PLIC what interrupt we should serve.
int
plic_claim(void)
{
  int hart = cpuid();
  int irq = *(uint32*)PLIC_SCLAIM(hart);
  return irq;
}

// tell the PLIC we've served this IRQ.
void
plic_complete(int irq)
{
  int hart = cpuid();
  *(uint32*)PLIC_SCLAIM(hart) = irq;
}
