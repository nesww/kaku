#include "idt.h"
#include <stdint.h>
#include "hw/vga/vga.h"
#include "hw/pic/pic.h"
#include "panic/panic.h"
#include "hw/kb/kb.h"
#include "idt_declare.h"
static idt_entry idt[IDT_TAB_SIZE];


void _idt_set_entry(int num, void *isr_wrapper) {
    idt_entry entry = {0};
    entry.zero = 0x0;
    entry.segment_selector = 0x08;
    entry.type_attr = 0x8E;
    uint32_t addr = (uint32_t)isr_wrapper;
    entry.low_handler_addr = (uint16_t)(addr & 0xffff);
    entry.high_handler_addr = (uint16_t)(addr >> 16);
    idt[num] = entry;
}

void idt_init(void) {
    void* handlers[] = {
        // intel legacy ISRs
        isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, isr9, isr10,
        isr11, isr12, isr13, isr14, isr15, isr16, isr17, isr18, isr19, isr20,
        isr21, isr22, isr23, isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31,
        // PIC ISRs
        isr_timer_stub, isr33, isr34, isr35, isr36, isr37, isr38, isr39, isr40, isr41, isr42, isr43, isr44, isr45, isr46, isr47
    };

    for (int i = 0; i < 48; ++i) {
        _idt_set_entry(i, handlers[i]);
    }

    // idt_descriptor desc;
    // desc.limit = sizeof(idt) - 1;
    // desc.base  = (uint32_t)idt;
    uint8_t idt_ptr[6];
    *(uint16_t *)idt_ptr = sizeof(idt) - 1;
    *(uint32_t *)(idt_ptr + 2) = (uint32_t)idt;

    __asm__ volatile(
        "lidt %0" : : "m"(*idt_ptr)
    );
}

void isr_handler(int num, struct interrupt_frame *frame) {
    if (num <= 32) {
        //TODO: change to a switch when other ints are handled
        if (num == INT_PAGE_FAULT) {
            uint32_t cr2;
            __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
            kernel_panicf("PAGE_FAULT: faulty address: %x\n> IP: %x", cr2, frame->ip);
        } else {
            kernel_panic_isr(num, frame);
        }
    } else {
        switch(num) {
            //PIC ints
            // PIC master IRQs
            case INT_PIC_TIMER:          /*vga_print("INT_PIC: timer");*/              break;
            case INT_PIC_KEYBOARD:
                kb_handle_interrupt();
                break;
            case INT_PIC_SERIAL_COM2:       vga_println("INT_PIC: serial COM2");         break;
            case INT_PIC_SERIAL_COM1:       vga_println("INT_PIC: serial COM1");         break;
            case INT_PIC_PARALLEL_PORT:     vga_println("INT_PIC: parallel port");       break;
            case INT_PIC_FLOPPY:            vga_println("INT_PIC: floppy");              break;
            case INT_PIC_PARALLEL_PORT2:    vga_println("INT_PIC: parallel port 2");     break;
            // PIC slave IRQs
            case INT_PIC_REALTIME_CLOCK:    vga_println("INT_PIC: realtime clock");      break;
            case INT_PIC_PS2:               vga_println("INT_PIC: PS/2");                break;
            case INT_PIC_FLOAT_COPROCESSOR: vga_println("INT_PIC: float coprocessor");   break;
            case INT_PIC_ATA_PRIMARY:       vga_println("INT_PIC: ATA primary disk");    break;
            case INT_PIC_ATA_SECONDARY:     vga_println("INT_PIC: ATA secondary disk");  break;
            default:
                vga_printf("INT_PIC: unknown exception or not handled: %d\n", num);
                break;
        }
        pic_sendEOI(num - PIC_INT_OFFSET);
    }
}
