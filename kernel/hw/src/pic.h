#pragma once

#include <stdint.h>
#include "io.h"

#define PIC_INT_OFFSET 32 /* offset for interruptions */

#define PIC_MASTER 0x20  //base address for IO master
#define PIC_SLAVE  0xa0  //base address for IO slave
#define PIC_MASTER_CMD  (PIC_MASTER)
#define PIC_MASTER_DATA (PIC_MASTER + 1)
#define PIC_SLAVE_CMD   (PIC_SLAVE)
#define PIC_SLAVE_DATA  (PIC_SLAVE + 1)

#define PIC_EOI 0x20 //end of interrupt command


#define INT_PIC_TIMER             32 // IRQ0
#define INT_PIC_KEYBOARD          33 // IRQ1
#define INT_PIC_SERIAL_COM2       35 // IRQ3
#define INT_PIC_SERIAL_COM1       36 // IRQ4
#define INT_PIC_PARALLEL_PORT     37 // IRQ5
#define INT_PIC_FLOPPY            38 // IRQ6
#define INT_PIC_PARALLEL_PORT2    39 // IRQ7

#define INT_PIC_REALTIME_CLOCK    40 // IRQ8
#define INT_PIC_PS2               44 // IRQ12
#define INT_PIC_FLOAT_COPROCESSOR 45 // IRQ13
#define INT_PIC_ATA_PRIMARY       46 // IRQ14
#define INT_PIC_ATA_SECONDARY     47 // IRQ15


static inline void pic_sendEOI(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC_SLAVE_CMD, PIC_EOI);
    }
    outb(PIC_MASTER_CMD, PIC_EOI);
}

static inline void pic_init() {
    outb(PIC_MASTER_CMD,  0x11); // ICW1 to configure PIC
    outb(PIC_MASTER_DATA, 0x20); // interrupts start at 32
    outb(PIC_MASTER_DATA, 0x04); // IRQ2 = slave PIC (= 0000 0100 = 2nd IRQ, good job Intel >:())
    outb(PIC_MASTER_DATA, 0x01);

    outb(PIC_SLAVE_CMD,  0x11); // config PIC
    outb(PIC_SLAVE_DATA, 0x28); // interrupts starts at 40
    outb(PIC_SLAVE_DATA, 0x02); // IRQ2 = slave
    outb(PIC_SLAVE_DATA, 0x01);

    //demasking IRQs
    outb(PIC_MASTER_DATA, 0x00);
    outb(PIC_SLAVE_DATA, 0x00);
}
