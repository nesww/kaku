#define FS_VFS_IPL
#define DEV_CONSOLE_IPL
#define MM_FRAME_IPL
#define MM_PAGING_IPL
#define PROC_PROC_IPL
#define PROC_SCHED_IPL
#define LIB_MEM_IPL
#define LIB_STR_IPL

#include <fs/mod.h>
#include <log/mod.h>
#include <mm/mod.h>
#include <proc/mod.h>
#include <dev/mod.h>
#include <lib/mod.h>

#include "elf.h"

int elf_load(const char *path, char **argv, char **envp) {
    vfs_node *file = vfs_open(path);
    if (!file) {
        SERIAL_ERROR("could not load elf: '%s' file not found\n", path);
        return -1;
    }
    char *buf = kmalloc(file->size);
    vfs_read(file, (uint8_t*)buf, file->size);

    elf_header *elfh = (elf_header*)buf;

    if (elfh->elf_magic != ELF_MAGIC) {
        SERIAL_ERROR("failed to load elf: no elf signature in given file\n");
        return -1;
    }

    proc *p = proc_create((void(*)())(elfh->elf_program_entry_offset));

    uint32_t image_end = 0;
    for (uint32_t i = 0; i < elfh->elf_program_header_table_entry_count; ++i) {
        elf_program_header *elfph = (elf_program_header*)(
            (uint8_t*)elfh
            + elfh->elf_program_header_table_offset + i * elfh->elf_program_header_table_entry_size
        );
        if (elfph->elfph_segtype == ELFPH_SEGTYPE_LOAD) {
            uint32_t seg_end = elfph->elfph_p_vaddr + elfph->elfph_p_memsz;
            if (seg_end > image_end) image_end = seg_end;

            uint32_t pages = (elfph->elfph_p_memsz + 0xFFF) >> 12;
            for (uint32_t j = 0; j < pages; ++j) {
                uint32_t frame = (uint32_t)mm_alloc_frame();
                mm_paging_map(p->proc_pd, frame, elfph->elfph_p_vaddr + j * 4096, PROC_USER_FLAGS);
                uint8_t *dst = (uint8_t*)PHYS_TO_VIRT(frame);
                uint32_t file_offset = j * 4096;
                if (file_offset >= elfph->elfph_p_filesz) {
                    kmemset(dst, 0, 4096);
                } else {
                    uint32_t to_copy = elfph->elfph_p_filesz - file_offset;
                    if (to_copy > 4096) to_copy = 4096;
                    kmemcpy(dst, (uint8_t*)elfh + elfph->elfph_p_offset + file_offset, to_copy);
                    kmemset(dst + to_copy, 0, 4096 - to_copy);
                }
            }
        }
    }
    kfree(buf);

    p->program_break = (image_end + 0xFFF) & ~0xFFFu;
    p->mmap_next = 0x7000000;

    if (argv) {
        char **argv_start = argv;
        int argc = 0;
        while(*argv) {argc++; argv++;};

        SERIAL_INFO("elf_load: argc=%d\n", argc);
        for (int i = 0; i < argc; i++) {
            SERIAL_INFO("argv_start[%d] = %x -> '%s'\n", i, (uint32_t)argv_start[i], argv_start[i]);
        }

        char **argv_kcopy = kmalloc(sizeof(char*)*argc);
        for (uint32_t i = 0; i < argc; ++i) {
            int arg_len = kstrlen(argv_start[i]);
            argv_kcopy[i] = kmalloc(arg_len + 1);
            kmemcpy(argv_kcopy[i], argv_start[i], arg_len + 1);
        };
        p->argc = argc;
        p->argv = argv_kcopy;
    } else {
        p->argc = 0;
        p->argv = 0;
    }

    if (envp) {
        char **env_start = envp;
        int envc = 0;
        while(*envp) {envc++; envp++;};

        SERIAL_INFO("elf_load: envc=%d\n", envc);
        for (int i = 0; i < envc; i++) {
            SERIAL_INFO("env_start[%d] = %x -> '%s'\n", i, (uint32_t)env_start[i], env_start[i]);
        }

        char **env_kcopy = kmalloc(sizeof(char*)*envc);
        for (uint32_t i = 0; i < envc; ++i) {
            int env_len = kstrlen(env_start[i]);
            env_kcopy[i] = kmalloc(env_len + 1);
            kmemcpy(env_kcopy[i], env_start[i], env_len + 1);
        };
        p->envc = envc;
        p->env = env_kcopy;
    } else {
        p->envc = 0;
        p->env = 0;
    }

    /* always STARTING so the scheduler lays out an argv/env array (at least
     * [NULL]) at the top of the user stack for every process */
    p->proc_state = STARTING;


    vfs_close(file);
    scheduler_add_proc(p);
    SERIAL_INFO("program '%s' started, pid:%x\n", path, p->proc_id);
    return p->proc_id;
}
