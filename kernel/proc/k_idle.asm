[bits 32]
[global idle_stack_top]
[global idle_entry]

section .bss
align 16
idle_stack_bottom:
    resb 4096
idle_stack_top:

section .text
idle_entry:
    sti
    hlt
    jmp idle_entry
