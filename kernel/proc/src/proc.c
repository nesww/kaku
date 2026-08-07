#define LIB_MEM_IPL
#define LIB_ARRAY_IPL
#define MM_FRAME_IPL
#define MM_PAGING_IPL
#define FS_VFS_IPL
#define FS_FD_IPL
#define PROC_PROC_IPL
#define PROC_SCHED_IPL

#include <lib/mod.h>
#include <mm/mod.h>
#include <log/mod.h>
#include <panic/mod.h>
#include <proc/mod.h>
#include <fs/mod.h>
#include <dev/mod.h>
#include <proc/mod.h>

#include "proc.h"

static uint32_t pid_counter = 0;
static proc *__idle_proc = 0;

proc* proc_get_idle(void) {
    return __idle_proc;
}

proc *proc_create(void(*entry)() /*uint32_t priority*/) {
    if (pid_counter == 0) panic("tried to create a process before kernel idle process was created\n");

    proc *p = kmalloc(sizeof(proc));
    kmemset(p, 0, sizeof(proc));

    p->proc_id = pid_counter++;
    p->proc_pd = mm_paging_create_pd();

    uint32_t ks_phys = (uint32_t)mm_alloc_frame();
    uint32_t ks_virt = PHYS_TO_VIRT(ks_phys) + 4096;

    proc_registers_state ctx = {0};
    ctx.eip      = (uint32_t)entry;
    ctx.cs       = 0x1B;
    ctx.eflags   = 0x202;
    ctx.user_esp = PROC_USER_STACK_TOP_VADDR;
    ctx.user_ss  = 0x23;

    p->kernel_stack = (uint32_t)kmemcpy((void*)(ks_virt - sizeof(proc_registers_state)), &ctx, sizeof(proc_registers_state));
    for (uint32_t i = 0; i < 4; ++i) {
        uint32_t frame = (uint32_t)mm_alloc_frame();
        uint32_t vaddr = PROC_USER_STACK_TOP_VADDR - (i+1) * 4096;
        mm_paging_map(p->proc_pd, frame, vaddr, PROC_USER_FLAGS);
    }

    p->user_stack = PROC_USER_STACK_TOP_VADDR;
    p->argc       = 0;
    p->argv       = 0;
    p->proc_state = READY;

    // if a process is being created, the one currently executing will be its parent (syscall only)
    // else, it's the first process of the process tree, therefore, no parent process
    proc *current = scheduler_get_current_proc();
    p->parent_pid = current ? current->proc_id : 0;
    p->exit_code = 0;
    p->waiting_for = 0;

    return p;
}
extern uint8_t idle_stack_top;
extern void idle_entry(void);

proc *proc_create_k_idle() {
    proc *p = kmalloc(sizeof(proc));
    kmemset(p, 0, sizeof(proc));
    p->proc_id = pid_counter++;
    p->proc_pd = (page_directory*)mm_paging_get_kernel_pd();
    uint32_t ks_top = (uint32_t)&idle_stack_top;
    proc_registers_state ctx = {0};
    ctx.eip      = (uint32_t)idle_entry;
    ctx.cs       = 0x08;
    ctx.eflags   = 0x202;
    ctx.user_esp = ks_top;
    ctx.user_ss  = 0x10;
    p->kernel_stack = (uint32_t)kmemcpy(
        (void*)(ks_top - sizeof(proc_registers_state)),
        &ctx,
        sizeof(proc_registers_state));
    p->proc_state = READY;
    __idle_proc = p;
    return p;
}

