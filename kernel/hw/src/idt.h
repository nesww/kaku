#pragma once

#include <stdint.h>
#include <types.h>

#define IDT_TAB_SIZE 256

#define INTERRUPTS_DISABLE() \
    __asm__ volatile("cli") \

#define INTERRUPTS_ENABLE() \
    __asm__ volatile("sti") \

void isr_handler(int num, struct interrupt_frame *frame);
void idt_init(void);

// INTERRUPTS
#define INT_DIVZERO 0
#define INT_INVALID_OPCODE 6
#define INT_DOUBLE_FAULT 8
#define INT_GEN_PROTECT_FAULT 13
#define INT_PAGE_FAULT 14


#define ISR(num) \
    __attribute__((interrupt)) void isr##num(struct interrupt_frame *frame) { \
        isr_handler(num, frame); \
    }

#define ISR_ERR(num) \
    __attribute__((interrupt)) void isr##num(struct interrupt_frame *frame, uintptr_t error_code) { \
        isr_handler(num, frame); \
    }

typedef struct {
    uint16_t low_handler_addr;
    uint16_t segment_selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t high_handler_addr;
} __attribute__((packed)) idt_entry;


typedef struct {
   uint16_t limit;
   uint32_t base;
} __attribute__((packed))idt_descriptor;

uint32_t page_fault_handler(uint32_t *regs);
