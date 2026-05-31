#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_BASE 0x3F8
#define SERIAL_COM1_LINE_STATUS_PORT   0x3FD
#define SERIAL_COM1_DATA_REGISTER_PORT SERIAL_BASE

#define SERIAL_INFO(str, ...) \
    serial_printf("[%s] <info>: " str, __func__, ##__VA_ARGS__);
#define SERIAL_WARN(str, ...) \
    serial_printf("[%s] <warn>: " str, __func__, ##__VA_ARGS__);
#define SERIAL_ERROR(str, ...) \
    serial_printf("[%s] <error>: " str, __func__, ##__VA_ARGS__);
#define SERIAL_KERNEL(str, ...) \
    serial_printf("<kernel>: " str, ##__VA_ARGS__);
#define SERIAL_PANIC(str, ...) \
    serial_printf("\n!!!!!!!!\n<PANIC>\n!!!!!!!!\n" str, ##__VA_ARGS__);

void serial_putchar(char c);
void serial_init(void);
void serial_printf(const char *str, ...);

#endif //SERIAL_H
