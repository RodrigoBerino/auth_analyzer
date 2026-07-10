#include "log_parser.h"

#include <string.h>

int log_line_is_failed_auth(const char *line, size_t len) {
    /* No formato auth.log usado no projeto, tentativas malsucedidas contem a
       palavra "Failed". */
    return memmem(line, len, "Failed", 6) != NULL;
}

int log_line_extract_ip(const char *line, size_t len, char *ip, size_t ip_size) {
    /* O IP vem depois do trecho " from ":
       Failed password for root from 185.220.101.42 port ... */
    const char *from = memmem(line, len, " from ", 6);
    if (!from || ip_size == 0) return -1;

    const char *start = from + 6;
    const char *end = line + len;
    size_t copied = 0;

    while (start + copied < end && start[copied] != ' ' && copied < ip_size - 1) {
        ip[copied] = start[copied];
        copied++;
    }

    if (copied == 0) return -1;
    ip[copied] = '\0';
    return 0;
}
