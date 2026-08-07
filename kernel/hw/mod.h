#pragma once

#include <stdint.h>

/*===========================================================================
 * hw — hardware low-level API
 *
 * selective includes: define HW_<SUBMOD>_IPL before including
 * this header to pull in a specific subsystem.
 *
 *   #define HW_IO_IPL
 *   #define HW_IRQ_IPL
 *   #define HW_TIMER_IPL
 *   #define HW_IDT_IPL
 *   #include <hw/mod.h>
 *
 * or define HW_ALL_IPL to include everything.
 *===========================================================================*/

#ifdef HW_ALL_IPL
#  define HW_IO_IPL
#  define HW_IRQ_IPL
#  define HW_TIMER_IPL
#  define HW_IDT_IPL
#endif

/*===========================================================================
 * hw/io — Port I/O
 *===========================================================================*/
#ifdef HW_IO_IPL

#define IO_KEYBOARD_DATA_PORT 0x60
#define IO_VGA_INDEX_PORT     0x3D4
#define IO_VGA_DATA_PORT      0x3D5
#define IO_VGA_DATA_POS_HIGH_BITS 0x0E
#define IO_VGA_DATA_POS_LOW_BITS  0x0F

static inline uint8_t hw_io_in8(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t hw_io_in16(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value): "Nd"(port));
    return value;
}

