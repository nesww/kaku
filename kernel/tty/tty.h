#ifndef TTY_H
#define TTY_H
#include <stdint.h>
#include <stdarg.h>

#define TTY_INFO(str, ...) \
    tty_printf("[%s] <info>: " str, __func__, ##__VA_ARGS__);
#define TTY_WARN(str, ...) \
    tty_printf("[%s] <warn>: " str, __func__, ##__VA_ARGS__);
#define TTY_ERROR(str, ...) \
    tty_printf("[%s] <error>: " str, __func__, ##__VA_ARGS__);
#define TTY_KERNEL(str, ...) \
    tty_printf("<kernel>: " str, ##__VA_ARGS__);

typedef struct {
    char c;
    uint32_t fg;
    uint32_t bg;
} tty_char;

void tty_init(void);
void tty_putchar(char c);
void tty_puts(const char *s);
void tty_printf(const char *s, ...);
void tty_vprintf(const char *s, va_list args);
void tty_flush();
void tty_flush_current_line(void);
void tty_clear(void);
void tty_set_fg(uint32_t color);
void tty_set_bg(uint32_t color);

#endif
