#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <stddef.h>

int log_line_is_failed_auth(const char *line, size_t len);
int log_line_extract_ip(const char *line, size_t len, char *ip, size_t ip_size);

#endif /* LOG_PARSER_H */
