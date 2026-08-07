#include <lib/stdio.h>
#include <lib/stdlib.h>
#include <lib/stdutils.h>
#include <lib/string.h>


#define KAI_STDIN_BUF_MAX 512
#define KAI_TOKENS_MAX 31

#define WD_MAX_LEN 512
static char *wd = NULL;

typedef uint8_t kai_cmd_t;
#define CMD_EMPTY   0x0
#define CMD_BUILTIN 0x1
#define CMD_OTHER   0x2

typedef uint8_t kai_cmd_outcode_t;
#define CMD_OUTCODE_SUCCESS 0x0
#define CMD_OUTCODE_FAILURE 0x1

/* normalize `target` (relative or absolute) against $PWD into `out`, a
 * canonical absolute path. Handles ".", "..", empty components, double
 * slashes, and the root. */
static void __builtin_cd__resolve(const char *target, char *out) {
    uint32_t n = 0;
    char **comp = calloc(KAI_TOKENS_MAX, sizeof(char*));
    if (!comp) { out[0] = '\0'; return; }

    if (target[0] != '/') {
        const char *pwd = getenv("PWD");
        if (!pwd) pwd = "/";
        char **base = calloc(KAI_TOKENS_MAX, sizeof(char*));
        if (base) {
            int bn = strsplit(pwd, base, "/");
            for (int i = 0; i < bn && n < KAI_TOKENS_MAX; ++i) comp[n++] = base[i];
            free(base);
        }
    }

    char **tc = calloc(KAI_TOKENS_MAX, sizeof(char*));
    if (tc) {
        int tn = strsplit(target, tc, "/");
        for (int i = 0; i < tn; ++i) {
            if (strcmp(tc[i], ".") == 0) {
                free(tc[i]);
            } else if (strcmp(tc[i], "..") == 0) {
                if (n > 0) free(comp[--n]);
                free(tc[i]);
            } else if (n < KAI_TOKENS_MAX) {
                comp[n++] = tc[i];
            } else {
                free(tc[i]);
            }
        }
        free(tc);
    }

    uint32_t used = 0;
    out[used++] = '/';
    for (uint32_t i = 0; i < n; ++i) {
        uint32_t len = strlen(comp[i]);
        if (used + 1 + len < WD_MAX_LEN) {
            if (i > 0) out[used++] = '/';
            for (uint32_t k = 0; k < len; ++k) out[used++] = comp[i][k];
            out[used] = '\0';
        }
    }
    if (n == 0) { out[0] = '/'; out[1] = '\0'; }

    for (uint32_t i = 0; i < n; ++i) free(comp[i]);
    free(comp);
}

kai_cmd_outcode_t __builtin_cd(char **args) {
    const char *target;
    const char *display;

    if (!args[1] || strlen(args[1]) == 0) {
        printf("kai: cd: cd with no args, defaulting to $HOME\n");
        const char *home = getenv("HOME");
        if (!home) {
            printf("kai: cd: error: no $HOME env var is set\n");
            return CMD_OUTCODE_FAILURE;
        }
        target  = home;
        display = home;
    } else {
        target  = args[1];
        display = args[1];
    }

    __builtin_cd__resolve(target, wd);

    /* validate the target is a directory */
    uint32_t n = 0;
    dir_entry *entries = readdir(wd, &n);
    if (!entries) {
        printf("cd: '%s' is not a directory or not found\n", display);
        return CMD_OUTCODE_FAILURE;
    }
    free(entries);

    setenv("PWD", wd);
    return CMD_OUTCODE_SUCCESS;
}

kai_cmd_t __process_builtin(const char *cmd, char **args) {
    kai_cmd_t cmd_type = CMD_EMPTY;

    if (!cmd || strlen(cmd) == 0) {
        return CMD_EMPTY;
    }

    if (!strcmp(cmd, "cd")) {
        __builtin_cd(args);

        cmd_type = CMD_BUILTIN;
        goto end;
    } else if (!strcmp(cmd, "pwd")) {
        const char *pwd = getenv("PWD");
        printf("Working directory: %s\n", pwd ? pwd : wd);
        cmd_type = CMD_BUILTIN;
        goto end;
    } else if (!strcmp(cmd, "exit")) {
        exit(0);
    }

    if (strlen(cmd) > 0) cmd_type = CMD_OTHER;

    end:
    return cmd_type;
}


int main(void) {
    wd = malloc(WD_MAX_LEN);
    strcpy(wd, "/user");
    setenv("PWD", wd);

    setenv("HOME", "/user");

    char *input_buf = malloc(KAI_STDIN_BUF_MAX);
    if (!input_buf) return 1;
    while (1) {
        const char *cur = getenv("PWD");
        printf("(kai) %s > ", cur ? cur : wd);
        read(input_buf, KAI_STDIN_BUF_MAX);
        println("");

        char **command_parts = calloc(KAI_TOKENS_MAX + 1, sizeof(char*));

        if (!command_parts) {
            printf("kai: internal error: command could not be split due to memory allocation failure\n");
            continue;
        }

        int n = 0;
        char *tok = strtok(input_buf, " ");
        while (tok && n < KAI_TOKENS_MAX) {
            command_parts[n++] = tok;
            tok = strtok(0, " ");
        }
        command_parts[n] = 0;

        kai_cmd_t cmd_type = __process_builtin(command_parts[0], command_parts);

        switch(cmd_type) {
            case CMD_OTHER: {
                char *bin        = command_parts[0];
                int bin_path_len = strlen("/bin/") + strlen(bin) + 1;
                char *bin_path   = malloc(bin_path_len);

                snprintf(bin_path, bin_path_len, "/bin/%s", bin);

                command_parts[0] = bin_path;

                int ret = exec(bin_path, command_parts);
                if (ret >= 0) {
                    waitpid(ret);
                } else {
                    printf("command '%s' not found in '%s' (default path)\n", bin, bin_path);
                }

                free(bin_path);
                free(command_parts);
            }
            break;
            case CMD_EMPTY: println(""); break;
            case CMD_BUILTIN: continue;
            default:;
        }
    }
    exit(0);
}
