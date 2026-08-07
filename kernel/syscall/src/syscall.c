#define PROC_ALL_IPL
#define TTY_ALL_IPL
#define DEV_CONSOLE_IPL
#define FS_ALL_IPL
#define MM_FRAME_IPL
#define MM_PAGING_IPL
#define LIB_ARRAY_IPL
#define LIB_MEM_IPL

#include <proc/mod.h>
#include <tty/mod.h>
#include <dev/mod.h>
#include <panic/mod.h>
#include <fs/mod.h>
#include <elf/mod.h>
#include <mm/mod.h>
#include <lib/mod.h>

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

    SERIAL_INFO("with string: %x\n", str);

    uint32_t start_y = tty_get_cursor_y();
    tty_puts(str);
    tty_flush_from(start_y);
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
    p->exit_code = EBX(regs);

    scheduler_notify_parent(p);
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

static uint32_t __syscall_exec(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    uint8_t *path = (uint8_t*)EBX(regs);
    char **argv   = (char **)ECX(regs);
    char **envp   = (char **)EDX(regs);

    SERIAL_INFO("argv addr: %x, argv[0]: %x\n", (uint32_t)argv, argv ? (uint32_t)argv[0] : 0);

    int pid   = elf_load((const char*)path, argv, envp);
    EAX(regs) = pid;
    return pid != -1 ? 0 : 1;
}

static uint32_t __syscall_kill(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    uint32_t pid = (uint32_t)EBX(regs);
    proc *p_to_kill = sched_get_proc(pid);
    SERIAL_INFO("will kill pid%x\n", pid);
    p_to_kill->proc_state = ZOMBIE;
    return 0;
}

static uint32_t __syscall_waitpid(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    uint32_t child_pid = EBX(regs);
    proc *child = sched_get_proc(child_pid);

    if (!child) {
        SERIAL_WARN("waitpid: child pid=%x not found\n", child_pid);
        EAX(regs) = -1;
        return 1;
    }

    if (child->parent_pid != p->proc_id) {
        SERIAL_WARN("waitpid: pid=%x is not parent of pid=%x\n", p->proc_id, child_pid);
        EAX(regs) = -2;
        return 1;
    }

    if (child->proc_state == ZOMBIE) {
        int exit_code = sched_reap_child(child_pid);
        EAX(regs) = exit_code;
        return 0;
    }

    p->proc_state = BLOCKED;
    p->waiting_for = child_pid;
}

typedef struct {
    uint32_t vnode_id;
    uint32_t size;
    uint8_t  is_dir;
    char     name[256];
} dir_entry;

static uint32_t __syscall_readdir(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    char *path       = (char*)EBX(regs);
    char *buf        = (char*)ECX(regs);
    uint32_t buf_len = EDX(regs);

    vfs_node *dir = vfs_open(path);
    if (!dir || !dir->is_dir) {
        SERIAL_WARN("readdir: path not found or not a directory\n");
        EAX(regs) = -1;
        return 1;
    }

    dynarray *entries = vfs_list(dir);
    if (!entries) {
        SERIAL_WARN("readdir: could not list directory\n");
        EAX(regs) = -1;
        vfs_close(dir);
        return 1;
    }

    uint32_t max_entries = buf_len / sizeof(dir_entry);
    uint32_t written = 0;
    for (uint32_t i = 0; i < entries->count && written < max_entries; i++) {
        fs_ext2_entry *entry = dynarray_get(entries, i);
        dir_entry *out = (dir_entry*)(buf + written * sizeof(dir_entry));

        uint32_t name_len = entry->entry_name_len;
        if (name_len >= sizeof(out->name)) name_len = sizeof(out->name) - 1;
        kmemcpy(out->name, entry->entry_name, name_len);
        out->name[name_len] = '\0';

        out->vnode_id = entry->entry_inode;
        out->is_dir   = (entry->entry_type == 2) ? 1 : 0;

        fs_ext2_inode ino = fs_ext2_read_inode(entry->entry_inode);
        out->size = ino.inode_size_low_bits;

        written++;
    }

    dynarray_free_deep(entries, kfree);
    vfs_close(dir);

    EAX(regs) = written;
    return 0;
}

typedef struct {
    uint32_t base;
    uint32_t len;
} mmap_region;

static void __syscall_unmap_pages(proc *p, uint32_t vaddr, uint32_t pages) {
    page_directory *pd = p->proc_pd;
    for (uint32_t i = 0; i < pages; ++i) {
        uint32_t addr = vaddr + i * 4096;
        uint32_t pd_idx = addr >> 22;
        uint32_t pt_idx = (addr >> 12) & 0x3FF;
        pd_entry pde = pd->entries[pd_idx];
        if (!(pde & 0x1)) continue;
        page_table *pt = (page_table*)PHYS_TO_VIRT(pde & 0xFFFFF000);
        pt_entry pte = pt->entries[pt_idx];
        if (pte & 0x1) {
            mm_free_frame(pte & 0xFFFFF000);
            pt->entries[pt_idx] = 0;
        }
    }
}