static inline void hw_io_out8(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void hw_io_out16(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) { return hw_io_in8(port); }
static inline uint16_t inw(uint16_t port) { return hw_io_in16(port); }
static inline void outb(uint16_t port, uint8_t value) { hw_io_out8(port, value); }
static inline void outw(uint16_t port, uint16_t value) { hw_io_out16(port, value); }

#endif

/*===========================================================================
 * hw/irq — PIC (Programmable Interrupt Controller)
 *===========================================================================*/
#ifdef HW_IRQ_IPL

#define HW_IRQ_TIMER             32
#define HW_IRQ_KEYBOARD          33
#define HW_IRQ_SERIAL_COM2       35
#define HW_IRQ_SERIAL_COM1       36
#define HW_IRQ_PARALLEL_PORT     37
#define HW_IRQ_FLOPPY            38
#define HW_IRQ_REALTIME_CLOCK    40
#define HW_IRQ_PS2               44
#define HW_IRQ_ATA_PRIMARY       46
#define HW_IRQ_ATA_SECONDARY     47

#define HW_PIC_MASTER 0x20
#define HW_PIC_SLAVE  0xa0
#define HW_PIC_EOI    0x20

#define INT_PIC_TIMER             HW_IRQ_TIMER
#define INT_PIC_KEYBOARD          HW_IRQ_KEYBOARD
#define INT_PIC_SERIAL_COM2       HW_IRQ_SERIAL_COM2
#define INT_PIC_SERIAL_COM1       HW_IRQ_SERIAL_COM1
#define INT_PIC_PARALLEL_PORT     HW_IRQ_PARALLEL_PORT
#define INT_PIC_PARALLEL_PORT2    39
#define INT_PIC_FLOPPY            HW_IRQ_FLOPPY
#define INT_PIC_FLOAT_COPROCESSOR 45
#define INT_PIC_REALTIME_CLOCK    HW_IRQ_REALTIME_CLOCK
#define INT_PIC_PS2               HW_IRQ_PS2
#define INT_PIC_ATA_PRIMARY       HW_IRQ_ATA_PRIMARY
#define INT_PIC_ATA_SECONDARY     HW_IRQ_ATA_SECONDARY

#define PIC_INT_OFFSET 32
#define PIC_MASTER     HW_PIC_MASTER
#define PIC_SLAVE      HW_PIC_SLAVE
#define PIC_EOI        HW_PIC_EOI

static inline void hw_irq_init(void) {
    hw_io_out8(HW_PIC_MASTER,     0x11);
    hw_io_out8(HW_PIC_MASTER + 1, 0x20);
    hw_io_out8(HW_PIC_MASTER + 1, 0x04);
    hw_io_out8(HW_PIC_MASTER + 1, 0x01);

    hw_io_out8(HW_PIC_SLAVE,     0x11);
    hw_io_out8(HW_PIC_SLAVE + 1, 0x28);
    hw_io_out8(HW_PIC_SLAVE + 1, 0x02);
    hw_io_out8(HW_PIC_SLAVE + 1, 0x01);

    hw_io_out8(HW_PIC_MASTER + 1, 0x00);
    hw_io_out8(HW_PIC_SLAVE + 1, 0x00);
}

static inline void hw_irq_eoi(uint8_t irq) {
    if (irq >= 8) {
        hw_io_out8(HW_PIC_SLAVE, HW_PIC_EOI);
    }
    hw_io_out8(HW_PIC_MASTER, HW_PIC_EOI);
}

static inline void pic_init(void) { hw_irq_init(); }
static inline void pic_sendEOI(uint8_t irq) { hw_irq_eoi(irq); }

#endif

/*===========================================================================
 * hw/timer — PIT (Programmable Interval Timer)
 *===========================================================================*/
#ifdef HW_TIMER_IPL

#define HW_PIT_BASE_FREQ   1193182
#define HW_PIT_CMD_PORT    0x43
#define HW_PIT_DATA_PORT   0x40

#define PIT_BASE_FREQ   HW_PIT_BASE_FREQ
#define PIT_TARGET_FREQ 100
#define PIT_DIVISOR     (PIT_BASE_FREQ/PIT_TARGET_FREQ)
#define PIT_CMD_PORT    HW_PIT_CMD_PORT
#define PIT_DATA_PORT   HW_PIT_DATA_PORT

void hw_timer_init(uint32_t freq_hz);
void pit_init(void);

#endif

/*===========================================================================
 * hw/idt — Interrupt Descriptor Table
 *===========================================================================*/
#ifdef HW_IDT_IPL

#include <types.h>

#define HW_IDT_SIZE 256
#define IDT_TAB_SIZE HW_IDT_SIZE

#define HW_INT_DIVZERO            0
#define HW_INT_INVALID_OPCODE     6
#define HW_INT_DOUBLE_FAULT       8
#define HW_INT_GEN_PROTECT_FAULT 13
#define HW_INT_PAGE_FAULT        14

#define INT_DIVZERO            HW_INT_DIVZERO
#define INT_INVALID_OPCODE     HW_INT_INVALID_OPCODE
#define INT_DOUBLE_FAULT       HW_INT_DOUBLE_FAULT
#define INT_GEN_PROTECT_FAULT  HW_INT_GEN_PROTECT_FAULT
#define INT_PAGE_FAULT         HW_INT_PAGE_FAULT

#define HW_INT_DISABLE() __asm__ volatile("cli")
#define HW_INT_ENABLE()  __asm__ volatile("sti")

#define INTERRUPTS_DISABLE() HW_INT_DISABLE()
#define INTERRUPTS_ENABLE()  HW_INT_ENABLE()

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
} __attribute__((packed)) idt_descriptor;

void hw_idt_init(void);
void idt_init(void);
void isr_handler(int num, struct interrupt_frame *frame);
uint32_t page_fault_handler(uint32_t *regs);

#define HW_ISR(num) \
    __attribute__((interrupt)) void hw_isr_##num(struct interrupt_frame *frame) { \
        isr_handler(num, frame); \
    }

#define HW_ISR_ERR(num) \
    __attribute__((interrupt)) void hw_isr_##num(struct interrupt_frame *frame, uintptr_t error_code) { \
        isr_handler(num, frame); \
    }

#define ISR(num) HW_ISR(num)
#define ISR_ERR(num) HW_ISR_ERR(num)

#endif

/*===========================================================================
 * Unified init — calls init for all included subsystems
 *===========================================================================*/
static inline void hw_init(void) {
#ifdef HW_IRQ_IPL
    hw_irq_init();
#endif
#ifdef HW_TIMER_IPL
    hw_timer_init(100);
#endif
#ifdef HW_IDT_IPL
    hw_idt_init();
#endif
}
