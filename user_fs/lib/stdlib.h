#pragma once

#include <sys/syscalls.h>
#include <lib/string.h>
#include <lib/mem.h>

#include "core.h"

#define NULL 0

/* defined by _start.o; the scheduler fills it with the process environment */
extern char **environ;

SINGLE_H_LIB_FUNC
int exec(const char *path, char *argv[]) {
    int pid;
    SYS_EXEC(path, argv, environ, pid);
    return pid;
}

SINGLE_H_LIB_FUNC
void exit(int excode) {
    SYS_EXIT(excode);
    while(1);
}

SINGLE_H_LIB_FUNC
void kill(int pid) {
    SYS_KILL(pid);
}

SINGLE_H_LIB_FUNC
int waitpid(int pid) {
    int exit_code;
    SYS_WAITPID(pid, exit_code);
    return exit_code;
}

/* returns the value of `name` (or 0 if unset) */
SINGLE_H_LIB_FUNC
const char *getenv(const char *name) {
    uint32_t nlen = strlen(name);
    for (uint32_t i = 0; environ[i]; ++i) {
        const char *p = environ[i];
        uint32_t j = 0;
        while (j < nlen && p[j] == name[j]) j++;
        if (j == nlen && p[j] == '=') {
            return p + j + 1;
        }
    }
    return 0;
}

/* sets `name=value` in the environment (overrides if already set) */
SINGLE_H_LIB_FUNC
int setenv(const char *name, const char *value) {
    uint32_t nlen = strlen(name);
    uint32_t vlen = strlen(value);

    char *entry = malloc(nlen + 1 + vlen + 1);
    if (!entry) return -1;
    uint32_t k = 0;
    for (uint32_t i = 0; i < nlen; ++i) entry[k++] = name[i];
    entry[k++] = '=';
    for (uint32_t i = 0; i < vlen; ++i) entry[k++] = value[i];
    entry[k] = '\0';

    uint32_t cnt = 0;
    int replace = -1;
    for (uint32_t i = 0; environ[i]; ++i) {
        uint32_t j = 0;
        while (j < nlen && environ[i][j] == name[j]) j++;
        if (j == nlen && environ[i][j] == '=') replace = (int)i;
        cnt++;
    }

    uint32_t newcnt = (replace >= 0) ? cnt : cnt + 1;
    char **newenv = malloc((newcnt + 1) * sizeof(char*));
    if (!newenv) {
        free(entry);
        return -1;
    }

    uint32_t w = 0;
    for (uint32_t i = 0; i < cnt; ++i) {
        if ((int)i != replace) newenv[w++] = environ[i];
    }
    newenv[w++] = entry;
    newenv[w] = 0;

    environ = newenv;
    return 0;
}

SINGLE_H_LIB_FUNC
int getpid() {
    int pid;
    SYS_GETPID(pid);
    return pid;
}