static uint32_t __syscall_sbrk(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    int32_t increment  = (int32_t)EBX(regs);
    uint32_t old_break = p->program_break;
    uint32_t new_break;
    if (increment < 0) {
        uint32_t dec = (uint32_t)(-increment);
        new_break = dec > old_break ? 0 : old_break - dec;
    } else {
        new_break = old_break + (uint32_t)increment;
    }

    uint32_t old_page = (old_break + 0xFFF) & ~0xFFFu;
    uint32_t new_page = (new_break + 0xFFF) & ~0xFFFu;

    if (new_page > old_page) {
        for (uint32_t addr = old_page; addr < new_page; addr += 4096) {
            uint32_t frame = (uint32_t)mm_alloc_frame();
            mm_paging_map(p->proc_pd, frame, addr, PROC_USER_FLAGS);
        }
    } else if (new_page < old_page) {
        __syscall_unmap_pages(p, new_page, (old_page - new_page) / 4096);
    }

    p->program_break = new_break;
    EAX(regs) = old_break;
    return 0;
}

static uint32_t __syscall_mmap(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    uint32_t size  = EBX(regs);
    uint32_t pages = (size + 0xFFF) >> 12;
    uint32_t base  = p->mmap_next;

    for (uint32_t i = 0; i < pages; ++i) {
        uint32_t frame = (uint32_t)mm_alloc_frame();
        mm_paging_map(p->proc_pd, frame, base + i * 4096, PROC_USER_FLAGS);
    }

    if (!p->mmap_regions) {
        p->mmap_regions = dynarray_new(4);
    }
    mmap_region *r = kmalloc(sizeof(mmap_region));
    r->base = base;
    r->len  = pages * 4096;
    dynarray_add((dynarray*)p->mmap_regions, r);

    p->mmap_next = base + pages * 4096;
    EAX(regs) = base;
    return 0;
}

static uint32_t __syscall_munmap(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    uint32_t addr = EBX(regs);
    if (!p->mmap_regions) {
        EAX(regs) = -1;
        return 1;
    }

    dynarray *regions = (dynarray*)p->mmap_regions;
    for (uint32_t i = 0; i < regions->count; ++i) {
        mmap_region *r = dynarray_get(regions, i);
        if (r->base == addr) {
            __syscall_unmap_pages(p, addr, r->len / 4096);
            kfree(r);
            dynarray_remove(regions, i);
            EAX(regs) = 0;
            return 0;
        }
    }
    EAX(regs) = -1;
    return 1;
}

static uint32_t __syscall_getpid(proc *p, uint32_t *regs) {
    SERIAL_INFO("[pid=%x]\n", p->proc_id);

    EAX(regs) = p->proc_id;
    return 0;
}

uint32_t syscall_handler(uint32_t *regs) {
    uint32_t syscall_num = EAX(regs);
    proc* current_proc = sched_get_current();
    if (!current_proc) {
        panic("had a syscall when no process was running");
    }

    serial_printf("\n");

    uint32_t ret = 0;

    switch(syscall_num) {
        case SYS_WRITE: ret = __syscall_write(current_proc, regs); break;
        case SYS_READ:  ret = __syscall_read(current_proc, regs);  break;
        case SYS_OPEN:  ret = __syscall_open(current_proc, regs);  break;
        case SYS_CLOSE: ret = __syscall_close(current_proc, regs); break;

        case SYS_READ_FILE:  ret = __syscall_read_file(current_proc, regs);  break;
        case SYS_WRITE_FILE: ret = __syscall_write_file(current_proc, regs); break;

        case SYS_EXIT:  ret = __syscall_exit(current_proc, regs);  break;

        case SYS_EXEC: ret = __syscall_exec(current_proc, regs); break;
        case SYS_KILL: ret = __syscall_kill(current_proc, regs); break;

        case SYS_WAITPID: ret = __syscall_waitpid(current_proc, regs); break;
        case SYS_READDIR: ret = __syscall_readdir(current_proc, regs); break;

        case SYS_SBRK:   ret = __syscall_sbrk(current_proc, regs);   break;
        case SYS_MMAP:   ret = __syscall_mmap(current_proc, regs);   break;
        case SYS_MUNMAP: ret = __syscall_munmap(current_proc, regs); break;

        case SYS_GETPID: ret = __syscall_getpid(current_proc, regs); break;

        default: TTY_WARN("unknown syscall: %x\n", syscall_num);
    }
    if (current_proc && current_proc->proc_state == BLOCKED || current_proc->proc_state == ZOMBIE) {
        return scheduler_yield(regs);
    }
    switch (ret) {
        case 1:  SERIAL_WARN("something went somewhat wrong during syscall handling"); break;
        case 2:  SERIAL_WARN("an error happened during syscall handling"); break;
        default: SERIAL_INFO("handle complete, will continue\n");
    }
    return 0;
}
