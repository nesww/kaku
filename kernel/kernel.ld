ENTRY(kernel_entry)
SECTIONS {
    . = 0xC0010000;
    .text : AT(0x10000) { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
    kernel_end = .;
    kernel_phys_end = . - 0xC0000000;
}
