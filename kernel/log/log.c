#include <stdarg.h>
#include <stdint.h>
#include <lib/print.h>

#include "log.h"

static log_sink_fn sinks[LOG_MAX_SINKS] = {0};
static uint8_t sink_count = 0;

void log_register_sink(log_sink_fn fn) {
    sinks[sink_count++] = fn;
}

void log_unregister_sink(log_sink_fn fn) {
    //TODO
}

void log_write(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_vwrite(fmt, args);
    va_end(args);
}

void log_vwrite(const char *fmt, va_list args) {
    char buf[512];
    kvsnprintf(buf, sizeof(buf), fmt, args);
    for (uint8_t i = 0; i < sink_count; ++i)
        sinks[i](buf);
}
