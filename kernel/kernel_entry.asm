[BITS 32]
[global kernel_entry]
[extern kernel_main]
KERNEL_VIRT_BASE equ 0xC0000000
KERNEL_LOAD_BASE equ 0x10000
KERNEL_PD_INDEX  equ (KERNEL_VIRT_BASE >> 22)

section .text

kernel_entry:
    mov eax, boot_pd - KERNEL_VIRT_BASE + KERNEL_LOAD_BASE
    mov dword [eax], 0x00000083
    mov dword [eax + 1 * 4], 0x00400083
    mov dword [eax + 2 * 4], 0x00800083
    mov dword [eax + 768 * 4], 0x00000083
    mov dword [eax + 769 * 4], 0x00400083
    mov dword [eax + 770 * 4], 0x00800083
    mov eax, cr4
    or eax, 0x10
    mov cr4, eax
    mov eax, boot_pd - KERNEL_VIRT_BASE + KERNEL_LOAD_BASE
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    lea eax, [higher_half]
    jmp eax
higher_half:
    mov eax, boot_pd - KERNEL_VIRT_BASE + KERNEL_LOAD_BASE
    mov dword [eax], 0
    mov dword [eax + 4], 0
    mov dword [eax + 8], 0
    mov eax, cr3
    mov cr3, eax
    mov esp, 0xC009ff00
    call kernel_main
    hlt

align 4096
boot_pd:
    times 1024 dd 0
