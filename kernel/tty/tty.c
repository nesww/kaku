#include <hw/vesa/vesa.h>
#include <fonts/default8x16.h>
#include <log/log.h>
#include <lib/print.h>
#include <lib/stdmem.h>

#include "tty.h"

#define TTY_CHAR_WIDTH  8
#define TTY_CHAR_HEIGHT 16

#define TTY_BACKBUFFER_SIZE ((VESA_WIDTH / TTY_CHAR_WIDTH) * (VESA_HEIGHT / TTY_CHAR_HEIGHT))

static tty_char tty_backbuffer[TTY_BACKBUFFER_SIZE] = {0};
static uint32_t tty_cursor_x = 0;
static uint32_t tty_cursor_y = 0;
static uint32_t tty_fg = 0x00FFFFFF;
static uint32_t tty_bg = 0x00000000;


static void __tty_flush_char(tty_char *tc, uint32_t px, uint32_t py);
static void __tty_flush_line(uint32_t line);
static void __tty_newline(void);
static void __tty_scroll(void);

static void __tty_newline(void) {
    tty_cursor_x = 0;
    __tty_flush_line(tty_cursor_y);
    tty_cursor_y++;
    if (tty_cursor_y >= VESA_HEIGHT / TTY_CHAR_HEIGHT)
        __tty_scroll();
}

static void __tty_log_sink(const char *str) {
    tty_puts(str);
    tty_flush();
}

void tty_init(void) {
    tty_cursor_x = 0;
    tty_cursor_y = 0;
    kmemset(tty_backbuffer, 0, sizeof(tty_backbuffer));
    log_register_sink(__tty_log_sink);
}

void tty_putchar(char c) {
    if (c == '\n') {
        __tty_newline();
        return;
    }
    uint32_t index = tty_cursor_y * (VESA_WIDTH / TTY_CHAR_WIDTH) + tty_cursor_x;
    tty_backbuffer[index].c = c;
    tty_backbuffer[index].fg = tty_fg;
    tty_backbuffer[index].bg = tty_bg;
    tty_cursor_x++;
    if (tty_cursor_x >= VESA_WIDTH / TTY_CHAR_WIDTH) {
        tty_cursor_x = 0;
        tty_cursor_y++;
        if (tty_cursor_y >= VESA_HEIGHT / TTY_CHAR_HEIGHT)
            __tty_scroll();
    }
}

void tty_puts(const char *s) {
    while (*s) tty_putchar(*s++);
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


static void __tty_flush_char(tty_char *tc, uint32_t px, uint32_t py) {
    uint8_t *glyph = default8x16_psf + 6 + (tc->c * TTY_CHAR_HEIGHT);
    for (uint32_t row = 0; row < TTY_CHAR_HEIGHT; ++row) {
        for (uint32_t col = 0; col < TTY_CHAR_WIDTH; ++col) {
            uint32_t color = (glyph[row] & (0x80 >> col)) ? tc->fg : tc->bg;
            vesa_set_pixel(px + col, py + row, color);
        }
    }
}

static void __tty_flush_cell(uint32_t i) {
    tty_char tc = tty_backbuffer[i];
    uint32_t cols = VESA_WIDTH / TTY_CHAR_WIDTH;
    uint32_t t_col = i % cols;
    uint32_t t_row = i / cols;
    uint32_t px = t_col * TTY_CHAR_WIDTH;
    uint32_t py = t_row * TTY_CHAR_HEIGHT;
    if (tc.c == 0) {
        for (uint32_t row = 0; row < TTY_CHAR_HEIGHT; ++row)
            for (uint32_t col = 0; col < TTY_CHAR_WIDTH; ++col)
                vesa_set_pixel(px + col, py + row, tty_bg);
        return;
    }
    __tty_flush_char(&tc, px, py);
}

static void __tty_flush_line(uint32_t line) {
    uint32_t cols = VESA_WIDTH / TTY_CHAR_WIDTH;
    for (uint32_t i = line * cols; i < (line + 1) * cols; ++i)
        __tty_flush_cell(i);
}


void tty_flush() {
    for (uint32_t i = 0; i < TTY_BACKBUFFER_SIZE; ++i)
        __tty_flush_cell(i);
}

void tty_flush_current_line(void) {
    __tty_flush_line(tty_cursor_y);
}

static void __tty_scroll(void) {
    uint32_t cols = VESA_WIDTH / TTY_CHAR_WIDTH;
    uint32_t rows = VESA_HEIGHT / TTY_CHAR_HEIGHT;
    kmemcpy(tty_backbuffer, tty_backbuffer + cols, (TTY_BACKBUFFER_SIZE - cols) * sizeof(tty_char));
    kmemset(tty_backbuffer + (rows - 1) * cols, 0, cols * sizeof(tty_char));
    tty_cursor_y = rows - 1;
    tty_flush();
}

void tty_clear(void) {
    tty_cursor_x = 0;
    tty_cursor_y = 0;
    kmemset(tty_backbuffer, 0, sizeof(tty_backbuffer));
    vesa_clear(tty_bg);
}

void tty_set_fg(uint32_t color) { tty_fg = color; }
void tty_set_bg(uint32_t color) { tty_bg = color; }
