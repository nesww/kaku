[bits 32]
[extern syscall_handler]

[global isr_syscall_stub]
isr_syscall_stub:
    pusha
    push esp
    call syscall_handler
    add esp, 4
    popa
    iret
