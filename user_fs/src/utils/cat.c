#include <sys/syscalls.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>

#define CAT_FILE_BUF_MAX_LEN 1024

char *__get_full_path(const char *file_path, const char *wd) {
    if (!wd) wd = "/";

    uint32_t wl = strlen(wd);
    uint32_t fl = strlen(file_path);
    char *full_path = malloc(wl + 1 + fl + 1);   /* wd + '/' + file_path + '\0' */
    if (!full_path) return NULL;

    strcpy(full_path, wd);
    if (wl > 0 && full_path[wl - 1] != '/') {
        full_path[wl++] = '/';
        full_path[wl] = '\0';
    }
    strcat(full_path, file_path);
    return full_path;
}

int main (int argc, char **argv) {
    if (argc < 2) {
        printf("cat: no file input given\n");
        return 1;
    }

    char *file_path = argv[1];
    int outcode     = 0;

    char *full_path;
    uint8_t is_path_absolute = 0x0;

    //absolute path
    if (file_path[0] == '/') {
        full_path = file_path;
        is_path_absolute = 0x1;
    } else {
        const char *wd = getenv("PWD");
        full_path = __get_full_path(file_path, wd);
        if (!full_path) {
            printf("cat: failed to concatenate working directory and given path: malloc failed\n");
            outcode = 1;
            goto end_file_path;
        }
    }

    int fd = open(full_path, FD_FLAGS_RO);
    if (fd < 0) {
        printf("cat: failed to open '%s'\n", full_path);
        outcode = 1;
        goto end_file_path;
    }

    //TODO: readf syscall to read from fd
    char *file_buf = malloc(CAT_FILE_BUF_MAX_LEN);
    if (!file_buf) {
        printf("cat: failed to allocate memory for reading file\n");
        outcode = 1;
        goto end_fd;
    }

    readf(fd, (uint8_t*)file_buf, CAT_FILE_BUF_MAX_LEN);

    printf("%s\n", file_buf);

    end_fd:
        close(fd);
    end_file_path:
        if (!is_path_absolute) free(full_path);

    return outcode;
}
