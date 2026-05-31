#ifndef STDCORE_H
#define STDCORE_H

#define TRUE 1
#define FALSE 0

#define KMIN(a,b) ((a) < (b) ? (a) : (b))
#define KMAX(a,b) ((a) > (b) ? (a) : (b))

#define KB(x) ((x) * 1024)
#define MB(x) ((x) * 1024 * 1024)

#define TODO(str)                           \
    do {                                    \
        if (str)                            \
            vga_printf("TODO: " #str "\n"); \
    } while(0)                              \

#endif //STDCORE_H
