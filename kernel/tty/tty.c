#include "tty.h"
#include "hw/vesa/vesa.h"
#include "fonts/default8x16.h"
#include "lib/print.h"

#define TTY_CHAR_WIDTH  8
#define TTY_CHAR_HEIGHT 16

static uint32_t tty_cursor_x = 0;
static uint32_t tty_cursor_y = 0;
static uint32_t tty_fg = 0x00FFFFFF;
static uint32_t tty_bg = 0x00000000;

void tty_init(void) {
    tty_cursor_x = 0;
    tty_cursor_y = 0;
}

void tty_putchar(char c) {
    if (c == '\n') {
        tty_cursor_x = 0;
        tty_cursor_y += TTY_CHAR_HEIGHT;
        return;
    }
    uint8_t *glyph = default8x16_psf + 6 + (c * TTY_CHAR_HEIGHT);
    uint32_t *fb = vesa_get_fb();
    uint32_t pitch = vesa_get_pitch();
    for (uint32_t row = 0; row < TTY_CHAR_HEIGHT; row++) {
        for (uint32_t col = 0; col < TTY_CHAR_WIDTH; col++) {
            uint32_t color = (glyph[row] & (0x80 >> col)) ? tty_fg : tty_bg;
            fb[(tty_cursor_y + row) * pitch + (tty_cursor_x + col)] = color;
        }
    }
    tty_cursor_x += TTY_CHAR_WIDTH;
    if (tty_cursor_x >= VESA_WIDTH) {
        tty_cursor_x = 0;
        tty_cursor_y += TTY_CHAR_HEIGHT;
    }
}

void tty_puts(const char *s) {
    while (*s) tty_putchar(*s++);
}

static void __tty_newline(void) {
    tty_cursor_x = 0;
    tty_cursor_y += TTY_CHAR_HEIGHT;
}

void tty_printf(const char *s, ...) {
    va_list args;
    va_start(args, s);
    kvprintf_to(tty_putchar, __tty_newline, s, args);
    va_end(args);
}

void tty_vprintf(const char *s, va_list args) {
    kvprintf_to(tty_putchar, __tty_newline, s, args);
}

void tty_clear(void) {
    tty_cursor_x = 0;
    tty_cursor_y = 0;
    vesa_clear(tty_bg);
}

void tty_set_fg(uint32_t color) { tty_fg = color; }
void tty_set_bg(uint32_t color) { tty_bg = color; }
