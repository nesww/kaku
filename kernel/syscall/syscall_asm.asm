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


[global jump_to_userspace]
jump_to_userspace:
    mov eax, [esp + 4]
    mov ecx, [esp + 8]

    mov dx, 0x23
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx

    push 0x23
    push ecx
    pushf
    push 0x1B
    push eax
    iret