void proc_destroy(proc *p) {
    mm_free_frame(VIRT_TO_PHYS(p->kernel_stack + sizeof(proc_registers_state) - 4096));

    for (uint32_t pd_index = 0; pd_index < 768; ++pd_index) {
        uint32_t pde = p->proc_pd->entries[pd_index];
        if (!(pde & 0x1)) continue;

        page_table *pt = (page_table*)PHYS_TO_VIRT(pde & 0xFFFFF000);
        for (uint32_t pt_index = 0; pt_index < 1024; ++pt_index) {
            uint32_t pte = pt->entries[pt_index];
            if (pte & 0x1) {
                mm_free_frame(pte & 0xFFFFF000);
            }
        }
        mm_free_frame(pde & 0xFFFFF000);
    }

    mm_free_frame(VIRT_TO_PHYS((uint32_t)p->proc_pd));
    if (p->mmap_regions) {
        dynarray *regions = (dynarray*)p->mmap_regions;
        for (uint32_t i = 0; i < regions->count; ++i) {
            kfree(dynarray_get(regions, i));
        }
        dynarray_free(regions);
    }
    if (p->argv) {
        for (uint32_t i = 0; i < p->argc; ++i) {
            kfree(p->argv[i]);
        }
        kfree(p->argv);
        p->argv = 0;
    }
    if (p->env) {
        for (uint32_t i = 0; i < p->envc; ++i) {
            kfree(p->env[i]);
        }
        kfree(p->env);
        p->env = 0;
    }
    kfree(p);
}
int proc_alloc_fd(proc *p, vfs_node *node, uint8_t flags) {
    for (uint32_t i = 3; i < MAX_FDS; ++i) {
        if (p->fds[i] == 0) {
            int entry_index = fd_find_by_node(node);
            if (entry_index == -1) {
                entry_index = fd_new(node, flags);
            }
            fd_get(entry_index)->ref_count++;
            proc_fd_entry *pfe = kmalloc(sizeof(proc_fd_entry));
            pfe->offset = 0;
            pfe->global_fd = entry_index;
            pfe->flags = flags;
            p->fds[i] = pfe;
            return i;
        }
    }
    return -1;
}

int proc_fd_read_file(proc *p, uint32_t fd, uint8_t *buf, uint32_t buf_len, uint32_t *sc_ret) {
    proc_fd_entry *pfe = p->fds[fd];
    if (!pfe) {
        SERIAL_ERROR("PID%x: given fd(%x) did not exist or is not open\n", p->proc_id, fd);
        *sc_ret = 2;
        return 0;
    }
    if (!(pfe->flags & FD_FLAGS_RO) && !(pfe->flags & FD_FLAGS_RW)) {
        SERIAL_ERROR("PID%x: given fd(%x) does not have read flags (its flag are: %x)\n", p->proc_id, fd, pfe->flags);
        *sc_ret = 2;
        return 0;
    }
    fd_table_entry *fte = fd_get(pfe->global_fd);
    *sc_ret = 0;
    return vfs_read(fte->node, buf, buf_len);
}


int proc_fd_write_file(proc *p, uint32_t fd, uint8_t *buf, uint32_t buf_len, uint32_t *sc_ret) {
    proc_fd_entry *pfe = p->fds[fd];
    if (!pfe) {
        SERIAL_ERROR("PID%x: given fd(%x) did not exist or is not open\n", p->proc_id, fd);
        *sc_ret = 2;
        return 0;
    }
    if (!(pfe->flags & FD_FLAGS_WO) && !(pfe->flags & FD_FLAGS_RW)) {
        SERIAL_ERROR("PID%x: given fd(%x) does not have write flags\n", p->proc_id);
        *sc_ret = 2;
        return 0;
    }
    fd_table_entry *fte = fd_get(pfe->global_fd);
    *sc_ret = 0;
    return vfs_write(fte->node, buf, buf_len);
}

void proc_close_fd(proc *p, uint32_t fd) {
    fd_table_entry *entry = fd_get(p->fds[fd]->global_fd);
    if (!entry) {
        SERIAL_WARN("tried to close fd for proc PID:%x when no corresponding global fd entry existed for fd: %x\n", p->proc_id, fd);
        return;
    };

    fd_remove(p->fds[fd]->global_fd);
    kfree(p->fds[fd]);
    p->fds[fd] = 0;
}
