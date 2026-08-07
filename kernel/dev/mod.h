#pragma once

#include <stdint.h>
#include <stdarg.h>

/*===========================================================================
 * dev — device drivers public API
 *
 * selective includes: define DEV_<SUBMOD>_IPL before including
 * this header to pull in a specific subsystem.
 *
 *   #define DEV_CONSOLE_IPL
 *   #define DEV_BLOCK_IPL
 *   #define DEV_INPUT_IPL
 *   #define DEV_FB_IPL
 *   #include <dev/mod.h>
 *
 * or define DEV_ALL_IPL to include everything.
 *===========================================================================*/

#ifdef DEV_ALL_IPL
#  define DEV_CONSOLE_IPL
#  define DEV_BLOCK_IPL
#  define DEV_INPUT_IPL
#  define DEV_FB_IPL
#endif

/*===========================================================================
 * dev/console — Kernel debug output (serial)
 *===========================================================================*/
#ifdef DEV_CONSOLE_IPL
#  include "src/serial.h"

static inline void dev_console_init(void)              { serial_init(); }
static inline void dev_console_putc(char c)            { serial_putchar(c); }
static inline void dev_console_puts(const char *s)     { serial_puts(s); }
#define dev_console_printf(fmt, ...) serial_printf(fmt, ##__VA_ARGS__)
#endif

/*===========================================================================
 * dev/block — Block storage (ATA now, AHCI later)
 *===========================================================================*/
#ifdef DEV_BLOCK_IPL
#  include "src/ata.h"

static inline int dev_block_init(void) {
    uint16_t identify_buf[256];
    return ata_identify(identify_buf);
}

static inline int dev_block_read(uint32_t lba, uint8_t sectors, void *buf) {
    if (!buf) return -1;
    ata_read(lba, sectors, (uint16_t *)buf);
    return 0;
}

static inline int dev_block_write(uint32_t lba, uint8_t sectors, const void *buf) {
    if (!buf) return -1;
    ata_write(lba, sectors, (uint16_t *)buf);
    return 0;
}

static inline uint32_t dev_block_sector_size(void) {
    return ATA_SECTOR_SIZE;
}
#endif

/*===========================================================================
 * dev/input — Input devices (keyboard now, more later)
 *===========================================================================*/
#ifdef DEV_INPUT_IPL
#  include "src/kb.h"

static inline void dev_input_init(void) { }

static inline char dev_input_poll(void) {
    return 0;
}

static inline void dev_input_push(char c) {
    (void)c;
}
#endif

/*===========================================================================
 * dev/fb — Framebuffer (VESA now, other backends later)
 *===========================================================================*/
#ifdef DEV_FB_IPL
#  include "src/vesa.h"

static inline void     dev_fb_init(void)                              { vesa_init(); }
static inline uint32_t *dev_fb_ptr(void)                              { return vesa_get_fb(); }
static inline uint16_t dev_fb_width(void)                             { return VESA_WIDTH; }
static inline uint16_t dev_fb_height(void)                            { return VESA_HEIGHT; }
static inline uint16_t dev_fb_pitch(void)                             { return vesa_get_pitch(); }
static inline void     dev_fb_set_pixel(uint32_t x, uint32_t y, uint32_t c) { vesa_set_pixel(x, y, c); }
static inline void     dev_fb_clear(uint32_t c)                       { vesa_clear(c); }
#endif

/*===========================================================================
 * Unified init — calls init for all included subsystems
 *===========================================================================*/
static inline void dev_init(void) {
#ifdef DEV_CONSOLE_IPL
    serial_init();
#endif
#ifdef DEV_FB_IPL
    vesa_init();
#endif
}
