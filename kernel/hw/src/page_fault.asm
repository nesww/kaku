[bits 32]
[extern page_fault_handler]

[global isr_page_fault_stub]
isr_page_fault_stub:
    add esp, 4
    pusha
    push esp
    call page_fault_handler
    add esp, 4
    test eax, eax
    jz .return
    mov esp, eax
    popa
    iret
.return:
    popa
    iret
