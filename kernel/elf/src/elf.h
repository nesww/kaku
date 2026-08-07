#pragma once

#include <stdint.h>

#define ELF_HEADER_SIZE 52
#define ELF_MAGIC       0x464C457F

#define ELF_INST_SET_X86     0x03
#define ELF_INST_SET_ARM     0x28
#define ELF_INST_SET_X86_64  0x3E
#define ELF_INST_SET_AARCH64 0xB7

typedef struct {
    uint32_t elf_magic;
    uint8_t  elf_parch;
    uint8_t  elf_header_ver;
    uint8_t  elf_os_abi;
    uint8_t __elf_unused[8];
    uint16_t elf_type;
    uint16_t elf_inst_set;
    uint32_t elf_version;
    uint32_t elf_program_entry_offset;
    uint32_t elf_program_header_table_offset;
    uint32_t elf_section_header_table_offset;
    uint32_t elf_flags;
    uint16_t elf_header_size;
    uint16_t elf_program_header_table_entry_size;
    uint16_t elf_program_header_table_entry_count;
    uint16_t elf_section_header_table_entry_size;
    uint16_t elf_section_header_table_entry_count;
    uint16_t elf_section_index_to_section_header_str_table;
} elf_header;

#define ELFPH_SEGTYPE_NULL   0
#define ELFPH_SEGTYPE_LOAD   1
#define ELFPH_SEGTYPE_DYN    2
#define ELFPH_SEGTYPE_INTERP 3
#define ELFPH_SEGTYPE_NOTE   4

#define ELFPH_FLAGS_EXEC 0x1
#define ELFPH_FLAGS_WRIT 0x2
#define ELFPH_FLAGS_READ 0x3

typedef struct {
    uint32_t elfph_segtype;
    uint32_t elfph_p_offset;
    uint32_t elfph_p_vaddr;
    uint32_t elfph_p_paddr;
    uint32_t elfph_p_filesz;
    uint32_t elfph_p_memsz;
    uint32_t elfph_flags;
    uint32_t elfph_req_alignment;
} elf_program_header;

int elf_load(const char *path, char **argv, char **envp);
