#include <proc/proc.h>
#include <proc/sched.h>
#include <tty/tty.h>
#include <hw/serial/serial.h>
#include <io/stdin.h>
#include <panic/panic.h>
#include <vfs/vfs.h>

#include "syscall.h"

#define EBX(regs) (regs[4])
#define ECX(regs) (regs[6])
#define EDX(regs) (regs[5])
#define EAX(regs) (regs[7])

static uint32_t __syscall_write(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    char *str =(char*)regs[4];
    if(!str) {
        TTY_WARN("SYS_WRITE: no string given to put\n");
        return 1;
    }
    tty_puts(str);
    tty_flush_current_line();
    return 0;
}

static uint32_t __syscall_read(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    char *read_buf_addr    = (char*)EBX(regs);
    uint32_t read_buf_size = (uint32_t)ECX(regs);

    p->proc_state          = BLOCKED;
    p->stdin_user_buf      = read_buf_addr;
    p->stdin_user_buf_size = read_buf_size;
    stdin_wait(p, read_buf_addr, read_buf_size);
    return 0;
}

static uint32_t __syscall_exit(proc* p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    p->proc_state = ZOMBIE;
    return 0;
}

static uint32_t __syscall_open(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    const char *file_path = (const char*)EBX(regs);
    uint8_t flags         = (uint8_t)ECX(regs);

    SERIAL_INFO("file_path: %s | flags: %x\n", file_path, flags);
    vfs_node *file = vfs_open(file_path);
    if (!file) {
        SERIAL_WARN("file not found: %s\n", file_path);
        EAX(regs) = -1;
        return 1;
    };

    EAX(regs) = proc_alloc_fd(p, file, flags);
    return 0;
}

static uint32_t __syscall_close(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    uint32_t fd = (uint32_t)EBX(regs);
    if (fd < 3) {
        SERIAL_WARN("tried closing fd less than 3: %d\n", fd);
        return 1;
    }

    proc_close_fd(p, fd);
    return 0;
}

static uint32_t __syscall_read_file(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    uint32_t fd      = (uint32_t)EBX(regs);
    uint8_t *buf     = (uint8_t*)ECX(regs);
    uint32_t buf_len = (uint32_t)EDX(regs);

    uint32_t ret;
    EAX(regs)= proc_fd_read_file(p, fd, buf, buf_len, &ret);
    return ret;
}

static uint32_t __syscall_write_file(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    uint32_t fd      = (uint32_t)EBX(regs);
    uint8_t *buf     = (uint8_t*)ECX(regs);
    uint32_t buf_len = (uint32_t)EDX(regs);

    uint32_t ret;
    EAX(regs) = proc_fd_write_file(p, fd, buf, buf_len, &ret);
    return ret;
}

uint32_t syscall_handler(uint32_t *regs) {
    uint32_t syscall_num = EAX(regs);
    proc* current_proc = scheduler_get_current_proc();
    if (!current_proc) {
        kernel_panic("had a syscall when no process was running");
    }

    serial_printf("\n\n");

    uint32_t ret = 0;

    switch(syscall_num) {
        case SYS_WRITE: ret = __syscall_write(current_proc, regs); break;
        case SYS_READ:  ret = __syscall_read(current_proc, regs);  break;
        case SYS_OPEN:  ret = __syscall_open(current_proc, regs);  break;
        case SYS_CLOSE: ret = __syscall_close(current_proc, regs); break;

        case SYS_READ_FILE:  ret = __syscall_read_file(current_proc, regs);  break;
        case SYS_WRITE_FILE: ret = __syscall_write_file(current_proc, regs); break;

        case SYS_EXIT:  ret = __syscall_exit(current_proc, regs);  break;

        default: TTY_WARN("unknown syscall: %x\n", syscall_num);
    }
    if (current_proc && current_proc->proc_state == BLOCKED) {
        return scheduler_yield(regs);
    }
    switch (ret) {
        case 1:  SERIAL_WARN("something went somewhat wrong during syscall handling"); break;
        case 2:  SERIAL_WARN("an error happened during syscall handling"); break;
        default: SERIAL_INFO("handle complete, will continue\n");
    }
    return 0;
}
