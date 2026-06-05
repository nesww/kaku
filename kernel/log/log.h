#pragma once

#include <stdarg.h>

#define LOG_MAX_SINKS 5
#define LOG_INFO(fmt, ...)  log_write("[info] " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  log_write("[warn] " fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log_write("[error] " fmt, ##__VA_ARGS__)
#define LOG_PANIC(fmt, ...) log_write("[panic] " fmt, ##__VA_ARGS__)

typedef void(*log_sink_fn)(const char *str);

void log_register_sink(log_sink_fn fn);
void log_unregister_sink(log_sink_fn fn);
void log_write(const char *fmt, ...);
void log_vwrite(const char *fmt, va_list args);
