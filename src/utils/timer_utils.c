#define _POSIX_C_SOURCE 199309L

#include "timer_utils.h"

#include <time.h>

double timer_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

size_t count_lines(const char *data, size_t size) {
    size_t total = 0;

    for (size_t i = 0; i < size; i++) {
        if (data[i] == '\n') total++;
    }

    return total;
}
