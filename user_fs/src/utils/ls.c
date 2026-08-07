#include <lib/stdio.h>
#include <lib/stdlib.h>

#define LS_MAX_LINE_LEN 40

int main(int argc, char **argv) {
    const char *path;
    if (argc > 1) {
        path = argv[1];
    } else {
        const char *pwd = getenv("PWD");
        if (!pwd) {
            printf("ls: error: no path given, and $PWD environment variable could not be read\n");
            return -1;
        }
        path = pwd;
    }

    uint32_t n = 0;
    dir_entry *entries = readdir(path, &n);
    if (!entries) {
        print("ls: readdir failed\n");
        return 1;
    }

    uint32_t line_len = 0;
    for (uint32_t i = 0; i < n; ++i) {
        const char *name = entries[i].name;
        uint32_t name_len = strlen(name);
        uint32_t sep = line_len == 0 ? 0 : 1;

        if (line_len + sep + name_len > LS_MAX_LINE_LEN) {
            print("\n");
            line_len = 0;
            sep = 0;
        }

        if (sep) {
            print("  ");
        }
        print(name);
        line_len += sep + name_len;
    }
    print("\n");

    free(entries);
    return 0;
}
