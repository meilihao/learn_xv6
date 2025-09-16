# PLIC(Platform Level Interrupt Controller)
ref:
- [RISC-V AIA support for RISC-V machines](https://qemu-project.gitlab.io/qemu/specs/riscv-aia.html)

    见`<qemu_source>/hw/riscv/virt.c的virt_memmap变量`
- [riscv-plic-spec](https://github.com/riscv/riscv-plic-spec)

> 目前riscv cpu有APLIC (Advanced Platform Level Interrupt Controller), 甚至带IMSIC (Incoming MSI
Controller), 但xv6仅支持PLIC.

中断的启用和禁用是本地的，只作用于执行该指令的单个 CPU 核心.

中断在关闭期间并不会被丢弃. 它们会被硬件（如 PLIC）缓冲起来，等待 CPU 重新打开中断. 一旦中断被重新启用，这些“待处理”的中断就会被立即处理.