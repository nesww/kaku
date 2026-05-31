#ifndef VGA_H
#define VGA_H

#include <stdarg.h>
#include <stdint.h>

#define VGA_ADDRESS 0xB8000
#define VGA_COLS    80
#define VGA_ROWS    25

#define VGA_BLACK         0x0
#define VGA_BLUE          0x1
#define VGA_GREEN         0x2
#define VGA_CYAN          0x3
#define VGA_RED           0x4
#define VGA_MAGENTA       0x5
#define VGA_BROWN         0x6
#define VGA_GRAY          0x7
#define VGA_DARK_GRAY     0x8
#define VGA_LIGHT_BLUE    0x9
#define VGA_LIGHT_GREEN   0xa
#define VGA_LIGHT_CYAN    0xb
#define VGA_LIGHT_RED     0xc
#define VGA_LIGHT_MAGENTA 0xd
#define VGA_YELLOW        0xe
#define VGA_WHITE         0xf

void vga_enable_cursor(void);
void vga_clear();
void vga_cclear(uint8_t color);
void vga_print(const char *str);
void vga_println(const char *str);
void vga_putchar(char c);
void vga_putint(int num);
void vga_puthex(uint8_t num);
void vga_puthex32(uint32_t num);
void vga_printf(const char *str, ...);
void vga_vprintf(const char *str, va_list args);
void vga_backspace(void);

/* only 4 first bits will be used, */
void vga_setforeground_color(uint8_t fg);
/* only 4 first bits will be used */
void vga_setbackground_color(uint8_t bg);

#endif
