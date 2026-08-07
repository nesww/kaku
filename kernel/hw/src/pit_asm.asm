[bits 32]
[extern scheduler]

[global isr_timer_stub]
isr_timer_stub:
    pusha
    push esp
    call scheduler
    add esp, 4
    mov esp, eax
    popa
    iret
