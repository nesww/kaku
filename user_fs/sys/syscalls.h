#pragma once

#define WRITE     1
#define READ      2
#define OPEN      3
#define CLOSE     4
#define READFILE  5
#define WRITEFILE 6
#define EXIT      7
#define EXEC      8
#define KILL      9
#define WAITPID  10
#define READDIR  11
#define SBRK     12
#define MMAP     13
#define MUNMAP   14
#define GETPID   15

#define SYS_WRITE(str) asm volatile( \
    "int $0x80\n"                \
    :: "a"(WRITE),"b"(str):          \
    );

#define SYS_READ(buf_addr, buf_size) asm volatile( \
    "int $0x80\n"                              \
    :: "a"(READ), "b"(buf_addr), "c"(buf_size)            \
    : "memory");

#define FD_FLAGS_RO 0x1
#define FD_FLAGS_WO 0x2
#define FD_FLAGS_RW 0x4

#define SYS_OPEN(file_path, flags, fd_out) asm volatile( \
    "int $0x80\n"                              \
    : "=a"(fd_out)                             \
    : "a"(OPEN), "b"(file_path), "c"(flags)       \
    :);

#define SYS_CLOSE(fd) asm volatile(            \
    "int $0x80\n"                              \
    :: "a"(CLOSE), "b"(fd)                         \
    :);

#define SYS_READ_FILE(fd, buf, buf_len, read_bytes_out) do { \
    asm volatile(                                     \
        "int $0x80\n"                                 \
        : "=a"(read_bytes_out)                        \
        : "a"(READFILE), "b"(fd), "c"(buf), "d"(buf_len)     \
        : "memory");                                  \
} while(0)

#define SYS_WRITE_FILE(fd, buf, buf_len, written_bytes_out) do { \
    asm volatile(                                          \
        "int $0x80\n"                                      \
        : "=a"(written_bytes_out)                          \
        : "a"(WRITEFILE),"b"(fd), "c"(buf), "d"(buf_len)   \
        : "memory");                                       \
} while(0)

#define SYS_EXIT(excode) asm volatile( \
    "int $0x80\n"                \
    :: "a"(EXIT), "b"(excode): "memory")

#define SYS_EXEC(path, argv, envp, pid_out) asm volatile ( \
    "int $0x80\n"                                           \
    : "=a"(pid_out)                                         \
    : "a"(EXEC), "b"(path), "c"(argv), "d"(envp)            \
    :                                                       \
)

#define SYS_KILL(pid) asm volatile ( \
    "int $0x80\n"                    \
    :: "a"(KILL), "b"(pid)           \
    :                                \
)

#define SYS_WAITPID(pid, exit_code_out) asm volatile( \
    "int $0x80\n"                                     \
    : "=a"(exit_code_out)                             \
    : "a"(WAITPID), "b"(pid)                          \
    : "memory")

#define SYS_READDIR(path, buf, buf_len, read_bytes_out) asm volatile( \
    "int $0x80\n"                                                     \
    : "=a"(read_bytes_out)                                            \
    : "a"(READDIR), "b"(path), "c"(buf), "d"(buf_len)                \
    : "memory"                                                        \
)

#define SYS_SBRK(increment, old_break_out) asm volatile( \
    "int $0x80\n"                                        \
    : "=a"(old_break_out)                                \
    : "a"(SBRK), "b"(increment)                          \
    : "memory"                                           \
)

#define SYS_MMAP(size, base_out) asm volatile( \
    "int $0x80\n"                              \
    : "=a"(base_out)                           \
    : "a"(MMAP), "b"(size)                     \
    : "memory"                                 \
)

#define SYS_MUNMAP(addr, size) asm volatile( \
    "int $0x80\n"                            \
    :: "a"(MUNMAP), "b"(addr), "c"(size)     \
    : "memory"                               \
)

#define SYS_GETPID(pid_out) asm volatile ( \
    "int $0x80\n"                          \
    : "=a"(pid_out)                        \
    : "a"(GETPID)                          \
    : "memory"                             \
)
