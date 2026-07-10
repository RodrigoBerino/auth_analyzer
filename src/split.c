#include "split.h"
#include <stdlib.h>

Block *split_file(char *data, size_t size, int n_blocks) {
    /* Divide o arquivo em N blocos aproximados. Cada bloco aponta para um
       intervalo dentro do mmap original; nenhuma copia de texto e feita. */
    Block *blocks = malloc((size_t)n_blocks * sizeof(Block));
    if (!blocks) return NULL;

    size_t chunk = size / (size_t)n_blocks;
    char  *pos   = data;
    char  *end   = data + size;

    for (int i = 0; i < n_blocks; i++) {
        blocks[i].block_id = i;
        blocks[i].start    = pos;

        if (i == n_blocks - 1) {
            blocks[i].end = end;
        } else {
            //validações
            char *boundary = pos + chunk;
            if (boundary > end) boundary = end;

            /* ajusta a fronteira ate o fim da linha atual. Assim uma linha de
               log nunca fica partida entre duas threads. */
            while (boundary < end && *boundary != '\n')
                boundary++;

            if (boundary < end)
                boundary++;

            blocks[i].end = boundary;
        }
        pos = blocks[i].end;
    }
    return blocks;
}
