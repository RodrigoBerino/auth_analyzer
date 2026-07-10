#ifndef SPLIT_H
#define SPLIT_H

#include <stddef.h>

typedef struct {
    char *start;
    char *end;
    int   block_id;
} Block;

// limites sempre ajustados para um '\n' — nenhuma linha e cortada entre blocos
Block *split_file(char *data, size_t size, int n_blocks);

#endif /* SPLIT_H */
