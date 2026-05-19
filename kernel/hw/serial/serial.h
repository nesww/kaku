#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_BASE 0x3F8
#define SERIAL_COM1_LINE_STATUS_PORT   0x3FD
#define SERIAL_COM1_DATA_REGISTER_PORT SERIAL_BASE

void serial_putchar(char c);
void serial_init(void);
void serial_printf(const char *str, ...);

#endif //SERIAL_H
