# PLIC(Platform Level Interrupt Controller)
ref:
- [RISC-V AIA support for RISC-V machines](https://qemu-project.gitlab.io/qemu/specs/riscv-aia.html)

    见`<qemu_source>/hw/riscv/virt.c的virt_memmap变量`
- [riscv-plic-spec](https://github.com/riscv/riscv-plic-spec)

> 目前riscv cpu有APLIC (Advanced Platform Level Interrupt Controller), 甚至带IMSIC (Incoming MSI
Controller), 但xv6仅支持PLIC.
