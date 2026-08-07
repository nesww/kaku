#include <stdint.h>

#define LIB_ALL_IPL
#define MM_ALL_IPL
#define PROC_ALL_IPL
#define DEV_CONSOLE_IPL
#define TTY_TTY_IPL

#include <lib/mod.h>
#include <proc/mod.h>
#include <dev/mod.h>
#include <tty/mod.h>

#include "stdin.h"

typedef struct {
    proc *p;
    char* buf;
    uint32_t buf_size;
} stdin_waiting_proc;

static uint8_t *__stdin_buf;
static uint32_t __stdin_buf_len = 0;
static stdin_waiting_proc __waiting_stdin;

#define STDIN_BUF_MAX 512

void stdin_init() {
    __stdin_buf = kmalloc(STDIN_BUF_MAX);
}

void stdin_wait(proc* p, char* buf, uint32_t size) {
    __waiting_stdin = (stdin_waiting_proc){
        .p = p,
        .buf = buf,
        .buf_size = size
    };
}

void stdin_put(char c) {
    SERIAL_INFO("stdin char: %x\n", c);
    if(!__stdin_buf) return;
    if (__stdin_buf_len + 1 >= STDIN_BUF_MAX) return;
    __stdin_buf[__stdin_buf_len++] = c;
    tty_putchar(c);
    tty_flush_current_line();
}

void stdin_pop(void) {
    SERIAL_INFO("stdin backspace\n");
    if(!__stdin_buf) return;
    if (__stdin_buf_len == 0) return;
    __stdin_buf_len--;
    tty_backspace();
}

void stdin_flush(void) {
    SERIAL_INFO("stdin is trying to get flushed\n");
    if(!__stdin_buf || !__waiting_stdin.p) {
        if (!__stdin_buf) {
            SERIAL_INFO("no stdin buf!\n");
        }
        if (!__waiting_stdin.p) {
            SERIAL_INFO("no __waiting_stdin.p!\n");
        }
        return;
    }
    scheduler_wake_stdin(
        __waiting_stdin.p,
        __waiting_stdin.buf,
        __waiting_stdin.buf_size,
        __stdin_buf,
        __stdin_buf_len);
    __stdin_buf_len = 0;
    __waiting_stdin.p = 0;
    SERIAL_INFO("stdin must have been flushed\n");
}
