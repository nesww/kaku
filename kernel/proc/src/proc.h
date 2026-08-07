#pragma once

#include <types.h>

#define PROC_USER_STACK_TOP_VADDR 0x8000000
#define PROC_USER_FLAGS 0x7 // present | writable | user

#define MAX_FDS 16

typedef enum {
    RUNNING, READY, BLOCKED, ZOMBIE,
    STARTING // when the process was not yet taken by the scheduler (args for the process still in kernel memory)
} proc_state;

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    //
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t user_ss;


} __attribute__((packed)) proc_registers_state;

typedef struct {
    uint32_t global_fd; //global fd_table index
    uint32_t offset;
    uint8_t  flags;
} proc_fd_entry;

typedef struct {
    uint32_t             proc_id;
    proc_registers_state reg_states;
    uint32_t             user_stack;
    uint32_t             kernel_stack;
    page_directory *     proc_pd;
    proc_state           proc_state;

    char *   stdin_user_buf;
    uint32_t stdin_user_buf_size;
    uint8_t *stdin_kernel_buf;
    uint32_t stdin_kernel_buf_len;

    uint32_t argc;
    char **argv;

    uint32_t envc;
    char **env;

    uint32_t program_break;
    uint32_t mmap_next;
    void *mmap_regions;

    uint32_t parent_pid;
    uint32_t exit_code;
    uint32_t waiting_for;

    proc_fd_entry *fds[MAX_FDS];
} proc;

proc *proc_create(void(*entry)() /*uint32_t priority*/);
proc *proc_create_k_idle( /*uint32_t priority*/);
void proc_destroy(proc *p);
proc* proc_get_idle(void);

int proc_alloc_fd(proc *p, vfs_node *node, uint8_t flags);
int proc_fd_read_file(proc *p, uint32_t fd, uint8_t *buf, uint32_t buf_len, uint32_t *sc_ret);
int proc_fd_write_file(proc *p, uint32_t fd, uint8_t *buf, uint32_t buf_len, uint32_t *sc_ret);
void proc_close_fd(proc *p, uint32_t fd);
