#include "syscall.h"
#include "tty/tty.h"
#include "hw/serial/serial.h"

void syscall_handler(uint32_t *regs) {
    uint32_t syscall_num = regs[7]; //from EAX
    TTY_INFO("syscall! %x:\n", syscall_num);
    SERIAL_INFO("syscall! %x:\n", syscall_num);
}
