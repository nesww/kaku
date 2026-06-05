#include <proc/proc.h>
#include <proc/sched.h>
#include <tty/tty.h>
#include <hw/serial/serial.h>

#include "syscall.h"

static void __syscall_write(uint32_t *regs) {
    proc *current_proc = scheduler_get_current_proc();
    if (!current_proc) return;
    SERIAL_INFO("[pid=%x] syscall: SYS_WRITE\n", current_proc->proc_id);
    char *str =(char*)regs[4];
    if(!str) {
        TTY_WARN("SYS_WRITE: no string given to put\n");
        return;
    }
    tty_puts(str);
    tty_flush_current_line();
}

static void __syscall_exit(uint32_t *regs) {
    proc *current_proc = scheduler_get_current_proc();
    if (!current_proc) return;
    SERIAL_INFO("[pid=%x] syscall: SYS_EXIT\n", current_proc->proc_id);
    current_proc->proc_state = ZOMBIE;
}

void syscall_handler(uint32_t *regs) {
    uint32_t syscall_num = regs[7]; //from EAX

    switch(syscall_num) {
        case SYS_WRITE: __syscall_write(regs); break;
        case SYS_EXIT:  __syscall_exit(regs);  break;
        default: TTY_WARN("unknown syscall: %x\n", syscall_num);
    }
}
