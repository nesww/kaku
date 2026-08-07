[BITS 16]
[org 0x7c00]; place bootloader code from 0x7c00


CODE_SEG equ gdt_code - gdt_start   ; = 0x08
DATA_SEG equ gdt_data - gdt_start   ; = 0x10
CODE_USER_SEG equ gdt_user_code - gdt_start ; = 0x18
DATA_USER_SEG equ gdt_user_data - gdt_start ; = 0x20

section .text
  cli
  xor ax, ax
  mov ss, ax; 16-bits stack section
  mov ax, 0x8000
  mov sp, ax ; stack pointer, only for bootloader

  mov [boot_drive], dl
  sti
  call disk_load
  call memory_map
  call vesa_init
  call enter_pm
  hlt

; load kernel code from the disk via EDD/LBA (handles kernels > 128 sectors)
disk_load:
    mov word [disk_packet + 4], 0      ; dest offset = 0
    mov word [disk_packet + 6], 0x1000 ; dest segment (0x1000:0x0000 = 0x10000)
    mov dword [disk_packet + 8], 1     ; start LBA = 1 (kernel at LBA sector 1)
    mov ax, KERNEL_SECTORS
    mov [remaining], ax
.load_loop:
    mov ax, [remaining]
    test ax, ax
    jz .done
    mov cx, 64
    cmp ax, cx
    jbe .set_chunk
    mov ax, cx
.set_chunk:
    mov [disk_packet + 2], ax          ; sectors to read this chunk
    mov si, disk_packet
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    mov cx, [remaining]
    sub cx, ax
    mov [remaining], cx
    add [disk_packet + 8], ax          ; LBA += chunk
    mov ax, [disk_packet + 2]
    shl ax, 5                          ; chunk * 32
    add [disk_packet + 6], ax          ; dest segment += chunk*32
    jmp .load_loop
.done:
    ret

disk_error:
    hlt

disk_packet:
    db 0x10        ; packet size
    db 0           ; reserved
    dw 0           ; sectors to read
    dw 0           ; dest offset
    dw 0x1000      ; dest segment
    dq 1           ; start LBA

remaining:
    dw 0

memory_map:
    ;counter for memory map entries
    mov dword [0x500], 0
    ;start of memory map
    mov di, 0x504
    xor ax, ax
    mov es, ax
    xor ebx, ebx            ; make the interruption start at entry 0
.memory_map_start:
    ;configure interrupt
    mov eax, 0xe820         ; int 0x15 function number
    mov ecx, 24             ; 24 bytes for one entry
    mov edx, 0x534d4150      ; SMAP signature
    int 0x15

    inc dword [0x500]
    add di, 24
    test ebx, ebx
    jz .done
    jmp .memory_map_start
.done:
    ret

vesa_init:
    mov ax, 0x4f02
    mov bx, 0x4144
    or bx, 0x4000
    int 0x10

    mov ax, 0x4f01
    mov cx, 0x4144
    mov di, 0x7e00
    xor bx, bx
    mov es, bx
    int 0x10
    mov eax, [0x7e00 + 0x28]
    mov [0x600], eax
    ret

; enter in protected mode (32 bits)
enter_pm:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:pm_start

; reset all registers before jumping to kernel
[BITS 32]
pm_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp
    mov eax, 0x10000
    jmp eax

boot_drive db 0

;GDT table for protected mode and memory segments
gdt_start:
gdt_null:
    dd 0x00000000
    dd 0x00000000
gdt_code:
    dw 0xFFFF       ; limit bits 0-15
    dw 0x0000       ; base bits 0-15
    db 0x00         ; base bits 16-23
    db 0x9A         ; access byte (code)
    db 0xCF         ; flags + limit bits 16-19
    db 0x00         ; base bits 24-31
gdt_data:
    dw 0xFFFF       ; limit bits 0-15
    dw 0x0000       ; base bits 0-15
    db 0x00         ; base bits 16-23
    db 0x92         ; access byte (data)
    db 0xCF         ; flags + limit bits 16-19
    db 0x00         ; base bits 24-31
gdt_user_code:
    dw 0xFFFF       ; limit bits 0-15
    dw 0x0000       ; base bits 0-15
    db 0x00         ; base bits 16-23
    db 0xFA         ; access byte (code, DPL=3)
    db 0xCF         ; flags + limit bits 16-19
    db 0x00         ; base bits 24-31
gdt_user_data:
    dw 0xFFFF       ; limit bits 0-15
    dw 0x0000       ; base bits 0-15
    db 0x00         ; base bits 16-23
    db 0xF2         ; access byte (data, DPL=3)
    db 0xCF         ; flags + limit bits 16-19
    db 0x00         ; base bits 24-31
gdt_tss:
    dq 0
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; to have the boot sector exactly 512 bytes
times 510-($-$$) db 0
dw 0xAA55
