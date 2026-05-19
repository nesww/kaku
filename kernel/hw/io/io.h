#ifndef IO_H
#define IO_H

#include <stdint.h>

#define IO_KEYBOARD_DATA_PORT 0x60

#define IO_VGA_INDEX_PORT 0x3D4
#define IO_VGA_DATA_PORT  0x3D5
#define IO_VGA_DATA_POS_HIGH_BITS 0x0E
#define IO_VGA_DATA_POS_LOW_BITS  0x0F

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value): "Nd"(port));
    return value;
}
#endif
