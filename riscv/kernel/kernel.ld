OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
  /*
   * ensure that entry.S / _entry is at 0x80000000,
   * where qemu's -kernel jumps.
   */
  . = 0x80000000;

  .text : {
    kernel/entry.o(_entry) /*确保 kernel/entry.o 文件中的 _entry 符号被强制放置在 .text 段的开头. 这保证了程序的入口点是第一个被加载的代码，从而满足 ENTRY(_entry) 的要求*/
    *(.text .text.*) /*合并所有文件.text节和任意以.text开头的节的内容*/
    . = ALIGN(0x1000); /*将当前地址以0x1000（4096）为base进行地址对齐*/
    _trampoline = .;
    *(trampsec) /*合并所有文件trampsec节*/
    . = ALIGN(0x1000);
    ASSERT(. - _trampoline == 0x1000, "error: trampoline larger than one page"); /*断言，确保trampoline不大于一个页*/
    PROVIDE(etext = .); /*定义了一个符号 etext, 标记代码段的结束*/
  }

  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*) /* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
  }

  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*) /* do not need to distinguish this from .data */
    . = ALIGN(16);
    *(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*) /* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
  }

  PROVIDE(end = .); /*定义了一个符号 end，它标记了整个内核镜像的结束地址. 这个符号在内核初始化时非常有用，可以用来计算内核的大小，或者确定堆（heap）的起始位置*/
}
